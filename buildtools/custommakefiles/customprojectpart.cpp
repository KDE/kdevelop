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
#include <qdom.h>

#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kdevgenericfactory.h>
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
#include "custommakeconfigwidget.h"
#include "config.h"
#include "envvartools.h"
#include "urlutil.h"


typedef KDevGenericFactory<CustomProjectPart> CustomProjectFactory;
static const KAboutData data("kdevcustomproject", I18N_NOOP("Build Tool"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevcustomproject, CustomProjectFactory( &data ) )

CustomProjectPart::CustomProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject("CustomProject", "customproject", parent, name ? name : "CustomProjectPart")
	, m_lastCompilationFailed(false)
{
    setInstance(CustomProjectFactory::instance());
    setXMLFile("kdevcustomproject.rc");

    m_executeAfterBuild = false;

    KAction *action;

    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    action->setToolTip(i18n("Build project"));
    action->setWhatsThis(i18n("<b>Build project</b><p>Runs <b>make</b> from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Build Options</b> tab."));

    action = new KAction( i18n("Compile &File"), "make_kdevelop",
                          this, SLOT(slotCompileFile()),
                          actionCollection(), "build_compilefile" );
    action->setToolTip(i18n("Compile file"));
    action->setWhatsThis(i18n("<b>Compile file</b><p>Runs <b>make filename.o</b> command from the directory where 'filename' is the name of currently opened file.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Build Options</b> tab."));

    action = new KAction( i18n("&Clean Project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "build_clean" );
    action->setToolTip(i18n("Clean project"));
    action->setWhatsThis(i18n("<b>Clean project</b><p>Runs <b>make clean</b> command from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Build Options</b> tab."));

    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_execute" );
    action->setToolTip(i18n("Execute program"));
    action->setWhatsThis(i18n("<b>Execute program</b><p>Executes the main program specified in project settings, <b>Run Options</b> tab. "
                            "If it is not specified then the active target is used to determine the application to run."));

    KActionMenu *menu = new KActionMenu( i18n("Build &Target"),
                                         actionCollection(), "build_target" );
    m_targetMenu = menu->popupMenu();
    menu->setToolTip(i18n("Build target"));
    menu->setWhatsThis(i18n("<b>Build target</b><p>Runs <b>make targetname</b> from the project directory (targetname is the name of the target selected).<br>"
                            "Environment variables and make arguments can be specified "
                            "in the project settings dialog, <b>Build Options</b> tab."));

    m_makeEnvironmentsSelector = new KSelectAction( i18n("Make &Environment"),0,
                            actionCollection(), "build_make_environment" );
    m_makeEnvironmentsSelector->setToolTip(i18n("Make Environment"));
    m_makeEnvironmentsSelector->setWhatsThis(i18n("<b>Make Environment</b><p> Choose the set of environment variables to be passed on to make.<br>"
                            "Environment variables can be specified in the project "
                            "settings dialog, <b>Build Options</b> tab."));

    connect( m_targetMenu, SIGNAL(aboutToShow()),
             this, SLOT(updateTargetMenu()) );
    connect( m_targetMenu, SIGNAL(activated(int)),
             this, SLOT(targetMenuActivated(int)) );
    connect( m_makeEnvironmentsSelector->popupMenu(), SIGNAL(aboutToShow()),
             this, SLOT(updateMakeEnvironmentsMenu()) );
    connect( m_makeEnvironmentsSelector->popupMenu(), SIGNAL(activated(int)),
             this, SLOT(makeEnvironmentsMenuActivated(int)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

    connect( makeFrontend(), SIGNAL(commandFinished(const QString&)),
             this, SLOT(slotCommandFinished(const QString&)) );
    connect( makeFrontend(), SIGNAL(commandFailed(const QString&)),
             this, SLOT(slotCommandFailed(const QString&)) );
}


CustomProjectPart::~CustomProjectPart()
{}


void CustomProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Run Options"));
    RunOptionsWidget *w1 = new RunOptionsWidget(*projectDom(), "/kdevcustomproject", buildDirectory(), vbox);
    connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );
    vbox = dlg->addVBoxPage(i18n("Build Options"));
    QTabWidget *buildtab = new QTabWidget(vbox);

    CustomBuildOptionsWidget *w2 = new CustomBuildOptionsWidget(*projectDom(), buildtab);
    connect( dlg, SIGNAL(okClicked()), w2, SLOT(accept()) );
    buildtab->addTab(w2, i18n("&Build"));

    CustomMakeConfigWidget *w3 = new CustomMakeConfigWidget(this, "/kdevcustomproject", buildtab);
    buildtab->addTab(w3, i18n("Ma&ke"));
    w2->setMakeOptionsWidget(buildtab, w3);
    connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );
}


void CustomProjectPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType( Context::FileContext ))
        return;

    const FileContext *fcontext = static_cast<const FileContext*>(context);
    if (fcontext->isDirectory()) {
        // remember the name of the directory
        m_contextDirName = fcontext->fileName();
        m_contextDirName = m_contextDirName.mid ( project()->projectDirectory().length() + 1 );
        popup->insertSeparator();
        int id = popup->insertItem( i18n("Make Active Directory"),
                           this, SLOT(slotChooseActiveDirectory()) );
        popup->setWhatsThis(id, i18n("<b>Make active directory</b><p>"
        "Chooses this directory as the destination for new files created using wizards "
        "like the <i>New Class</i> wizard."));
        return;
    }
    m_contextAddFiles.clear();
    m_contextRemoveFiles.clear();

    if( fcontext->urls().size() == 1 )
    {
        QString contextFileName = URLUtil::canonicalPath(fcontext->fileName());
        bool inProject = project()->isProjectFile(contextFileName);
        QString popupstr = QFileInfo(contextFileName).fileName();
        if (contextFileName.startsWith(projectDirectory()+ "/"))
            contextFileName.remove(0, projectDirectory().length()+1);

        popup->insertSeparator();
        if (inProject)
        {
	    m_contextRemoveFiles << contextFileName;
            int id = popup->insertItem( i18n("Remove %1 From Project").arg(popupstr),
                               this, SLOT(slotRemoveFromProject()) );
            popup->setWhatsThis(id, i18n("<b>Remove from project</b><p>Removes current file from the list of files in project. "
                "Note that the file should be manually excluded from corresponding makefile or build.xml."));
        }
        else
        {
	    m_contextAddFiles << contextFileName;
            int id = popup->insertItem( i18n("Add %1 to Project").arg(popupstr),
                               this, SLOT(slotAddToProject()) );
            popup->setWhatsThis(id, i18n("<b>Add to project</b><p>Adds current file to the list of files in project. "
                "Note that the file should be manually added to corresponding makefile or build.xml."));
        }
    }
    else   // more than one file
    {
        const KURL::List urls = fcontext->urls();
	for (KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it)
	{
	    if ((*it).isLocalFile())
	    {
	    	QString path(URLUtil::canonicalPath((*it).path()));
		QString relPath( path );
                if (relPath.startsWith(projectDirectory()+ "/"))
                    relPath.remove(0, projectDirectory().length()+1);
		if (project()->isProjectFile(path))
		    m_contextRemoveFiles << relPath;
		else
		    m_contextAddFiles << relPath;
	    }
	}

        if (m_contextAddFiles.size() > 0)
	{
            int id = popup->insertItem( i18n("Add Selected Files to Project"),
                               this, SLOT(slotAddToProject()) );
            popup->setWhatsThis(id, i18n("<b>Add to project</b><p>Adds selected files to the list of files in project. "
                "Note that the files should be manually added to corresponding makefile or build.xml."));
	}

	if (m_contextRemoveFiles.size() > 0)
	{
            int id = popup->insertItem( i18n("Remove Selected Files From Project"),
                               this, SLOT(slotRemoveFromProject()) );
            popup->setWhatsThis(id, i18n("<b>Remove from project</b><p>Removes selected files from the list of files in project. "
                "Note that the files should be manually excluded from corresponding makefile or build.xml."));
	}
    }
}


void CustomProjectPart::slotAddToProject()
{
    addFiles(m_contextAddFiles);
}


void CustomProjectPart::slotRemoveFromProject()
{
    removeFiles(m_contextRemoveFiles);
}


void CustomProjectPart::slotChooseActiveDirectory()
{
    QDomDocument &dom = *projectDom();
    DomUtil::writeEntry(dom, "/kdevcustomproject/general/activedir", m_contextDirName);
}


void CustomProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectDirectory = dirName;
    m_projectName = projectName;

    QDomDocument &dom = *projectDom();
    // Set the default directory radio to "executable"
    if (DomUtil::readEntry(dom, "/kdevcustomproject/run/directoryradio") == "" ) {
        DomUtil::writeEntry(dom, "/kdevcustomproject/run/directoryradio", "executable");
    }

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

    // check if there is an old envvars entry (from old project file with single make environment)
    QDomElement el =
        DomUtil::elementByPath( dom , "/kdevcustomproject/make/envvars");
    if (!el.isNull()) {
        QDomElement envs = DomUtil::createElementByPath( dom , "/kdevcustomproject/make/environments");
        DomUtil::makeEmpty(envs);
        el.setTagName("default");
        envs.appendChild(el);
    }

    KDevProject::openProject( dirName, projectName );
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
        if ( dirEntries )
        {
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
        }
    } while (!s.isEmpty());

    QApplication::restoreOverrideCursor();
}


void CustomProjectPart::closeProject()
{
    saveProject();
}

void CustomProjectPart::saveProject()
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


QString CustomProjectPart::projectDirectory() const
{
    return m_projectDirectory;
}


QString CustomProjectPart::projectName() const
{
    return m_projectName;
}


/** Retuns a PairList with the run environment variables */
DomUtil::PairList CustomProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry(*projectDom(), "/kdevcustomproject/run/envvars", "envvar", "name", "value");
}


/** Retuns the currently selected run directory
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The directory where the executable is
  *   if run/directoryradio == build
  *        The directory where the executable is relative to build directory
  *   if run/directoryradio == custom
  *        The custom directory absolute path
  */
QString CustomProjectPart::runDirectory() const
{
    QDomDocument &dom = *projectDom();

    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevcustomproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevcustomproject/run/mainprogram");

    if ( directoryRadioString == "build" )
        return buildDirectory();

    if ( directoryRadioString == "custom" )
        return DomUtil::readEntry(dom, "/kdevcustomproject/run/customdirectory");

    int pos = DomMainProgram.findRev('/');
    if (pos != -1)
        return buildDirectory() + "/" + DomMainProgram.left(pos);

    return buildDirectory() + "/" + DomMainProgram;

}


/** Retuns the currently selected main program
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The executable name
  *   if run/directoryradio == build
  *        The path to executable relative to build directory
  *   if run/directoryradio == custom or relative == false
  *        The absolute path to executable
  */
QString CustomProjectPart::mainProgram(bool relative) const
{
    QDomDocument &dom = *projectDom();

    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevcustomproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevcustomproject/run/mainprogram");

    if ( directoryRadioString == "custom" )
        return DomMainProgram;

    if ( relative == false )
        return buildDirectory() + "/" + DomMainProgram;

    if ( directoryRadioString == "executable" ) {
        int pos = DomMainProgram.findRev('/');
        if (pos != -1)
            return DomMainProgram.mid(pos+1);
        return DomMainProgram;
    }
    else
        return DomMainProgram;
}

/** Retuns a QString with the run command line arguments */
QString CustomProjectPart::runArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevcustomproject/run/programargs");
}

QString CustomProjectPart::activeDirectory() const
{
    QDomDocument &dom = *projectDom();
    return DomUtil::readEntry(dom, "/kdevcustomproject/general/activedir");
}


QStringList CustomProjectPart::allFiles() const
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
	
	saveProject();

	kdDebug(9025) << "Emitting addedFilesToProject" << endl;
	emit addedFilesToProject ( fileList );
}

void CustomProjectPart::removeFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );

	this->removeFiles( fileList );
}

void CustomProjectPart::removeFiles ( const QStringList& fileList )
{
	kdDebug(9025) << "Emitting removedFilesFromProject" << endl;
	emit removedFilesFromProject ( fileList );

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		m_sourceFiles.remove ( *it );
	}
	
	saveProject();
}

QString CustomProjectPart::buildDirectory() const
{
    QString dir = DomUtil::readEntry(*projectDom(), "/kdevcustomproject/build/builddir");
    return dir.isEmpty()? projectDirectory() : dir;
}


QString CustomProjectPart::makeEnvironment() const
{
    // Get the make environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*projectDom(), "/kdevcustomproject/make/environments/" + currentMakeEnvironment(), "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
        environstr += EnvVarTools::quote((*it).second);
        environstr += " ";
    }
    return environstr;
}


void CustomProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QDomDocument &dom = *projectDom();
    bool ant = DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "ant";

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
        cmdline += " " + DomUtil::readEntry(dom, "/kdevcustomproject/make/makeoptions");
    }

    cmdline += " ";
    cmdline += target;

    QString dircmd = "cd ";
    dircmd += KProcess::quote(dir);
    dircmd += " && ";

    int prio = DomUtil::readIntEntry(dom, "/kdevcustomproject/make/prio");
    QString nice;
    if (prio != 0) {
        nice = QString("nice -n%1 ").arg(prio);
    }

    cmdline.prepend(nice);
    cmdline.prepend(makeEnvironment());
    m_buildCommand = dircmd + cmdline;
    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}


void CustomProjectPart::slotBuild()
{
    m_lastCompilationFailed = false;
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
    QString baseName = fi.baseName(true);
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

    if( DomUtil::readBoolEntry(*projectDom(), "/kdevcustomproject/run/autocompile", true) && isDirty() ){
        m_executeAfterBuild = true;
        slotBuild();
        return;
    }

    // Get the run environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars = runEnvironmentVars();
    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
        environstr += EnvVarTools::quote((*it).second);
        environstr += " ";
    }

    if (mainProgram(true).isEmpty())
    // Do not execute non executable targets
        return;

    QString program = environstr;
    // Adds the ./ that is necessary to execute the program in bash shells
    if (!mainProgram(true).startsWith("/"))
        program += "./";
    program += mainProgram(true);
    program += " " + runArguments();

    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevcustomproject/run/terminal");

    kdDebug(9025) << "runDirectory: <" << runDirectory() << ">" <<endl;
    kdDebug(9025) << "environstr  : <" << environstr << ">" <<endl;
    kdDebug(9025) << "mainProgram : <" << mainProgram(true) << ">" <<endl;
    kdDebug(9025) << "runArguments: <" << runArguments() << ">" <<endl;

    appFrontend()->startAppCommand(runDirectory(), program, inTerminal);
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

void CustomProjectPart::updateMakeEnvironmentsMenu()
{
    QDomDocument &dom = *projectDom();
    bool makeUsed = (DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "make");
    if (makeUsed) {
        QStringList l = allMakeEnvironments();
        m_makeEnvironmentsSelector->setItems(l);
        m_makeEnvironmentsSelector->setCurrentItem(l.findIndex(currentMakeEnvironment()));
    }
    else {
        m_makeEnvironmentsSelector->clear();
    }
    /*
    m_makeEnvironmentsMenu->clear();
    QDomDocument &dom = *projectDom();

        QStringList environments = allMakeEnvironments();
        QStringList::ConstIterator it;
        int id = 0;
        for (it = environments.begin(); it != environments.end(); ++it)
            m_makeEnvironmentsMenu->insertItem(*it, id++);
    }
    */
}

void CustomProjectPart::makeEnvironmentsMenuActivated(int id)
{
    QDomDocument &dom = *projectDom();
    QString environment = allMakeEnvironments()[id];
    DomUtil::writeEntry(dom, "/kdevcustomproject/make/selectedenvironment", environment);
}

void CustomProjectPart::slotCommandFinished( const QString& command )
{
    kdDebug(9020) << "CustomProjectPart::slotProcessFinished()" << endl;

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

void CustomProjectPart::slotCommandFailed( const QString& /*command*/ )
{
    kdDebug(9020) << k_funcinfo << endl;

    m_lastCompilationFailed = true;
}

bool CustomProjectPart::isDirty()
{
    if (m_lastCompilationFailed) return true;

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


QStringList CustomProjectPart::allMakeEnvironments() const
{
    QDomDocument &dom = *projectDom();

    QStringList allConfigs;

    QDomNode node =
        DomUtil::elementByPath( dom , "/kdevcustomproject/make/environments");
    // extract the names of the different make environments
    QDomElement childEl = node.firstChild().toElement();
    while (!childEl.isNull()) {
        QString config = childEl.tagName();
        allConfigs.append(config);
        childEl = childEl.nextSibling().toElement();
    }
    if (allConfigs.isEmpty())
        allConfigs.append("default");

    return allConfigs;
}


QString CustomProjectPart::currentMakeEnvironment() const
{
    QStringList allEnvs = allMakeEnvironments();
    QDomDocument &dom = *projectDom();
    QString environment = DomUtil::readEntry(dom, "/kdevcustomproject/make/selectedenvironment");
    if (environment.isEmpty() || !allEnvs.contains(environment ))
        environment  = allEnvs[0];
    return environment;
}


#include "customprojectpart.moc"


/*!
    \fn CustomProjectPart::distFiles() const
 */
QStringList CustomProjectPart::distFiles() const
{
   	QStringList sourceList = allFiles();
	// Scan current source directory for any .pro files.
	QString projectDir = projectDirectory();
	QDir dir(projectDir);
	QStringList files = dir.entryList( "*README*");
	return sourceList + files;
}
