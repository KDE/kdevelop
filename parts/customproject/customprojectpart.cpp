/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "customprojectpart.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qvaluestack.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kpopupmenu.h>
#include <kdeversion.h>
#include <kprocess.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "runoptionswidget.h"
#include "makeoptionswidget.h"
#include "custombuildoptionswidget.h"
#include "config.h"


typedef KGenericFactory<CustomProjectPart> CustomProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcustomproject, CustomProjectFactory( "kdevcustomproject" ) );

CustomProjectPart::CustomProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject("CustomProject", "customproject", parent, name ? name : "CustomProjectPart")
{
    setInstance(CustomProjectFactory::instance());
    setXMLFile("kdevcustomproject.rc");
    
    m_executeAfterBuild = false;    

    KAction *action;

    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );

    action = new KAction( i18n("Compile &File"), "make_kdevelop",
                          this, SLOT(slotCompileFile()),
                          actionCollection(), "build_compilefile" );

    action = new KAction( i18n("&Clean Project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "build_clean" );

    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_execute" );

    KActionMenu *menu = new KActionMenu( i18n("Build &Target"),
                                         actionCollection(), "build_target" );
    m_targetMenu = menu->popupMenu();

    connect( m_targetMenu, SIGNAL(aboutToShow()),
             this, SLOT(updateTargetMenu()) );
    connect( m_targetMenu, SIGNAL(activated(int)),
             this, SLOT(targetMenuActivated(int)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    
    connect( makeFrontend(), SIGNAL(commandFinished(const QString&)),
	     this, SLOT(slotCommandFinished(const QString&)) );
}


CustomProjectPart::~CustomProjectPart()
{}


void CustomProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Run Options"));
    RunOptionsWidget *w1 = new RunOptionsWidget(*projectDom(), "/kdevcustomproject",projectDirectory(), vbox);
    connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );
    vbox = dlg->addVBoxPage(i18n("Build Options"));
    QTabWidget *buildtab = new QTabWidget(vbox);

    CustomBuildOptionsWidget *w2 = new CustomBuildOptionsWidget(*projectDom(), buildtab);
    connect( dlg, SIGNAL(okClicked()), w2, SLOT(accept()) );
    buildtab->addTab(w2, i18n("Build"));

    MakeOptionsWidget *w3 = new MakeOptionsWidget(*projectDom(), "/kdevcustomproject", buildtab);
    connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );
    buildtab->addTab(w3, i18n("Make"));

    w2->setMakeOptionsWidget(buildtab, w3);
}


void CustomProjectPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType("file"))
        return;

    const FileContext *fcontext = static_cast<const FileContext*>(context);
    if (fcontext->isDirectory())
        return;

    m_contextFileName = fcontext->fileName();
    bool inProject = project()->allFiles().contains(m_contextFileName.mid ( project()->projectDirectory().length() + 1 ));
    QString popupstr = QFileInfo(m_contextFileName).fileName();
    if (m_contextFileName.startsWith(projectDirectory()+ "/"))
        m_contextFileName.remove(0, projectDirectory().length()+1);

    popup->insertSeparator();
    if (inProject)
        popup->insertItem( i18n("Remove from Project: %1").arg(popupstr),
                           this, SLOT(slotRemoveFromProject()) );
    else
        popup->insertItem( i18n("Add to Project: %1").arg(popupstr),
                           this, SLOT(slotAddToProject()) );
}


void CustomProjectPart::slotAddToProject()
{
    addFile(m_contextFileName);
}


void CustomProjectPart::slotRemoveFromProject()
{
    removeFile(m_contextFileName);
}


void CustomProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectDirectory = dirName;
    m_projectName = projectName;

    QFile f(dirName + "/" + projectName + ".filelist");
    if (f.open(IO_ReadOnly)) {
        QTextStream stream(&f);
        while (!stream.atEnd()) {
            QString s = stream.readLine();
            if (!s.startsWith("#"))
                m_sourceFiles << s;
        }
    } else {
        int r = KMessageBox::questionYesNo(mainWindow()->main(),
                                           i18n("This project does not contain any files yet.\n"
                                                "Populate it with all C/C++/Java files below "
                                                "the project directory?"));
        if (r == KMessageBox::Yes)
            populateProject();
    }
}


void CustomProjectPart::populateProject()
{
    QApplication::setOverrideCursor(Qt::waitCursor);

    QValueStack<QString> s;
    int prefixlen = m_projectDirectory.length()+1;
    s.push(m_projectDirectory);

    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9025) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        QPtrListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug(9025) << "Pushing: " << path << endl;
                s.push(path);
            }
            else {
                kdDebug(9025) << "Adding: " << path << endl;
                m_sourceFiles.append(path.mid(prefixlen));
            }
        }
    } while (!s.isEmpty());

    QApplication::restoreOverrideCursor();
}


void CustomProjectPart::closeProject()
{
    QFile f(m_projectDirectory + "/" + m_projectName + ".filelist");
    if (!f.open(IO_WriteOnly))
        return;

    QTextStream stream(&f);
    stream << "# KDevelop Custom Project File List" << endl;

    QStringList::ConstIterator it;
    for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it)
        stream << (*it) << endl;
    f.close();
}


QString CustomProjectPart::projectDirectory()
{
    return m_projectDirectory;
}


QString CustomProjectPart::projectName()
{
    return m_projectName;
}


QString CustomProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return QDir::cleanDirPath(projectDirectory() + "/" + DomUtil::readEntry(dom, "/kdevcustomproject/run/mainprogram"));
}


QString CustomProjectPart::activeDirectory()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevcustomproject/general/activedir");
}


QStringList CustomProjectPart::allFiles()
{
//     QStringList res;
//
//     QStringList::ConstIterator it;
//     for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it) {
//         QString fileName = *it;
//         if (!fileName.startsWith("/")) {
//             fileName.prepend("/");
//             fileName.prepend(m_projectDirectory);
//         }
//         res += fileName;
//     }
//
//     return res;

	// return all files relative to the project directory!
	return m_sourceFiles;
}


void CustomProjectPart::addFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );

	this->addFiles ( fileList );
}

void CustomProjectPart::addFiles ( const QStringList& fileList )
{
	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		m_sourceFiles.append ( *it );
	}

	kdDebug(9025) << "Emitting addedFilesToProject" << endl;
	emit addedFilesToProject ( fileList );
}

void CustomProjectPart::removeFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );

	this->addFiles ( fileList );
}

void CustomProjectPart::removeFiles ( const QStringList& fileList )
{
	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		m_sourceFiles.remove ( *it );
	}

	kdDebug(9025) << "Emitting removedFilesFromProject" << endl;
	emit removedFilesFromProject ( fileList );
}

QString CustomProjectPart::buildDirectory()
{
    QString dir = DomUtil::readEntry(*projectDom(), "/kdevcustomproject/build/builddir");
    return dir.isEmpty()? projectDirectory() : dir;
}


void CustomProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QDomDocument &dom = *projectDom();
    bool ant = DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "ant";

    if (!ant) {
        QFileInfo fi(dir + "/Makefile");
	QFileInfo fi2(dir + "/makefile");
        if (!fi.exists() && !fi2.exists()) {
            KMessageBox::information(mainWindow()->main(),
                                     i18n("There is no Makefile in this directory."));
            return;
        }
    }

    QString cmdline;
    if (ant) {
        cmdline = "ant";
    } else {
        cmdline = DomUtil::readEntry(dom, "/kdevcustomproject/make/makebin");
        if (cmdline.isEmpty())
            cmdline = MAKE_COMMAND;
        if (!DomUtil::readBoolEntry(dom, "/kdevcustomproject/make/abortonerror"))
            cmdline += " -k";
        int jobs = DomUtil::readIntEntry(dom, "/kdevcustomproject/make/numberofjobs");
        if (jobs != 0) {
            cmdline += " -j";
            cmdline += QString::number(jobs);
        }
        if (DomUtil::readBoolEntry(dom, "/kdevcustomproject/make/dontact"))
            cmdline += " -n";
    }

    cmdline += " ";
    cmdline += target;

    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}


void CustomProjectPart::slotBuild()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1(""));
}


void CustomProjectPart::slotCompileFile()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    if (!part || !part->url().isLocalFile())
        return;

    QString fileName = part->url().path();
    QFileInfo fi(fileName);
    QString sourceDir = fi.dirPath();
    QString baseName = fi.baseName();
    kdDebug(9020) << "Compiling " << fileName
                  << "in dir " << sourceDir
                  << " with baseName " << baseName << endl;

    // What would be nice: In case of non-recursive build system, climb up from
    // the source dir until a Makefile is found

    QString buildDir = sourceDir;
    QString target = baseName + ".o";
    kdDebug(9020) << "builddir " << buildDir << ", target " << target << endl;

    startMakeCommand(buildDir, target);
}


void CustomProjectPart::slotClean()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1("clean"));
}


void CustomProjectPart::slotExecute()
{
    partController()->saveAllFiles();

    if( !m_executeAfterBuild && isDirty() ){
        m_executeAfterBuild = true;
        slotBuild();
        return;
    }

    QString directory;
    QString program = project()->mainProgram();
    int pos = program.findRev('/');
    if (pos != -1) {
        // Directory where the executable is
        directory = program.left(pos+1);
        // Executable name with a "./" prepended for execution with bash shells
        program   = "./" + (project()->mainProgram()).mid(pos+1);
    }

    program += " " + DomUtil::readEntry(*projectDom(), "/kdevcustomproject/run/programargs");

    // Get the run environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*projectDom(), "/kdevcustomproject/run/envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
        environstr += KProcess::quote((*it).second);
        environstr += " ";
    }
    program.prepend(environstr);

    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevcustomproject/run/terminal");
    appFrontend()->startAppCommand(directory, program, inTerminal);
}


void CustomProjectPart::updateTargetMenu()
{
    m_targets.clear();
    m_targetMenu->clear();

    QDomDocument &dom = *projectDom();
    bool ant = DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "ant";

    if (ant) {
        QFile f(buildDirectory() + "/build.xml");
        if (!f.open(IO_ReadOnly)) {
            kdDebug(9025) << "No build file" << endl;
            return;
        }
        QDomDocument dom;
        if (!dom.setContent(&f)) {
            kdDebug(9025) << "Build script not valid xml" << endl;
            f.close();
            return;
        }
        f.close();

        QDomNode node = dom.documentElement().firstChild();
        while (!node.isNull()) {
            if (node.toElement().tagName() == "target")
                m_targets.append(node.toElement().attribute("name"));
            node = node.nextSibling();
        }
    } else {
    	kdDebug(9025) << "Trying to load a makefile... " << endl;
        QFile f(buildDirectory() + "/Makefile");
	if (!f.exists())
	    f.setName( buildDirectory() + "/makefile" );
        if (!f.open(IO_ReadOnly)) {
            kdDebug(9025) << "No Makefile" << endl;
            return;
        }
        QTextStream stream(&f);
        //QRegExp re(".PHONY\\s*:(.*)");
	QRegExp re("^([^($%.#][^)\\s]+):.*$");
	re.setMinimal(true);
	QString str = "";
        while (!stream.atEnd()) {
            QString str = stream.readLine();
            // Read all continuation lines
	    // kdDebug(9025) << "Trying: " << str.simplifyWhiteSpace() << endl;
            //while (str.right(1) == "\\" && !stream.atEnd()) {
            //    str.remove(str.length()-1, 1);
            //    str += stream.readLine();
            //}
            if (re.search(str) != -1)
            {
	        kdDebug(9025) << "Adding target: " << re.cap(1) << endl;
		m_targets += re.cap(1).simplifyWhiteSpace();
            }
        }
        f.close();
    }

    int id = 0;
    QStringList::ConstIterator it;
    for (it = m_targets.begin(); it != m_targets.end(); ++it)
        m_targetMenu->insertItem(*it, id++);
}


void CustomProjectPart::targetMenuActivated(int id)
{
    QString target = m_targets[id];
    startMakeCommand(buildDirectory(), target);
}

void CustomProjectPart::slotCommandFinished( const QString& command )
{
    kdDebug(9020) << "CustomProjectPart::slotProcessFinished()" << endl;

    Q_UNUSED( command );

    if( m_buildCommand != command )
        return;

    m_buildCommand = QString::null;

    m_timestamp.clear();
    QStringList fileList = allFiles();
    QStringList::Iterator it = fileList.begin();
    while( it != fileList.end() ){
        QString fileName = *it;
        ++it;

        m_timestamp[ fileName ] = QFileInfo( projectDirectory(), fileName ).lastModified();
    }

    emit projectCompiled();

    if( m_executeAfterBuild ){
        slotExecute();
        m_executeAfterBuild = false;
    }
}

bool CustomProjectPart::isDirty()
{
    QStringList fileList = allFiles();
    QStringList::Iterator it = fileList.begin();
    while( it != fileList.end() ){
        QString fileName = *it;
        ++it;

        QMap<QString, QDateTime>::Iterator it = m_timestamp.find( fileName );
        QDateTime t = QFileInfo( projectDirectory(), fileName ).lastModified();
        if( it == m_timestamp.end() || *it != t ){
            return true;
        }
    }

    return false;
}

#include "customprojectpart.moc"
