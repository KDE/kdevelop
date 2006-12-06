/***************************************************************************
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jakob@jsg.dk                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "trollprojectpart.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdirwatch.h>
#include <kstatusbar.h>
#include <qmessagebox.h>
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kprocess.h>
#include <kurlrequesterdlg.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kfile.h>
#include <makeoptionswidget.h>


#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "trollprojectwidget.h"
#include "runoptionswidget.h"
#include "config.h"
#include "envvartools.h"
#include "qmakeoptionswidget.h"

#include <kdevplugininfo.h>

typedef KDevGenericFactory<TrollProjectPart> TrollProjectFactory;
static const KDevPluginInfo data("kdevtrollproject");
K_EXPORT_COMPONENT_FACTORY( libkdevtrollproject, TrollProjectFactory( data ) )

TrollProjectPart::TrollProjectPart(QObject *parent, const char *name, const QStringList& args )
    : KDevBuildTool(&data, parent, name ? name : "TrollProjectPart")
{
    setInstance(TrollProjectFactory::instance());

    if ( args.count() == 1 && args[0] == "TMake" )
        m_tmakeProject = true;
    else
        m_tmakeProject = false;

    setXMLFile("kdevtrollproject.rc");

    m_executeAfterBuild = false;

    m_dirWatch = new KDirWatch(this);

    m_widget = new TrollProjectWidget(this);
    m_widget->setIcon(SmallIcon("qmakerun"));
    m_widget->setCaption(i18n("QMake Manager"));
    QWhatsThis::add(m_widget, i18n("<b>QMake manager</b><p>"
                                   "The QMake manager project tree consists of two parts. The 'overview' "
                                   "in the upper half shows the subprojects, each one having a "
                                   ".pro file. The 'details' view in the lower half shows the "
                                   "list of files for the active subproject selected in the overview."));

    mainWindow()->embedSelectViewRight(m_widget, i18n("QMake Manager"), i18n("QMake manager"));

    KAction *action;

    const QIconSet icon(SmallIcon("compfile"));
    action = new KAction( i18n("Compile &File"), "compfile", 0,
                          m_widget, SLOT(slotBuildOpenFile()),
                          actionCollection(),"build_compilefile"  );
    action->setToolTip(i18n("Compile file"));
    action->setWhatsThis(i18n("<b>Compile file</b><p>Runs <b>make filename.o</b> command from the directory where 'filename' is the name of currently opened file.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));


    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          m_widget, SLOT(slotBuildProject()),
                          actionCollection(), "build_build_project" );
    action->setToolTip(i18n("Build project"));
    action->setWhatsThis(i18n("<b>Build project</b><p>Runs <b>make</b> from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("&Rebuild Project"),"rebuild" , 0,
                          m_widget, SLOT(slotRebuildProject()),
                          actionCollection(),"build_rebuild_project"  );
    action->setToolTip(i18n("Rebuild project"));
    action->setWhatsThis(i18n("<b>Rebuild project</b><p>Runs <b>make clean</b> and then <b>make</b> from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("&Clean Project"), 0,
                          m_widget, SLOT(slotCleanProject()),
                          actionCollection(), "build_clean_project" );
    action->setToolTip(i18n("Clean project"));
    action->setWhatsThis(i18n("<b>Clean project</b><p>Runs <b>make clean</b> command from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("&Dist-Clean Project"), 0,
                          m_widget, SLOT(slotDistCleanProject()),
                          actionCollection(), "build_distclean_project" );
    action->setToolTip(i18n("Dist-Clean project"));
    action->setWhatsThis(i18n("<b>Dist-Clean project</b><p>Runs <b>make distclean</b> command from the "
                              "project directory.<br>Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("Execute Main Program"), "exec", SHIFT+Key_F9,
                          this, SLOT(slotBuildAndExecuteProject()),
                          actionCollection(), "build_execute_project" );
    action->setToolTip(i18n("Execute main program"));
    action->setWhatsThis(i18n("<b>Execute program</b><p>Executes the main program specified in project settings, <b>Run Options</b> tab."));

    action = new KAction( i18n("&Build Subproject"), "make_kdevelop", Key_F7,
                          m_widget, SLOT(slotBuildTarget()),
                          actionCollection(), "build_build_target" );
    action->setToolTip(i18n("Build subproject"));
    action->setWhatsThis(i18n("<b>Build subproject</b><p>Runs <b>make</b> from the current subproject directory. "
                              "Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("&Rebuild Subproject"), "rebuild", 0,
                          m_widget, SLOT(slotRebuildTarget()),
                          actionCollection(),"build_rebuild_target"  );
    action->setToolTip(i18n("Rebuild subproject"));
    action->setWhatsThis(i18n("<b>Rebuild subproject</b><p>Runs <b>make clean</b> and then <b>make</b> from the current subproject directory. "
                              "Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("&Clean Subproject"), 0,
                          m_widget, SLOT(slotCleanTarget()),
                          actionCollection(), "build_clean_target" );
    action->setToolTip(i18n("Clean subproject"));
    action->setWhatsThis(i18n("<b>Clean subproject</b><p>Runs <b>make clean</b> from the current subproject directory. "
                              "Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("&Dist-Clean Subproject"), 0,
                          m_widget, SLOT(slotDistCleanTarget()),
                          actionCollection(), "build_distclean_target" );
    action->setToolTip(i18n("Dist-Clean subproject"));
    action->setWhatsThis(i18n("<b>Dist-Clean subproject</b><p>Runs <b>make distclean</b> from the current"
                              " subproject directory. Current subproject is a subproject selected in <b>QMake manager</b> 'overview' window.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));

    action = new KAction( i18n("Execute Subproject"), "exec", 0,
                          this, SLOT(slotBuildAndExecuteTarget()),
                          actionCollection(), "build_execute_target" );
    action->setToolTip(i18n("Execute subproject"));
    action->setWhatsThis(i18n("<b>Execute subproject</b><p>Executes the target program for the currently selected subproject. "
        "This action is allowed only if a type of the subproject is 'application'. The type of the subproject can be "
        "defined in <b>Subproject Settings</b> dialog (open it from the subproject context menu)."));

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    connect( makeFrontend(), SIGNAL(commandFinished(const QString&)),
             this, SLOT(slotCommandFinished(const QString&)) );

    QString m_defaultQtDir = DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/root", "");
    QString m_qmakePath = DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/qmake", "");

    if( m_defaultQtDir.isEmpty() || !isValidQtDir( m_defaultQtDir ) )
    {
        m_defaultQtDir = findQtDir();
        kdDebug(9024) << "Setting default dir to: " << m_defaultQtDir << endl;
        DomUtil::writeEntry(*projectDom(), "/kdevcppsupport/qt/root", m_defaultQtDir );
    }
    if( m_qmakePath.isEmpty() || !isExecutable( m_qmakePath ) )
    {
        m_qmakePath = findExecutable( "qmake" );
        kdDebug(9024) << "Setting qmake binary to: " << m_qmakePath << endl;
        DomUtil::writeEntry(*projectDom(), "/kdevcppsupport/qt/qmake", m_qmakePath );
    }
}


TrollProjectPart::~TrollProjectPart()
{
    if (m_widget)
        mainWindow()->removeView(m_widget);
    delete m_widget;
}

QString TrollProjectPart::makeEnvironment()
{
    // Get the make environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*projectDom(), "/kdevtrollproject/make/envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    bool hasQtDir = false;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        if( (*it).first == "QTDIR" )
	    hasQtDir = true;

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

    if( !hasQtDir && !isQt4Project() && !DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/root", "").isEmpty() )
    {
         environstr += QString( "QTDIR=" ) + EnvVarTools::quote( DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/root", "") ) + QString( " " );
    }

    return environstr;
}

void TrollProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Run Options"), i18n("Run Options"), BarIcon( "make", KIcon::SizeMedium ));
    RunOptionsWidget *optdlg = new RunOptionsWidget(*projectDom(), "/kdevtrollproject", buildDirectory(), vbox);

    vbox = dlg->addVBoxPage(i18n("Make Options"), i18n("Make Options"), BarIcon( "make", KIcon::SizeMedium ));
    MakeOptionsWidget *w4 = new MakeOptionsWidget(*projectDom(), "/kdevtrollproject", vbox);

    vbox = dlg->addVBoxPage(i18n("QMake Manager Options"), i18n("QMake Manager Options"), BarIcon( "make", KIcon::SizeMedium ));
    QMakeOptionsWidget *qm = new QMakeOptionsWidget(*projectDom(), "/kdevtrollproject", vbox);


    connect( dlg, SIGNAL(okClicked()), w4, SLOT(accept()) );
    connect( dlg, SIGNAL(okClicked()), qm, SLOT(accept()) );
    connect( dlg, SIGNAL(okClicked()), optdlg, SLOT(accept()) );
}


void TrollProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    mainWindow()->statusBar()->message( i18n("Loading Project...") );

    QString defaultQtDir = DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/root", "");
    if( !isQt4Project() && ( defaultQtDir.isEmpty() || !isValidQtDir( defaultQtDir ) ) )
    {
        bool doask = true;
        while( doask )
        {
            KURLRequesterDlg dlg( i18n("Choose Qt3 directory"),
                i18n("Choose the Qt3 directory to use. This directory needs to have an include directory containing qt.h.")
                                  , m_widget, 0);
            dlg.urlRequester() ->setMode( KFile::Directory | KFile::LocalOnly );
            dlg.urlRequester() ->setURL( QString::null );
            dlg.urlRequester() ->completionObject() ->setDir( "/" );

            if ( dlg.exec() == QDialog::Accepted && !dlg.urlRequester() ->url().isEmpty() )
            {
                QString qtdir = dlg.urlRequester()->url();
                if( !isValidQtDir( qtdir ) )
                {
                    if( KMessageBox::warningYesNo( m_widget,
                                                i18n("The directory you gave is not a proper Qt directory, the "
                                                    "project might not work properly without one.\nPlease make "
                                                    "sure you give a directory that contains a bin with the "
                                                    "qmake binary in it and for Qt3 project also contains an "
                                                    "include directory with qt.h in it.\nDo you want to try "
                                                    "setting a Qt directory again?"),
                                                i18n("Wrong Qt directory given"))
                        == KMessageBox::Yes
                    )
                    doask = true;
                else
                    doask = false;
                }else
                {
                    defaultQtDir = qtdir;
                    doask = false;
                }

            }else
            {
                if( KMessageBox::warningYesNo( m_widget,
                                               i18n("You didn't specify a Qt directory, the project might not "
                                                   "work properly without one.\nDo you want to try setting a Qt"
                                                   " directory again?"),
                                               i18n("No Qt directory given"))
                        == KMessageBox::Yes
                    )
                    doask = true;
                else
                    doask = false;
            }
        }
    }
    QString qmakePath = DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/qmake", "");
    if( qmakePath.isEmpty() || !isExecutable( qmakePath ) )
    {
        bool doask = true;
        while( doask )
        {
            KURLRequesterDlg dlg( i18n("Choose QMake executable"),
                i18n("Choose the QMake binary to use. QMake is used to generate Makefiles from the project files."), m_widget, 0);
            dlg.urlRequester() ->setMode( KFile::Directory | KFile::LocalOnly );
            dlg.urlRequester() ->setURL( QString::null );
            dlg.urlRequester() ->completionObject() ->setDir( "/" );

            if ( dlg.exec() == QDialog::Accepted && !dlg.urlRequester() ->url().isEmpty() )
            {
                QString qmake = dlg.urlRequester()->url();
                if( !isExecutable( qmake ) )
                {
                    if( KMessageBox::warningYesNo( m_widget,
                                                i18n("The binary you gave is not executable, the "
                                                    "project might not work properly.\nPlease make "
                                                    "sure you give a qmake binary that is executable.\nDo you want to try "
                                                    "setting the QMake binary again?"),
                                                i18n("Wrong QMake binary given"))
                        == KMessageBox::Yes
                    )
                    doask = true;
                else
                    doask = false;
                }else
                {
                    qmakePath = qmake;
                    doask = false;
                }

            }else
            {
                if( KMessageBox::warningYesNo( m_widget,
                                               i18n("You didn't specify a QMake binary, the project might not "
                                                   "work properly without one.\nDo you want to try setting a QMake"
                                                   " binary again?"),
                                               i18n("No QMake binary given"))
                        == KMessageBox::Yes
                    )
                    doask = true;
                else
                    doask = false;
            }
        }
    }
    DomUtil::writeEntry( *projectDom(), "/kdevcppsupport/qt/root", defaultQtDir );
    DomUtil::writeEntry( *projectDom(), "/kdevcppsupport/qt/qmake", qmakePath );

    m_widget->openProject(dirName);

    m_projectName = projectName;

    QDomDocument &dom = *projectDom();
    // Set the default directory radio to "executable"
    if (DomUtil::readEntry(dom, "/kdevtrollproject/run/directoryradio") == "" ) {
        DomUtil::writeEntry(dom, "/kdevtrollproject/run/directoryradio", "executable");
    }

    KDevProject::openProject( dirName, projectName );
}


void TrollProjectPart::closeProject()
{
    m_widget->closeProject();
}


QString TrollProjectPart::projectDirectory() const
{
  return m_widget->projectDirectory();
}


QString TrollProjectPart::buildDirectory() const
{
  return m_widget->projectDirectory();
}

QString TrollProjectPart::projectName() const
{
    return m_projectName;
}


/** Retuns a PairList with the run environment variables */
DomUtil::PairList TrollProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry(*projectDom(), "/kdevtrollproject/run/envvars", "envvar", "name", "value");
}

void TrollProjectPart::slotBuildAndExecuteProject()
{
    partController()->saveAllFiles();
    if (isDirty()) {
        m_executeAfterBuild = true;
        m_widget->slotBuildProject();
    } else
        m_widget->slotExecuteProject();
}

void TrollProjectPart::slotBuildAndExecuteTarget()
{
    partController()->saveAllFiles();
    if (isDirty()) {
        m_executeAfterBuild = true;
        m_widget->slotBuildTarget();
    } else
        m_widget->slotExecuteTarget();
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
QString TrollProjectPart::runDirectory() const
{
    QDomDocument &dom = *projectDom();

    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevtrollproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevtrollproject/run/mainprogram");

    if ( directoryRadioString == "build" )
        return buildDirectory();

    if ( directoryRadioString == "custom" )
        return DomUtil::readEntry(dom, "/kdevtrollproject/run/customdirectory");

    int pos = DomMainProgram.findRev(QString( QChar( QDir::separator() ) ));
    if (pos != -1)
        return buildDirectory() + QString( QChar( QDir::separator() ) ) + DomMainProgram.left(pos);

    if ( DomMainProgram.isEmpty() )
    {
        return m_widget->subprojectDirectory();
    }
    return buildDirectory() + QString( QChar( QDir::separator() ) ) + DomMainProgram;

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
QString TrollProjectPart::mainProgram(bool relative) const
{
    QDomDocument &dom = *projectDom();

    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevtrollproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevtrollproject/run/mainprogram");

    if ( directoryRadioString == "custom" )
        return DomMainProgram;

    if ( relative == false && !DomMainProgram.isEmpty() )
        return buildDirectory() + QString( QChar( QDir::separator() ) ) + DomMainProgram;

    if ( directoryRadioString == "executable" ) {
        int pos = DomMainProgram.findRev(QString( QChar( QDir::separator() ) ));
        if (pos != -1)
            return DomMainProgram.mid(pos+1);

        if ( DomMainProgram.isEmpty() )
        {
            return runDirectory() + QString( QChar( QDir::separator() ) ) + m_widget->getCurrentOutputFilename();
        }
        return DomMainProgram;
    }
    else
        return DomMainProgram;
}


/** Retuns a QString with the run command line arguments */
QString TrollProjectPart::runArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevtrollproject/run/programargs");
}


QString TrollProjectPart::activeDirectory() const
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevtrollproject/general/activedir");
}


QStringList TrollProjectPart::allFiles() const
{
    return m_widget->allFiles();
}


void TrollProjectPart::addFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );

    this->addFiles ( QStringList( fileName ) );
}

void TrollProjectPart::addFiles ( const QStringList &fileList )
{
    QStringList files = fileList;
//     for (QStringList::iterator it = files.begin(); it != files.end(); ++it)
//     {
// //        if (!(*it).contains(projectDirectory()))
//       kdDebug(9024) << "Checking if isProjFile:" << (*it) << "?" << isProjectFile(projectDirectory() + QString( QChar( QDir::separator() ) ) + (*it)) << "|" << projectDirectory() << endl;
//         if (!isProjectFile(projectDirectory() + QString( QChar( QDir::separator() ) ) + (*it)))
//             *it = projectDirectory() + QString( QChar( QDir::separator() ) ) + (*it);
//     }
    m_widget->addFiles(files);

//	emit addedFilesToProject ( files );
}

void TrollProjectPart::removeFile(const QString & /* fileName */)
{
    /// \FIXME
/*	QStringList fileList;
	fileList.append ( fileName );

	this->removeFiles ( fileList );*/
}

void TrollProjectPart::removeFiles ( const QStringList& fileList )
{
/// \FIXME missing remove files functionality
// 	QStringList::ConstIterator it;
//
// 	it = fileList.begin();
//
// 	for ( ; it != fileList.end(); ++it )
// 	{
// 		FIXME
// 	}

	emit removedFilesFromProject ( fileList );
}
/*
void TrollProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QFileInfo fi(dir + "/Makefile");
    if (!fi.exists()) {
        int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory. Run qmake first?"), QString::null, i18n("Run qmake"), i18n("Do Not Run"));
        if (r == KMessageBox::No)
            return;
        startQMakeCommand(dir);
    }
    QDomDocument &dom = *projectDom();

    if (target=="clean")
    {
      QString cmdline = DomUtil::readEntry(dom, "/kdevtrollproject/make/makebin");
      if (cmdline.isEmpty())
          cmdline = MAKE_COMMAND;
      cmdline += " clean";
      QString dircmd = "cd ";
      dircmd += dir;
      dircmd += " && ";
      cmdline.prepend(makeEnvironment());
      makeFrontend()->queueCommand(dir, dircmd + cmdline);
    }

    QString cmdline = DomUtil::readEntry(dom, "/kdevtrollproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    if (!DomUtil::readBoolEntry(dom, "/kdevtrollproject/make/abortonerror"))
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry(dom, "/kdevtrollproject/make/numberofjobs");
    if (jobs != 0) {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(dom, "/kdevtrollproject/make/dontact"))
        cmdline += " -n";

    cmdline += " ";
    cmdline += target;

    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    cmdline.prepend(makeEnvironment());
    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}
*/

void TrollProjectPart::startQMakeCommand(const QString &dir)
{
    QFileInfo fi(dir);
    QString cmdline;

    if ( isTMakeProject() )
    {
    	cmdline = "tmake ";
    }else
    {
      cmdline = DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/qmake", "")+" ";
    }

    //QString cmdline = QString::fromLatin1( isTMakeProject() ? "tmake " : "qmake " );
//    cmdline += fi.baseName() + ".pro";
    QDir d(dir);
    QStringList l = d.entryList("*.pro");

    cmdline += l.count()?l[0]:(fi.baseName() + ".pro");

//    cmdline += QString::fromLatin1( " -o Makefile" );

    QString dircmd = "cd ";
    dircmd += KProcess::quote(dir);
    dircmd += " && ";

    cmdline.prepend(makeEnvironment());
    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}

void TrollProjectPart::queueCmd(const QString &dir, const QString &cmd)
{
    makeFrontend()->queueCommand(dir, cmd);
}

void TrollProjectPart::slotCommandFinished( const QString& command )
{
    Q_UNUSED( command );

//     if( m_buildCommand != command )
//         return;
//
//     m_buildCommand = QString::null;

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
        m_widget->slotExecuteProject();
        m_executeAfterBuild = false;
    }
}

bool TrollProjectPart::isDirty()
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

KDevProject::Options TrollProjectPart::options( ) const
{
    return UsesQMakeBuildSystem;
}

bool TrollProjectPart::isValidQtDir( const QString& path ) const
{
    QFileInfo inc( path + QString( QChar( QDir::separator() ) )+
                   "include"+QString( QChar( QDir::separator() ) )+
                   "qt.h" );
    return ( isQt4Project() || ( !isQt4Project() && inc.exists() ) );
}

void TrollProjectPart::buildBinDirs( QStringList & dirs ) const
{
    if( !isQt4Project() )
    {
        QString m_defaultQtDir = DomUtil::readEntry(*projectDom(), "/kdevcppsupport/qt/root", "");
        if( !m_defaultQtDir.isEmpty() )
            dirs << (m_defaultQtDir + QString( QChar( QDir::separator() ) ) + "bin" );
        dirs << ( ::getenv("QTDIR") + QString( QChar( QDir::separator() ) ) + "bin" );
    }
    QStringList paths = QStringList::split(":",::getenv("PATH"));
    dirs += paths;
    QString binpath = QDir::rootDirPath() + "bin";
    if( dirs.findIndex( binpath ) != -1 )
        dirs << binpath;

    binpath = QDir::rootDirPath() + "usr" + QString( QChar( QDir::separator() ) ) + "bin";
    if( dirs.findIndex( binpath ) != -1 )
        dirs << binpath;
    binpath = QDir::rootDirPath() + "usr" + QString( QChar( QDir::separator() ) ) + "local" + QString( QChar( QDir::separator() ) ) + "bin";
    if( dirs.findIndex( binpath ) != -1 )
        dirs << binpath;
}


QString TrollProjectPart::findExecutable( const QString& execname ) const
{
    QStringList dirs;
    buildBinDirs( dirs );

    for( QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it )
    {
        QString designer = *it + QString( QChar( QDir::separator() ) ) + execname;
        if( !designer.isEmpty() && isExecutable( designer ) )
        {
            return designer;
        }
    }
    return "";
}

bool TrollProjectPart::isExecutable( const QString& path ) const
{
    QFileInfo fi(path);
    return( fi.exists() && fi.isExecutable() );
}

QString TrollProjectPart::findQtDir()
{
    QStringList qtdirs;
    if( !isQt4Project() )
        qtdirs.push_back( ::getenv("QTDIR") );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"lib"+QString( QChar( QDir::separator() ) )+"qt"+QString("%1").arg( DomUtil::readEntry( *projectDom(), "/kdevcppsupport/qt/version", "3") ) );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"lib"+QString( QChar( QDir::separator() ) )+"qt"+QString( QChar( QDir::separator() ) )+QString("%1").arg( DomUtil::readEntry( *projectDom(), "/kdevcppsupport/qt/version", "3") ) );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"share"+QString( QChar( QDir::separator() ) )+"qt"+QString("%1").arg( DomUtil::readEntry( *projectDom(), "/kdevcppsupport/qt/version", "3") ) );
    qtdirs.push_back( QDir::rootDirPath()+"usr" );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"lib"+QString( QChar( QDir::separator() ) )+"qt" );

    for( QStringList::Iterator it=qtdirs.begin(); it!=qtdirs.end(); ++it )
    {
        QString qtdir = *it;
        if( !qtdir.isEmpty() && isValidQtDir(qtdir) )
        {
            return qtdir;
        }
    }
    return "";
}


QStringList recursiveProFind( const QString &currDir, const QString &baseDir )
{
	QStringList fileList;

	if( !currDir.contains( QString( QChar ( QDir::separator() ) ) +".." )
		&& !currDir.contains( QString( QChar( QDir::separator() ) )+".") )
	{
		QDir dir(currDir);
		QStringList dirList = dir.entryList(QDir::Dirs );
		QStringList::Iterator idx = dirList.begin();
		for( ; idx != dirList.end(); ++idx )
		{
			fileList += recursiveProFind( currDir + QString( QChar( QDir::separator() ) ) + (*idx),baseDir );
		}
		QStringList newFiles = dir.entryList("*.pro *.PRO");
		idx = newFiles.begin();
		for( ; idx != newFiles.end(); ++idx )
		{
			QString file = currDir + QString( QChar( QDir::separator() ) ) + (*idx);
			fileList.append( file.remove( baseDir ) );
		}
	}


	return fileList;
}

/*!
    \fn TrollProjectPart::distFiles() const
 */
QStringList TrollProjectPart::distFiles() const
{
	QStringList sourceList = allFiles();
	// Scan current source directory for any .pro files.
	QString projectDir = projectDirectory();
	QStringList files = recursiveProFind( projectDir, projectDir + QString( QChar( QDir::separator() ) ) );
	return sourceList + files;
}

bool TrollProjectPart::isQt4Project() const
{
	return ( DomUtil::readIntEntry( *projectDom(), "kdevcppsupport/qt/version", 3 ) == 4 );
}

KDirWatch* TrollProjectPart::dirWatch()
{
    return m_dirWatch;
}

void TrollProjectPart::slotBuild()
{
    m_widget->slotBuildProject();
}

#include "trollprojectpart.moc"

//kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on


