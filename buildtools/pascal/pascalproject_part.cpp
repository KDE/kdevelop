/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qdom.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qvaluestack.h>
#include <qregexp.h>
#include <qvbox.h>
#include <qlabel.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <kservice.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kprocess.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevcompileroptions.h"
#include "runoptionswidget.h"
#include "envvartools.h"

#include "pascalproject_widget.h"
#include "pascalproject_part.h"
#include "pascalprojectoptionsdlg.h"
#include "pascalglobaloptionsdlg.h"

#include <kdevplugininfo.h>

typedef KDevGenericFactory<PascalProjectPart> PascalProjectFactory;
static const KDevPluginInfo data("kdevpascalproject");
K_EXPORT_COMPONENT_FACTORY( libkdevpascalproject, PascalProjectFactory( data ) )

PascalProjectPart::PascalProjectPart(QObject *parent, const char *name, const QStringList& )
    :KDevBuildTool(&data, parent, name ? name : "PascalProjectPart" )
{
    setInstance(PascalProjectFactory::instance());
    setXMLFile("kdevpascalproject.rc");

    KAction *action;
    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    action->setToolTip(i18n("Build project"));
    action->setWhatsThis(i18n("<b>Build project</b><p>Runs the compiler on a main source file of the project. "
        "The compiler and the main source file can be set in project settings, <b>Pascal Compiler</b> tab."));
    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_execute" );
    action->setToolTip(i18n("Execute program"));
    action->setWhatsThis(i18n("<b>Execute program</b><p>Executes the main program specified in project settings, <b>Run options</b> tab. "
        "If nothing is set, the binary file with the same name as the main source file name is executed."));

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );

//  m_widget = new PascalProjectWidget(this);

//  QWhatsThis::add(m_widget, i18n("WHAT DOES THIS PART DO?"));

  // now you decide what should happen to the widget. Take a look at kdevcore.h
  // or at other plugins how to embed it.

  // if you want to embed your widget as an outputview, simply uncomment
  // the following line.

  // mainWindow()->embedOutputView( m_widget, "name that should appear", "enter a tooltip" );

}

PascalProjectPart::~PascalProjectPart()
{
//  delete m_widget;
}

/**
 * @todo This should really be merged with FileTreeWidget::matchesHidePattern()
 * and put in its own class. Currently this is repeated in scriptprojectpart.cpp, pascalproject_part.cpp, adaproject_part.cpp
 */
static bool matchesPattern(const QString &fileName, const QStringList &patternList)
{
    QStringList::ConstIterator it;
    for (it = patternList.begin(); it != patternList.end(); ++it) {
        QRegExp re(*it, true, true);
        if (re.search(fileName) == 0 && re.matchedLength() == (int)fileName.length())
            return true;
    }

    return false;
}

void PascalProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_buildDir = dirName;
    m_projectDir = dirName;
    m_projectName = projectName;

    QDomDocument &dom = *projectDom();
    // Set the default directory radio to "executable"
    if (DomUtil::readEntry(dom, "/kdevpascalproject/run/directoryradio") == "" ) {
        DomUtil::writeEntry(dom, "/kdevpascalproject/run/directoryradio", "executable");
    }

    loadProjectConfig();

    // Put all files from all subdirectories into file list
    QValueStack<QString> s;
    int prefixlen = m_projectDir.length()+1;
    s.push(m_projectDir);

    QStringList includepatternList;
    if ( languageSupport() )
    {
	KMimeType::List list = languageSupport()->mimeTypes();
	KMimeType::List::Iterator it = list.begin();
	while( it != list.end() ){
	    includepatternList += (*it)->patterns();
	    ++it;
	}
    }
    QString excludepatterns = "*~";
    QStringList excludepatternList = QStringList::split(",", excludepatterns);

    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9033) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        QPtrListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug(9033) << "Pushing: " << path << endl;
                s.push(path);
            }
            else {
                if (matchesPattern(path, includepatternList)
                    && !matchesPattern(path, excludepatternList)) {
                    kdDebug(9033) << "Adding: " << path << endl;
                    m_sourceFiles.append(path.mid(prefixlen));
                } else {
                    kdDebug(9033) << "Ignoring: " << path << endl;
                }
            }
        }
    } while (!s.isEmpty());
    
    KDevProject::openProject( dirName, projectName );
}

void PascalProjectPart::closeProject()
{
}

/** Retuns a PairList with the run environment variables */
DomUtil::PairList PascalProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry(*projectDom(), "/kdevpascalproject/run/envvars", "envvar", "name", "value");
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
QString PascalProjectPart::runDirectory() const
{
    QDomDocument &dom = *projectDom();

    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevpascalproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevpascalproject/run/mainprogram");

    if ( directoryRadioString == "build" )
        return buildDirectory();

    if ( directoryRadioString == "custom" )
        return DomUtil::readEntry(dom, "/kdevpascalproject/run/customdirectory");

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
QString PascalProjectPart::mainProgram(bool relative) const
{
    QDomDocument &dom = *projectDom();
    QString configMainProg = DomUtil::readEntry(dom, "/kdevpascalproject/run/mainprogram", "");
    if (configMainProg.isEmpty())
    {
        QFileInfo fi(mainSource());
        return buildDirectory() + "/" + fi.baseName();
    }
    else
        return QDir::cleanDirPath(projectDirectory() + "/" + configMainProg);

    /// \FIXME put the code below into use!
    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevpascalproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevpascalproject/run/mainprogram");

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
QString PascalProjectPart::runArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevpascalproject/run/programargs");
}

QString PascalProjectPart::mainSource() const
{
    return projectDirectory() + "/" + m_mainSource;
}

void PascalProjectPart::setMainSource(QString fullPath)
{
    m_mainSource = fullPath.replace(QRegExp(QString(projectDirectory() + QString("/"))),"");
}

QString PascalProjectPart::projectDirectory() const
{
    return m_projectDir;
}

QString PascalProjectPart::projectName() const
{
    return m_projectName;
}

QString PascalProjectPart::activeDirectory() const
{
    QFileInfo fi(mainSource());
    return fi.dirPath(true).replace(QRegExp(projectDirectory()),"");
}

QString PascalProjectPart::buildDirectory() const
{
    QFileInfo fi(mainSource());
    return fi.dirPath(true);
}

void PascalProjectPart::listOfFiles(QStringList &result, QString path) const
{
    QDir d(path);
    if (!d.exists())
        return;
    QFileInfoList *entries = const_cast<QFileInfoList*>(d.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden));
    for (QFileInfo *it = entries->first(); it; it = entries->next())
    {
        if ((it->isDir()) && (it->filePath() != path))
        {
//            qWarning("entering dir %s", it->dirPath().latin1());
            listOfFiles(result, it->dirPath());
        }
        else
        {
//            qWarning("adding to result: %s", it->filePath().latin1());
            result << it->filePath();
        }
    }
}

QStringList PascalProjectPart::allFiles() const
{
//    QStringList files;

//    listOfFiles(files, projectDirectory());

//    return files;
    return m_sourceFiles;
}

void PascalProjectPart::addFile(const QString& /*fileName*/)
{
}

void PascalProjectPart::addFiles(const QStringList& /*fileList*/)
{
}

void PascalProjectPart::removeFile(const QString& /*fileName*/)
{
}

void PascalProjectPart::removeFiles(const QStringList& /*fileList*/)
{
}

void PascalProjectPart::slotBuild()
{
    if (partController()->saveAllFiles()==false)
       return; //user cancelled

    QString cmdline = m_compilerExec + " " + m_compilerOpts + " ";

    if (cmdline.isEmpty())
    {
        KMessageBox::sorry(0, i18n("Could not find pascal compiler.\nCheck if your compiler settings are correct."));
        return;
    }

    QFileInfo fi(mainSource());
    cmdline += fi.fileName();

    QString dircmd = "cd ";
    dircmd += KProcess::quote(buildDirectory());
    dircmd += " && ";

    makeFrontend()->queueCommand(buildDirectory(), dircmd + cmdline);
}

void PascalProjectPart::slotExecute()
{
    partController()->saveAllFiles();

    QDomDocument &dom = *(projectDom());
    bool runInTerminal = DomUtil::readBoolEntry(dom, "/kdevpascalproject/run/terminal", true);

    // Get the run environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*projectDom(), "/kdevpascalproject/run/envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
/*
#if (KDE_VERSION > 305)
        environstr += KProcess::quote((*it).second);
#else
        environstr += KShellProcess::quote((*it).second);
#endif
*/
        environstr += EnvVarTools::quote((*it).second);
        environstr += " ";
    }

    QString program = mainProgram();
    program.prepend(environstr);
    program += " " + DomUtil::readEntry(*projectDom(), "/kdevpascalproject/run/programargs");

    appFrontend()->startAppCommand(buildDirectory(), program, runInTerminal);
}

void PascalProjectPart::changedFiles( const QStringList & fileList )
{
    KDevProject::changedFiles(fileList);
}

void PascalProjectPart::changedFile( const QString & fileName )
{
    KDevProject::changedFile(fileName);
}

void PascalProjectPart::projectConfigWidget( KDialogBase * dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Pascal Compiler"));
    PascalProjectOptionsDlg *w = new PascalProjectOptionsDlg(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(okClicked()), this, SLOT(loadProjectConfig()) );

    vbox = dlg->addVBoxPage(i18n("Run Options"), i18n("Run Options"), BarIcon( "make", KIcon::SizeMedium ));
    RunOptionsWidget *w3 = new RunOptionsWidget(*projectDom(), "/kdevpascalproject", buildDirectory(), vbox);
    w3->mainprogram_label->setText(i18n("Main program (relative to project directory):"));
    connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );

}

void PascalProjectPart::loadProjectConfig( )
{
    QDomDocument &dom = *(projectDom());

    QString config = DomUtil::readEntry(dom, "/kdevpascalproject/general/useconfiguration", "default");
    m_mainSource = DomUtil::readEntry(dom, QString("/kdevpascalproject/configurations/") + config + QString("/mainsource") );
    m_compilerOpts = DomUtil::readEntry(dom, QString("/kdevpascalproject/configurations/") + config + QString("/compileroptions"));
    m_compilerExec = DomUtil::readEntry(dom, QString("/kdevpascalproject/configurations/") + config + QString("/compilerexec"));

    if (m_compilerExec.isEmpty())
    {
        KTrader::OfferList offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Pascal'");
        QValueList<KService::Ptr>::ConstIterator it;
        for (it = offers.begin(); it != offers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                m_compilerExec = (*it)->exec();
                break;
            }
        }
    }
}

void PascalProjectPart::configWidget( KDialogBase * dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Pascal Compiler"));
    PascalGlobalOptionsDlg *w = new PascalGlobalOptionsDlg(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

KDevCompilerOptions *PascalProjectPart::createCompilerOptions(const QString &name)
{
    KService::Ptr service = KService::serviceByDesktopName(name);
    if (!service) {
        kdDebug() << "Can't find service " << name;
        return 0;
    }

    KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
    if (!factory) {
        QString errorMessage = KLibLoader::self()->lastErrorMessage();
        KMessageBox::error(0, i18n("There was an error loading the module %1.\n"
                                   "The diagnostics is:\n%2").arg(service->name()).arg(errorMessage));
        exit(1);
    }

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(this, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug() << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;

    return dlg;
}

QString PascalProjectPart::defaultOptions( const QString compiler ) const
{
    KConfig *config = KGlobal::config();
    config->setGroup("Pascal Compiler");
    return config->readPathEntry(compiler);
}

#include "pascalproject_part.moc"


/*!
    \fn PascalProjectPart::distFiles() const
 */
QStringList PascalProjectPart::distFiles() const
{
   	QStringList sourceList = allFiles();
	// Scan current source directory for any .pro files.
	QString projectDir = projectDirectory();
	QDir dir(projectDir);
	QStringList files = dir.entryList( "Makefile");
	return sourceList + files;
}
