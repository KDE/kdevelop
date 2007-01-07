/* Copyright (C) 2003 Oliver Kellogg
 * okellogg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "adaproject_part.h"

#include <qdom.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qvaluestack.h>
#include <qregexp.h>
#include <qvbox.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <kprocess.h>
#include <kservice.h>
#include <kconfig.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevcompileroptions.h"
#include "kdevgenericfactory.h"
#include <kdevplugininfo.h>

#include "adaproject_widget.h"
#include "adaprojectoptionsdlg.h"
#include "adaglobaloptionsdlg.h"

typedef KDevGenericFactory<AdaProjectPart> AdaProjectFactory;
static const KDevPluginInfo data("kdevadaproject");
K_EXPORT_COMPONENT_FACTORY( libkdevadaproject, AdaProjectFactory( data ) )

AdaProjectPart::AdaProjectPart(QObject *parent, const char *name, const QStringList& )
    :KDevBuildTool(&data, parent, name ? name : "AdaProjectPart" )
{
    setInstance(AdaProjectFactory::instance());
    setXMLFile("kdevadaproject.rc");

    KAction *action;
    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_execute" );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );

//  m_widget = new AdaProjectWidget(this);

//  QWhatsThis::add(m_widget, i18n("WHAT DOES THIS PART DO?"));

  // now you decide what should happen to the widget. Take a look at kdevcore.h
  // or at other plugins how to embed it.

  // if you want to embed your widget as an outputview, simply uncomment
  // the following line.

  // mainWindow()->embedOutputView( m_widget, "name that should appear", "enter a tooltip" );

}

AdaProjectPart::~AdaProjectPart()
{
//  delete m_widget;
}

/**
 * This should really be merged with FileTreeWidget::matchesHidePattern()
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

void AdaProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_buildDir = dirName;
    m_projectDir = dirName;
    m_projectName = projectName;

    QDomDocument &dom = *projectDom();
    // Set the default directory radio to "executable"
    if (DomUtil::readEntry(dom, "/kdevadaproject/run/directoryradio") == "" ) {
        DomUtil::writeEntry(dom, "/kdevadaproject/run/directoryradio", "executable");
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
        kdDebug() << "AdaProjectPart::openProject examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
	if( !dirEntries )
	    break;

        QPtrListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug() << "AdaProjectPart::openProject pushing: " << path << endl;
                s.push(path);
            }
            else {
                if (matchesPattern(path, includepatternList)
                    && !matchesPattern(path, excludepatternList)) {
                    kdDebug() << "AdaProjectPart::openProject adding: " << path << endl;
                    m_sourceFiles.append(path.mid(prefixlen));
                } else {
                    kdDebug() << "AdaProjectPart::openProject ignoring: " << path << endl;
                }
            }
        }
    } while (!s.isEmpty());

    KDevProject::openProject( dirName, projectName );
}

void AdaProjectPart::closeProject()
{
}

/** Retuns a PairList with the run environment variables */
DomUtil::PairList AdaProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry(*projectDom(), "/kdevadaproject/run/envvars", "envvar", "name", "value");
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
QString AdaProjectPart::runDirectory() const
{
    QString cwd = defaultRunDirectory("kdevadaproject");
    if (cwd.isEmpty())
      cwd = buildDirectory();
    return cwd;
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
QString AdaProjectPart::mainProgram() const
{
    QDomDocument * dom = projectDom();

    if ( !dom ) return QString();

    QString DomMainProgram = DomUtil::readEntry( *dom, "/kdevadaproject/run/mainprogram");

    if ( DomMainProgram.isEmpty() ) return QString();

    if ( DomMainProgram.startsWith("/") )   // assume absolute path
    {
        return DomMainProgram;
    }
    else // assume project relative path
    {
        return projectDirectory() + "/" + DomMainProgram;
    }

    return QString();
}

/** Retuns a QString with the run command line arguments */
QString AdaProjectPart::debugArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevadaproject/run/globaldebugarguments");
}


/** Retuns a QString with the run command line arguments */
QString AdaProjectPart::runArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevadaproject/run/programargs");
}

QString AdaProjectPart::mainSource() const
{
    return projectDirectory() + "/" + m_mainSource;
}

void AdaProjectPart::setMainSource(QString fullPath)
{
    m_mainSource = fullPath.replace(QRegExp(QString(projectDirectory() + QString("/"))),"");
}

QString AdaProjectPart::projectDirectory() const
{
    return m_projectDir;
}

QString AdaProjectPart::projectName() const
{
    return m_projectName;
}

QString AdaProjectPart::activeDirectory() const
{
    QFileInfo fi(mainSource());
    return fi.dirPath(true).replace(QRegExp(projectDirectory()),"");
}

QString AdaProjectPart::buildDirectory() const
{
    QFileInfo fi(mainSource());
    return fi.dirPath(true);
}

void AdaProjectPart::listOfFiles(QStringList &result, QString path) const
{
    QDir d(path);
    if (!d.exists())
        return;

    const QFileInfoList *entries = d.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden);
    if( !entries )
        return;

    QFileInfoListIterator it( *entries );
    while( const QFileInfo* fileInfo = it.current() )
    {
        ++it;

        if (fileInfo->isDir() && fileInfo->filePath() != path)
        {
            kdDebug() << "entering dir " << fileInfo->dirPath() << endl;
            listOfFiles(result, fileInfo->dirPath());
        }
        else
        {
            kdDebug() << "adding to result: " << fileInfo->filePath() << endl;
            result << fileInfo->filePath();
        }
    }
}

QStringList AdaProjectPart::allFiles() const
{
//    QStringList files;

//    listOfFiles(files, projectDirectory());

//    return files;
    return m_sourceFiles;
}

void AdaProjectPart::addFile(const QString& /*fileName*/)
{
}

void AdaProjectPart::addFiles(const QStringList& /*fileList*/)
{
}

void AdaProjectPart::removeFile(const QString& /*fileName*/)
{
}

void AdaProjectPart::removeFiles(const QStringList& /*fileList*/)
{
}

void AdaProjectPart::slotBuild()
{
    if (partController()->saveAllFiles()==false)
       return; //user cancelled

    QString cmdline = m_compilerExec + " " + m_compilerOpts + " ";

    if (cmdline.isEmpty())
    {
        KMessageBox::sorry(0, i18n("Could not find ada compiler.\nCheck if your compiler settings are correct."));
        return;
    }

    QFileInfo fi(mainSource());
    cmdline += fi.fileName();

    QString dircmd = "cd ";
    dircmd += KProcess::quote(buildDirectory());
    dircmd += " && ";

    makeFrontend()->queueCommand(buildDirectory(), dircmd + cmdline);
}

void AdaProjectPart::slotExecute()
{
    partController()->saveAllFiles();
    QString program = "./";
    appFrontend()->startAppCommand(buildDirectory(), mainProgram(), true);
}

void AdaProjectPart::changedFiles( const QStringList & fileList )
{
    KDevProject::changedFiles(fileList);
}

void AdaProjectPart::changedFile( const QString & fileName )
{
    KDevProject::changedFile(fileName);
}

void AdaProjectPart::projectConfigWidget( KDialogBase * dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Ada Compiler"));
    AdaProjectOptionsDlg *w = new AdaProjectOptionsDlg(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(okClicked()), this, SLOT(loadProjectConfig()) );
}

void AdaProjectPart::loadProjectConfig( )
{
    QDomDocument &dom = *(projectDom());

    QString config = DomUtil::readEntry(dom, "/kdevadaproject/general/useconfiguration", "default");
    m_mainSource = DomUtil::readEntry(dom, QString("/kdevadaproject/configurations/") + config + QString("/mainsource") );
    m_compilerOpts = DomUtil::readEntry(dom, QString("/kdevadaproject/configurations/") + config + QString("/compileroptions"));
    m_compilerExec = DomUtil::readEntry(dom, QString("/kdevadaproject/configurations/") + config + QString("/compilerexec"));

    if (m_compilerExec.isEmpty())
    {
        KTrader::OfferList offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Ada'");
        QValueList<KService::Ptr>::ConstIterator it;
        for (it = offers.begin(); it != offers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                m_compilerExec = (*it)->exec();
                break;
            }
        }
    }
}

void AdaProjectPart::configWidget( KDialogBase * dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Ada Compiler"));
    AdaGlobalOptionsDlg *w = new AdaGlobalOptionsDlg(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

KDevCompilerOptions *AdaProjectPart::createCompilerOptions(const QString &name)
{
    KService::Ptr service = KService::serviceByDesktopName(name);
    if (!service) {
        kdDebug() << "AdaProjectPart::createCompilerOptions can't find service " << name;
        return 0;
    }

    KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
    if (!factory) {
        QString errorMessage = KLibLoader::self()->lastErrorMessage();
        KMessageBox::error(0, i18n("There was an error loading the module %1.\n"
                                   "The diagnostics are:\n%2").arg(service->name()).arg(errorMessage));
        exit(1);
    }

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(this, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug() << "AdaProjectPart::createCompilerOptions: component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;

    return dlg;
}

QString AdaProjectPart::defaultOptions( const QString compiler )
{
    KConfig *config = KGlobal::config();
    config->setGroup("Ada Compiler");
    return config->readPathEntry(compiler);
}

#include "adaproject_part.moc"


/*!
    \fn AdaProjectPart::distFiles() const
 */
QStringList AdaProjectPart::distFiles() const
{
	QStringList sourceList = allFiles();
	// Scan current source directory for any .pro files.
	QString projectDir = projectDirectory();
	QDir dir(projectDir);
	QStringList files = dir.entryList( "Makefile");
	return sourceList + files;
}
