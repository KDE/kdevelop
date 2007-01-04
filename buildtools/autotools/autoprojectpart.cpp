/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   Copyright (C) 2002 by Victor Roeder                                   *
 *   victor_roeder@gmx.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include "autoprojectpart.h"
#include "autolistviewitems.h"
#include "configureoptionswidget.h"
#include "addtranslationdlg.h"
#include "addicondlg.h"
#include "autoprojectwidget.h"

#include <qdom.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qwhatsthis.h>
#include <qregexp.h>
#include <qgroupbox.h>

#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kdeversion.h>
#include <kprocess.h>

#include <domutil.h>
#include <kdevcore.h>
#include <kdevmakefrontend.h>
#include <kdevappfrontend.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>
#include <makeoptionswidget.h>
#include <runoptionswidget.h>
#include <envvartools.h>

#include <configwidgetproxy.h>
#include <kdevplugininfo.h>

#define CONFIGURE_OPTIONS 1
#define RUN_OPTIONS 2
#define MAKE_OPTIONS 3

static const KDevPluginInfo data("kdevautoproject");

K_EXPORT_COMPONENT_FACTORY( libkdevautoproject, AutoProjectFactory( data ) )

AutoProjectPart::AutoProjectPart(QObject *parent, const char *name, const QStringList &args)
    : KDevBuildTool(&data, parent, name ? name : "AutoProjectPart")
    , m_lastCompilationFailed(false)
{
    setInstance(AutoProjectFactory::instance());

    setXMLFile("kdevautoproject.rc");

    m_executeAfterBuild = false;
    m_isKDE = (args[0] == "kde");
    m_needMakefileCvs = false;

    m_widget = new AutoProjectWidget(this, m_isKDE);
    m_widget->setIcon(SmallIcon( info()->icon() ));
    m_widget->setCaption(i18n("Automake Manager"));
    QWhatsThis::add(m_widget, i18n("<b>Automake manager</b><p>"
                                   "The project tree consists of two parts. The 'overview' "
                                   "in the upper half shows the subprojects, each one having a "
                                   "Makefile.am. The 'details' view in the lower half shows the "
                                   "targets and files for the subproject selected in the overview."));

    mainWindow()->embedSelectViewRight(m_widget, i18n("Automake Manager"), i18n("Automake manager"));
    KAction *action;

    action = new KAction( i18n("Add Translation..."), 0,
                          this, SLOT(slotAddTranslation()),
                          actionCollection(), "project_addtranslation" );
    action->setToolTip(i18n("Add translation"));
    action->setWhatsThis(i18n("<b>Add translation</b><p>Creates .po file for the selected language."));
    action->setGroup("autotools");


    if (!m_isKDE)
        action->setEnabled(false);

    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    action->setToolTip(i18n("Build project"));
    action->setWhatsThis(i18n("<b>Build project</b><p>Runs <b>make</b> from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("Build &Active Target"), "make_kdevelop", Key_F7,
                          this, SLOT(slotBuildActiveTarget()),
                          actionCollection(), "build_buildactivetarget" );
    action->setToolTip(i18n("Build active target"));
    action->setWhatsThis(i18n("<b>Build active target</b><p>Constructs a series of make commands to build an active target. "
                              "Also builds dependent targets.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("Compile &File"), "make_kdevelop",
                          this, SLOT(slotCompileFile()),
                          actionCollection(), "build_compilefile" );
    action->setToolTip(i18n("Compile file"));
    action->setWhatsThis(i18n("<b>Compile file</b><p>Runs <b>make filename.o</b> command from the directory where 'filename' is the name of currently opened file.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("Run Configure"), 0,
                          this, SLOT(slotConfigure()),
                          actionCollection(), "build_configure" );
    action->setToolTip(i18n("Run configure"));
    action->setWhatsThis(i18n("<b>Run configure</b><p>Executes <b>configure</b> with flags, arguments "
                              "and environment variables specified in the project settings dialog, "
                              "<b>Configure Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("Run automake && friends"), 0,
                          this, SLOT(slotMakefilecvs()),
                          actionCollection(), "build_makefilecvs" );
    action->setToolTip(i18n("Run automake && friends"));
    action->setWhatsThis(i18n("<b>Run automake && friends</b><p>Executes<br><b>make -f Makefile.cvs</b><br><b>./configure</b><br>commands from the project directory."));
    action->setGroup("autotools");

    action = new KAction( i18n("Install"), 0,
                          this, SLOT(slotInstall()),
                          actionCollection(), "build_install" );
    action->setToolTip(i18n("Install"));
    action->setWhatsThis(i18n("<b>Install</b><p>Runs <b>make install</b> command from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("Install (as root user)"), 0,
                          this, SLOT(slotInstallWithKdesu()),
                          actionCollection(), "build_install_kdesu" );
    action->setToolTip(i18n("Install as root user"));
    action->setWhatsThis(i18n("<b>Install</b><p>Runs <b>make install</b> command from the project directory with root privileges.<br>"
                              "It is executed via kdesu command.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("&Clean Project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "build_clean" );
    action->setToolTip(i18n("Clean project"));
    action->setWhatsThis(i18n("<b>Clean project</b><p>Runs <b>make clean</b> command from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("&Distclean"), 0,
                          this, SLOT(slotDistClean()),
                          actionCollection(), "build_distclean" );
    action->setToolTip(i18n("Distclean"));
    action->setWhatsThis(i18n("<b>Distclean</b><p>Runs <b>make distclean</b> command from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    action = new KAction( i18n("Make Messages && Merge"), 0,
                          this, SLOT(slotMakeMessages()),
                          actionCollection(), "build_messages" );
    action->setToolTip(i18n("Make messages && merge"));
    action->setWhatsThis(i18n("<b>Make messages && merge</b><p>Runs <b>make package-messages</b> command from the project directory.<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Make Options</b> tab."));
    action->setGroup("autotools");

    if (!m_isKDE)
        action->setEnabled(false);

    buildConfigAction = new KSelectAction( i18n("Build Configuration"), 0,
                                           actionCollection(), "project_configuration" );
    buildConfigAction->setToolTip(i18n("Build configuration menu"));
    buildConfigAction->setWhatsThis(i18n("<b>Build configuration menu</b><p>Allows to switch between project build configurations.<br>"
                                         "Build configuration is a set of build and top source directory settings, "
                                         "configure flags and arguments, compiler flags, etc.<br>"
                                         "Modify build configurations in project settings dialog, <b>Configure Options</b> tab."));
    buildConfigAction->setGroup("autotools");

    QDomDocument &dom = *projectDom();
    if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/run/disable_default")) {
        //ok we handle the execute in this kpart
        action = new KAction( i18n("Execute Program"), "exec", SHIFT+Key_F9,
                              this, SLOT(slotExecute()),
                              actionCollection(), "build_execute" );
        action->setToolTip(i18n("Execute program"));
        action->setWhatsThis(i18n("<b>Execute program</b><p>Executes the currently active target or the main program specified in project settings, <b>Run Options</b> tab."));
        action->setGroup("autotools");
    }

    connect( buildConfigAction, SIGNAL(activated(const QString&)),
             this, SLOT(slotBuildConfigChanged(const QString&)) );
    connect( buildConfigAction->popupMenu(), SIGNAL(aboutToShow()),
             this, SLOT(slotBuildConfigAboutToShow()) );

//    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)) );

    _configProxy = new ConfigWidgetProxy( core() );
    _configProxy->createProjectConfigPage( i18n("Configure Options"), CONFIGURE_OPTIONS, info()->icon() );
    _configProxy->createProjectConfigPage( i18n("Run Options"), RUN_OPTIONS, info()->icon() );
    _configProxy->createProjectConfigPage( i18n("Make Options"), MAKE_OPTIONS, info()->icon() );
    connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
             this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );


    connect( makeFrontend(), SIGNAL(commandFinished(const QString&)),
             this, SLOT(slotCommandFinished(const QString&)) );
    connect( makeFrontend(), SIGNAL(commandFailed(const QString&)),
             this, SLOT(slotCommandFailed(const QString&)) );

    setWantautotools();


}


AutoProjectPart::~AutoProjectPart()
{
    if (m_widget)
    {
        mainWindow()->removeView(m_widget);
    }
    delete m_widget;
    delete _configProxy;
}


void AutoProjectPart::insertConfigWidget( const KDialogBase* dlg, QWidget * page, unsigned int pagenumber )
{
    switch ( pagenumber )
    {
    case CONFIGURE_OPTIONS:
        {
            ConfigureOptionsWidget *w2 = new ConfigureOptionsWidget(this, page );
            connect( dlg, SIGNAL(okClicked()), w2, SLOT(accept()) );
        }
        break;

    case RUN_OPTIONS:
        {
            QDomDocument &dom = *projectDom();
            if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/run/disable_default"))
            {
                //ok we handle the execute in this kpart
                RunOptionsWidget *w3 = new RunOptionsWidget(*projectDom(), "/kdevautoproject", buildDirectory(), page );
                connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );
            }
        }
        break;

    case MAKE_OPTIONS:
        {
            MakeOptionsWidget *w4 = new MakeOptionsWidget(*projectDom(), "/kdevautoproject", page );
            connect( dlg, SIGNAL(okClicked()), w4, SLOT(accept()) );
        }
        break;
    }
}

void AutoProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectName = projectName;
    m_projectPath =dirName;

	m_widget->openProject(dirName);

    QDomDocument &dom = *projectDom();
    QString activeTarget = DomUtil::readEntry(dom, "/kdevautoproject/general/activetarget");
    kdDebug(9020) << k_funcinfo << "activeTarget " << activeTarget << endl;
    if (!activeTarget.isEmpty())
        m_widget->setActiveTarget(activeTarget);

    // Set the default directory radio to "executable"
    if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/run/disable_default") && DomUtil::readEntry(dom, "/kdevautoproject/run/directoryradio") == "" ) {
        DomUtil::writeEntry(dom, "/kdevautoproject/run/directoryradio", "executable");
    }

    KDevProject::openProject( dirName, projectName );
}


void AutoProjectPart::closeProject()
{
    m_widget->closeProject();
}


QString AutoProjectPart::projectDirectory() const
{
    return m_projectPath;
}


QString AutoProjectPart::projectName() const
{
    return m_projectName;
}


/** Retuns a PairList with the run environment variables */
DomUtil::PairList AutoProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry(*projectDom(), "/kdevautoproject/run/envvars", "envvar", "name", "value");
}


/** Retuns the currently selected run directory
  * If no main Program was selected in the Run Options dialog
  * use the currently active target instead to calculate it.
  * The returned string can be:
  *   if /kdevautoproject/run/directoryradio == executable
  *        The directory where the executable is
  *   if /kdevautoproject/run/directoryradio == build
  *        The directory where the executable is relative to build directory
  *   if /kdevautoproject/run/directoryradio == custom
  *        The custom directory absolute path
  */
QString AutoProjectPart::runDirectory() const
{

    QDomDocument &dom = *projectDom();

    if( DomUtil::readBoolEntry(dom, "/kdevautoproject/run/useglobalprogram", true) )
    {
        return defaultRunDirectory("kdevautoproject");
    }else
    {
        return buildDirectory() + "/" + activeDirectory();
    }
}


/** Retuns the currently selected main program
  * If no main Program was selected in the Run Options dialog
  * use the currently active target instead.
  * The returned string can be:
  *   if /kdevautoproject/run/directoryradio == executable
  *        The executable name
  *   if /kdevautoproject/run/directoryradio == build
  *        The path to executable relative to build directory
  *   if /kdevautoproject/run/directoryradio == custom or relative == false
  *        The absolute path to executable
  */
/*
QString AutoProjectPart::mainProgram(bool relative) const
{
    QDomDocument &dom = *projectDom();

    if( DomUtil::readBoolEntry(dom, "/kdevautoproject/run/useglobalprogram", true) )
    {
        QString directoryRadioString = DomUtil::readEntry(dom, "/kdevautoproject/run/directoryradio");
        QString DomMainProgram = DomUtil::readEntry(dom, "/kdevautoproject/run/mainprogram");

        // A Main Program was specified, return it
        if ( directoryRadioString == "custom" )
            return DomMainProgram;

        if ( relative == false )
            return buildDirectory() + "/" + DomMainProgram;

        if ( directoryRadioString != "executable" )
            return DomMainProgram;

        int pos = DomMainProgram.findRev('/');
        if (pos != -1)
            return DomMainProgram.mid(pos+1);
        return DomMainProgram;


    } else {
        // If no Main Program was specified, return the active target

        QString directoryRadioString = DomUtil::readEntry(dom, "/kdevautoproject/run/directoryradio");
        // Get a pointer to the active target
        TargetItem* titem = m_widget->activeTarget();

        if ( !titem ) {
            kdDebug ( 9020 ) << k_funcinfo << "Error! : There's no active target! -> Unable to determine the main program in AutoProjectPart::mainProgram()" << endl;
            return QString::null;
        }

        if ( titem->primary != "PROGRAMS" ) {
            kdDebug ( 9020 ) << k_funcinfo << "Error! : Active target isn't binary (" << titem->primary << ") ! -> Unable to determine the main program in AutoProjectPart::mainProgram()" << endl;
            return QString::null;
        }

        if (relative == false || directoryRadioString == "custom")
            return buildDirectory() + "/" + activeDirectory() + "/" + titem->name;

        if ( directoryRadioString == "executable" )
            return titem->name;

        return activeDirectory() + "/" + titem->name;

    }
}
*/

QString AutoProjectPart::mainProgram() const
{
    QDomDocument * dom = projectDom();

    if ( !dom ) return QString();

    if( DomUtil::readBoolEntry(*dom, "/kdevautoproject/run/useglobalprogram", false) )
    {
        QString DomMainProgram = DomUtil::readEntry(*dom, "/kdevautoproject/run/mainprogram");

        if ( DomMainProgram.isEmpty() ) return QString();

        if ( DomMainProgram.startsWith("/") )   // assume absolute path
        {
            return DomMainProgram;
        }
        else // assume project relative path
        {
            return projectDirectory() + "/" + DomMainProgram;
        }

    } 
    else // If no Main Program was specified, return the active target
    {
        TargetItem* titem = m_widget->activeTarget();

        if ( !titem ) {
            kdDebug ( 9020 ) << k_funcinfo << "Error! : There's no active target! -> Unable to determine the main program in AutoProjectPart::mainProgram()" << endl;
            return QString::null;
        }

        if ( titem->primary != "PROGRAMS" ) {
            kdDebug ( 9020 ) << k_funcinfo << "Error! : Active target isn't binary (" << titem->primary << ") ! -> Unable to determine the main program in AutoProjectPart::mainProgram()" << endl;
            return QString::null;
        }

        return activeDirectory() + "/" + titem->name;
    }
}


/** Retuns a QString with the run command line arguments */
QString AutoProjectPart::runArguments() const
{
    QDomDocument &dom = *projectDom();

    if( DomUtil::readBoolEntry(dom, "/kdevautoproject/run/useglobalprogram", true) )
    {
        QString DomMainProgram = DomUtil::readEntry(dom, "/kdevautoproject/run/mainprogram");
        QString DomProgramArguments = DomUtil::readEntry(*projectDom(), "/kdevautoproject/run/programargs");

        if ( DomMainProgram.isEmpty() && DomProgramArguments.isEmpty() )
            return QString::null;
        else
            return DomProgramArguments;
    }else
    {
        return DomUtil::readEntry(*projectDom(), "/kdevautoproject/run/runarguments/" + m_widget->activeTarget()->name);
    }
}


QString AutoProjectPart::activeDirectory() const
{
    return m_widget->activeDirectory();
}


QStringList AutoProjectPart::allFiles() const
{
    return m_widget->allFiles();
}


void AutoProjectPart::setWantautotools()
{
    QDomDocument &dom = *projectDom();
    QDomElement el  = DomUtil::elementByPath(dom, "/kdevautoproject/make");
    if ( el.namedItem("envvars").isNull() ) {
        DomUtil::PairList list;
        list << DomUtil::Pair("WANT_AUTOCONF_2_5", "1");
        list << DomUtil::Pair("WANT_AUTOMAKE_1_6", "1");
        DomUtil::writePairListEntry(dom, "/kdevautoproject/make/envvars", "envvar", "name", "value", list);
    }
}


QString AutoProjectPart::makeEnvironment() const
{
    // Get the make environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*projectDom(), "/kdevautoproject/make/envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it)
    {
        environstr += (*it).first;
        environstr += "=";
        environstr += EnvVarTools::quote((*it).second);
        environstr += " ";
    }
    return environstr;
}


void AutoProjectPart::addFile(const QString &fileName)
{
    QStringList fileList;
    fileList.append ( fileName );

    this->addFiles ( fileList );
}

void AutoProjectPart::addFiles ( const QStringList& fileList )
{
    QString directory, name;
    QStringList::ConstIterator it;
    bool messageBoxShown = false;

    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
        int pos = ( *it ).findRev('/');
        if (pos != -1)
        {
            directory = ( *it ).left(pos);
            name = ( *it ).mid(pos+1);
        }
        else
        {
            directory = "";
            name = ( *it );
        }

        if (directory != m_widget->activeDirectory() ||
                directory.isEmpty())
        {
            if ( !messageBoxShown )
            {
                KMessageBox::information(m_widget, i18n("The directory you selected is not the active directory.\n"
                                                        "You should 'activate' the target you're currently working on in Automake Manager.\n"
                                                        "Just right-click a target and choose 'Make Target Active'."),
                                         i18n ( "No Active Target Found" ), "No automake manager active target warning" );
                messageBoxShown = true;
            }
        }
    }

    m_widget->addFiles(fileList);
}

void AutoProjectPart::removeFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );

	this->removeFiles ( fileList );
}

void AutoProjectPart::removeFiles ( const QStringList& fileList )
{
	/// \FIXME m_widget->removeFiles does nothing!
	m_widget->removeFiles ( fileList );

	emit removedFilesFromProject ( fileList );
}

QStringList AutoProjectPart::allBuildConfigs() const
{
    QDomDocument &dom = *projectDom();

    QStringList allConfigs;
    allConfigs.append("default");

    QDomNode node = dom.documentElement().namedItem("kdevautoproject").namedItem("configurations");
    QDomElement childEl = node.firstChild().toElement();
    while (!childEl.isNull())
    {
        QString config = childEl.tagName();
        kdDebug(9020) << k_funcinfo << "Found config " << config << endl;
        if (config != "default")
            allConfigs.append(config);
        childEl = childEl.nextSibling().toElement();
    }

    return allConfigs;
}


QString AutoProjectPart::currentBuildConfig() const
{
    QDomDocument &dom = *projectDom();

    QString config = DomUtil::readEntry(dom, "/kdevautoproject/general/useconfiguration");
    if (config.isEmpty() || !allBuildConfigs().contains(config))
        config = "default";

    return config;
}


QString AutoProjectPart::buildDirectory() const
{
    QString prefix = "/kdevautoproject/configurations/" + currentBuildConfig() + "/";

    QString builddir = DomUtil::readEntry(*projectDom(), prefix + "builddir");
    if (builddir.isEmpty())
        return topsourceDirectory();
    else if (builddir.startsWith("/"))
        return builddir;
    else
        return projectDirectory() + "/" + builddir;
}

QString AutoProjectPart::topsourceDirectory() const
{
    QString prefix = "/kdevautoproject/configurations/" + currentBuildConfig() + "/";

    QString topsourcedir = DomUtil::readEntry(*projectDom(), prefix + "topsourcedir");
    if (topsourcedir.isEmpty())
        return projectDirectory();
    else if (topsourcedir.startsWith("/"))
        return topsourcedir;
    else
        return projectDirectory() + "/" + topsourcedir;
}

QString AutoProjectPart::constructMakeCommandLine(const QString &dir, const QString &target) const
{

    QString preCommand;
    QFileInfo fi1();
    kdDebug(9020) << k_funcinfo << "Looking for Makefile in " << dir << endl;
    if ( !QFile::exists(dir + "/GNUmakefile") && !QFile::exists(dir + "/makefile")
            && ! QFile::exists(dir + "/Makefile") )
    {
        if (!QFile::exists(buildDirectory() + "/configure"))
        {
            int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory\n"
                                               "and no configure script for this project.\n"
                                               "Run automake & friends and configure first?"), QString::null, i18n("Run Them"), i18n("Do Not Run"));
            if (r == KMessageBox::No)
                return QString::null;
            preCommand = makefileCvsCommand();
            if (preCommand.isNull())
                return QString::null;
            preCommand += " && ";
            preCommand += configureCommand() + " && ";
        }
        else
        {
            int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory. Run 'configure' first?"), QString::null, i18n("Run configure"), i18n("Do Not Run"));
            if (r == KMessageBox::No)
                return QString::null;
            preCommand = configureCommand() + " && ";
        }
    }
    QDomDocument &dom = *projectDom();

    QString cmdline = DomUtil::readEntry(dom, "/kdevautoproject/make/makebin");
    int prio = DomUtil::readIntEntry(dom, "/kdevautoproject/make/prio");
    QString nice;
    kdDebug(9020) << k_funcinfo << "nice = " << prio<< endl;
    if (prio != 0)
    {
        nice = QString("nice -n%1 ").arg(prio);
    }

    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/make/abortonerror"))
        cmdline += " -k";
    bool runmultiple = DomUtil::readBoolEntry(dom, "/kdevautoproject/make/runmultiplejobs");
    int jobs = DomUtil::readIntEntry(dom, "/kdevautoproject/make/numberofjobs");
    if (runmultiple && jobs != 0)
    {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(dom, "/kdevautoproject/make/dontact"))
        cmdline += " -n";

    cmdline += " ";
    cmdline += target;
    cmdline.prepend(nice);
    cmdline.prepend(makeEnvironment());

    QString dircmd = "cd ";
    dircmd += KProcess::quote(dir);
    dircmd += " && ";

    return preCommand + dircmd + cmdline;
}


void AutoProjectPart::startMakeCommand(const QString &dir, const QString &target, bool withKdesu)
{
    if (partController()->saveAllFiles()==false)
       return; //user cancelled
    kdDebug(9020) << "startMakeCommand:" << dir << ": "<< target << endl;
    m_buildCommand = constructMakeCommandLine(dir, target);

    if (withKdesu)
        m_buildCommand = "kdesu -t -c '" + m_buildCommand + "'";

    if (!m_buildCommand.isNull())
         makeFrontend()->queueCommand(dir, m_buildCommand);
}


/** Adds the make command for the libraries that the target depends on
  * to the make frontend queue (this is a recursive function) */
void AutoProjectPart::queueInternalLibDependenciesBuild(TargetItem* titem)
{

    QString addstr = (titem->primary == "PROGRAMS")? titem->ldadd : titem->libadd;
    QStringList l2 = QStringList::split(QRegExp("[ \t]"), addstr); // list of dependencies
    QString tdir;          // temp target directory
    QString tname;         // temp target name
    QString tcmd;          // temp command line
    QStringList::Iterator l2it;
    for (l2it = l2.begin(); l2it != l2.end(); ++l2it)
    {
        QString dependency = *l2it;
        if (dependency.startsWith("$(top_builddir)/"))
        {
            // These are the internal libraries
            dependency.remove("$(top_builddir)/");

            tdir = buildDirectory();
            if (!tdir.endsWith("/") && !tdir.isEmpty())
                tdir += "/";
            int pos = dependency.findRev('/');
            if (pos == -1)
            {
                tname = dependency;
            }
            else
            {
                tdir += dependency.left(pos+1);
                tname = dependency.mid(pos+1);
            }
            kdDebug(9020) << "Scheduling : <" << tdir << ">  target <" << tname << ">" << endl;

            // Recursively queue the dependencies for building
            SubprojectItem *spi = m_widget->subprojectItemForPath( dependency.left(pos) );
            if (spi)
            {
                QPtrList< TargetItem > tl = spi->targets;
                // Cycle through the list of targets to find the one we're looking for
                TargetItem *ti = tl.first();
                do
                {
                    if (ti->name == tname)
                    {
                        // found it: queue it and stop looking
                        queueInternalLibDependenciesBuild(ti);
                        break;
                    }
                    ti = tl.next();
                }
                while (ti);
            }

            kdDebug(9020) << "queueInternalLibDependenciesBuild:" << tdir << ": "<< tname << endl;
            tcmd = constructMakeCommandLine(tdir, tname);
            if (!tcmd.isNull())
            {
                makeFrontend()->queueCommand( tdir, tcmd);
            }
        }
    }
}


void AutoProjectPart::slotBuild()
{
    //m_lastCompilationFailed = false;

    if( m_needMakefileCvs )
    {
        slotMakefilecvs();
        slotConfigure();
        m_needMakefileCvs = false;
    }

    startMakeCommand(buildDirectory(), QString::fromLatin1(""));
}


void AutoProjectPart::buildTarget(QString relpath, TargetItem* titem)
{

    if ( !titem )
        return;

    //m_lastCompilationFailed = false;

    // Calculate the complete name of the target and store it in name
    QString name = titem->name;
    if ( titem->primary == "KDEDOCS" )
        name = "index.cache.bz2";

    // Calculate the full path of the target and store it in path
    QString path = buildDirectory();
    if (!path.endsWith("/") && !path.isEmpty())
        path += "/";
    if (relpath.at(0) == '/')
        path += relpath.mid(1);
    else
        path += relpath;

    // Save all files once
    partController()->saveAllFiles();

    // Add the make command for the libraries that the target depends on to the make frontend queue
    // if this recursive behavour is un-wanted comment the next line
    queueInternalLibDependenciesBuild(titem);

    // Calculate the "make" command line for the target
    //QString relpath = dir.path().mid( projectDirectory().length() );
    m_runProg=buildDirectory() + "/" + relpath+"/"+name;
    kdDebug(9020) << "buildTarget:" << buildDirectory()<< endl;
    kdDebug(9020) << "buildTarget:" << relpath << "  " << path << ": "<< name << " : " << m_runProg << endl;
    QString tcmd = constructMakeCommandLine( path, name );

    // Call make
    if (!tcmd.isNull())
    {
        m_buildCommand = tcmd;
        makeFrontend()->queueCommand( path, tcmd);
    }
}


void AutoProjectPart::slotBuildActiveTarget()
{
  // Get a pointer to the active target
  TargetItem* titem = m_widget->activeTarget();

  if ( !titem )
    return;

  // build it
  buildTarget(activeDirectory(), titem);
}


void AutoProjectPart::slotCompileFile()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    if (!part || !part->url().isLocalFile())
        return;

    QString fileName = part->url().path();
    QFileInfo fi(fileName);
    QString sourceDir = fi.dirPath();
    QString baseName = fi.baseName(true);
    kdDebug(9020) << "Compiling " << fileName
                  << " in dir " << sourceDir
                  << " with baseName " << baseName << endl;

    QString projectDir = projectDirectory();
    if (!sourceDir.startsWith(projectDir)) {
        KMessageBox::sorry(m_widget, i18n("Can only compile files in directories which belong to the project."));
        return;
    }

    QString buildDir = buildDirectory() + sourceDir.mid(projectDir.length());
    QString target = baseName + ".lo";
    kdDebug(9020) << "builddir " << buildDir << ", target " << target << endl;

    startMakeCommand(buildDir, target);
}

QString AutoProjectPart::configureCommand() const
{
    QDomDocument &dom = *projectDom();
    QString prefix = "/kdevautoproject/configurations/" + currentBuildConfig() + "/";

    QString cmdline = "\"" + topsourceDirectory();
    cmdline += "/configure\"";
    QString cc = DomUtil::readEntry(dom, prefix + "ccompilerbinary");
    if (!cc.isEmpty())
        cmdline.prepend(QString("CC=%1 ").arg(cc));
    QString cflags = DomUtil::readEntry(dom, prefix + "cflags");
    if (!cflags.isEmpty())
        cmdline.prepend(QString("CFLAGS=\"%1\" ").arg(cflags));
    QString cxx = DomUtil::readEntry(dom, prefix + "cxxcompilerbinary");
    if (!cxx.isEmpty())
        cmdline.prepend(QString("CXX=%1 ").arg(cxx));
    QString cxxflags = DomUtil::readEntry(dom, prefix + "cxxflags");
    if (!cxxflags.isEmpty())
        cmdline.prepend(QString("CXXFLAGS=\"%1\" ").arg(cxxflags));
    QString f77 = DomUtil::readEntry(dom, prefix + "f77compilerbinary");
    if (!f77.isEmpty())
        cmdline.prepend(QString("F77=%1 ").arg(f77));
    QString fflags = DomUtil::readEntry(dom, prefix + "f77flags");
    if (!fflags.isEmpty())
        cmdline.prepend(QString("FFLAGS=\"%1\" ").arg(fflags));
    QString cppflags = DomUtil::readEntry(dom, prefix + "cppflags");
    if (!cppflags.isEmpty())
        cmdline.prepend(QString("CPPFLAGS=\"%1\" ").arg(cppflags));
    QString ldflags = DomUtil::readEntry(dom, prefix + "ldflags");
    if (!ldflags.isEmpty())
        cmdline.prepend(QString("LDFLAGS=\"%1\" ").arg(ldflags));

    QString configargs = DomUtil::readEntry(dom, prefix + "configargs");
    if (!configargs.isEmpty()) {
        cmdline += " ";
        cmdline += configargs;
    }

   DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*projectDom(), prefix + "envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
        environstr += EnvVarTools::quote((*it).second);
        environstr += " ";
    }
    cmdline.prepend(environstr);

    QString builddir = buildDirectory();
    QString dircmd;

    // if the build directory doesn't exist, add it's creation to the configureCommand
    if ( !QFile::exists(builddir)) {
        dircmd = "mkdir ";
        dircmd += KProcess::quote(builddir);
        dircmd += " && ";
    }

    // add "cd into the build directory" to the configureCommand
    dircmd += "cd ";
    dircmd += KProcess::quote(builddir);
    dircmd += " && ";

    return dircmd + cmdline;
}

void AutoProjectPart::slotConfigure()
{
    QString cmdline = configureCommand();
    if (cmdline.isNull())
        return;

    makeFrontend()->queueCommand(buildDirectory(), cmdline);
}

QString AutoProjectPart::makefileCvsCommand() const
{
    kdDebug(9020) << "makefileCvsCommand: runDirectory       :" << runDirectory() << ":" <<endl;
    kdDebug(9020) << "makefileCvsCommand: topsourceDirectory :" << topsourceDirectory() << ":" <<endl;
    kdDebug(9020) << "makefileCvsCommand: makeEnvironment    :" << makeEnvironment() << ":" <<endl;
    kdDebug(9020) << "makefileCvsCommand: currentBuildConfig :" << currentBuildConfig() << ":" <<endl;


    QString cmdline = DomUtil::readEntry(*projectDom(), "/kdevautoproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;

    int prio = DomUtil::readIntEntry(*projectDom(), "/kdevautoproject/make/prio");
    QString nice;
    kdDebug(9020) << "makefileCvsCommand() nice = " << prio<< endl;
    if (prio != 0) {
        nice = QString("nice -n%1 ").arg(prio);
    }

    if (QFile::exists(topsourceDirectory() + "/Makefile.cvs"))
        cmdline += " -f Makefile.cvs";
    else if (QFile::exists(topsourceDirectory() + "/Makefile.dist"))
        cmdline += " -f Makefile.dist";
    else if (QFile::exists(topsourceDirectory() + "/autogen.sh"))
        cmdline = "./autogen.sh";
    else {
        KMessageBox::sorry(m_widget, i18n("There is neither a Makefile.cvs file nor an "
                                          "autogen.sh script in the project directory."));
        return QString::null;
    }

    cmdline.prepend(nice);
    cmdline.prepend(makeEnvironment());

    QString dircmd = "cd ";
    dircmd += KProcess::quote(topsourceDirectory());
    dircmd += " && ";

    return dircmd + cmdline;
}

void AutoProjectPart::slotMakefilecvs()
{
    QString cmdline = makefileCvsCommand();
    if ( cmdline.isNull() )
        return;

    makeFrontend()->queueCommand(projectDirectory(), cmdline);
}


void AutoProjectPart::slotInstall()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1("install"));
}


void AutoProjectPart::slotInstallWithKdesu()
{
    // First issue "make" to build the entire project with the current user
    // This way we make sure all files are up to date before we do the "make install"
    slotBuild();

    // After that issue "make install" with the root user
    startMakeCommand(buildDirectory(), QString::fromLatin1("install"), true);
}


void AutoProjectPart::slotClean()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1("clean"));
}


void AutoProjectPart::slotDistClean()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1("distclean"));
}


void AutoProjectPart::slotMakeMessages()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1("package-messages"));
}


/** Checks if the currently selected main program or,
  * if no main Program was selected in the Run Options dialog,
  * the currently active target is up-to-date and builds it if necessary.
  * In the end checks if the program is already running and if not calls the
  * slotExecute2() function to execute it or asks the user what to do.
  */
void AutoProjectPart::slotExecute()
{
    partController()->saveAllFiles();
    QDomDocument &dom = *projectDom();

    m_runProg=m_runProg.isEmpty()?mainProgram():m_runProg;

    bool _auto = false;
    if( DomUtil::readBoolEntry(dom, "/kdevautoproject/run/autocompile", true) && isDirty() ){
        m_executeAfterBuild = true;
        if ( DomUtil::readEntry(dom, "/kdevautoproject/run/mainprogram").isEmpty() ){
        // If no Main Program was specified, build the active target
            kdDebug(9020) << "slotExecute: before slotBuildActiveTarget" << endl;
            slotBuildActiveTarget();
        }
        else{
        // A Main Program was specified, build all targets because we don't know which is it
            kdDebug(9020) << "slotExecute: before slotBuild" << endl;
            slotBuild();
        }
        _auto = true;
    }

    if( DomUtil::readBoolEntry(dom, "/kdevautoproject/run/autoinstall", false) && isDirty() ){
        m_executeAfterBuild = true;
        // Use kdesu??
        if( DomUtil::readBoolEntry(dom, "/kdevautoproject/run/autokdesu", false) ){
            //slotInstallWithKdesu assumes that it hasn't just been build...
            kdDebug(9020) << "slotExecute: before startMakeCommand" << endl;
            _auto ? slotInstallWithKdesu() : startMakeCommand(buildDirectory(), QString::fromLatin1("install"), true);
        }
        else{
            kdDebug(9020) << "slotExecute: before slotInstall" << endl;
            slotInstall();
        }
        _auto = true;
    }

    if ( _auto ){
        m_runProg.truncate(0);
        return;
    }

    if (appFrontend()->isRunning()) {
        if (KMessageBox::questionYesNo(m_widget, i18n("Your application is currently running. Do you want to restart it?"), i18n("Application Already Running"), i18n("&Restart Application"), i18n("Do &Nothing")) == KMessageBox::No)
            return;
        connect(appFrontend(), SIGNAL(processExited()), SLOT(slotExecute2()));
        appFrontend()->stopApplication();
        return;
    }
    kdDebug(9020) << "slotExecute: before slotExecute2" << endl;
    slotExecute2();
}

void AutoProjectPart::executeTarget(const QDir& dir, const TargetItem* titem)
{
    m_executeAfterBuild=true;
    partController()->saveAllFiles();

    bool is_dirty = false;
    QDateTime t = QFileInfo(dir , titem->name ).lastModified();
    QPtrListIterator<FileItem> it( titem->sources );
    for( ; it.current() ; ++it )
    {
        if( t < QFileInfo(dir , (*it)->name).lastModified())
            is_dirty = true;
    }

    if( DomUtil::readBoolEntry(*projectDom(), "/kdevautoproject/run/autocompile", true) && is_dirty )
    {
        connect( makeFrontend(), SIGNAL(commandFinished(const QString&)), this, SLOT(slotExecuteTargetAfterBuild(const QString&)) );
        connect( makeFrontend(), SIGNAL(commandFailed(const QString&)), this, SLOT(slotNotExecuteTargetAfterBuildFailed(const QString&)) );

        m_runProg=titem->name;
        m_executeTargetAfterBuild.first = dir;
        m_executeTargetAfterBuild.second = const_cast<TargetItem*>(titem);

        QString relpath = dir.path().mid( projectDirectory().length() );
        kdDebug(9020) << "executeTarget: before buildTarget " << relpath << endl;
        buildTarget(relpath, const_cast<TargetItem*>(titem));
        return;
    }


    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevautoproject/run/terminal");

    QString program = environString();
    if(!titem->name.startsWith("/"))
        program += "./";
    program += titem->name;

    QString args = DomUtil::readEntry(*projectDom(), "/kdevautoproject/run/runarguments/" + titem->name);

    program += " " + args;
    kdDebug(9020) << "executeTarget:cmd=" << dir.path() << " " << program << endl;
    appFrontend()->startAppCommand(dir.path(), program ,inTerminal);
    m_executeAfterBuild=false;

}

void AutoProjectPart::slotExecuteTargetAfterBuild(const QString& command)
{
    kdDebug(9020) << "slotExecuteTargetAfterBuild " << command << endl;
    if ( m_executeAfterBuild && constructMakeCommandLine(m_executeTargetAfterBuild.first.path(), m_executeTargetAfterBuild.second->name) == command )
    {
        disconnect( makeFrontend(), SIGNAL(commandFinished(const QString&)), this, SLOT(slotExecuteAfterTargetBuild()) );
        disconnect( makeFrontend(), SIGNAL(commandFailed(const QString&)), this, SLOT(slotExecuteAfterTargetBuildFailed()) );
        kdDebug(9020) << "slotExecuteTargetAfterBuild " << endl;
        executeTarget(m_executeTargetAfterBuild.first, m_executeTargetAfterBuild.second);
    }

}

void AutoProjectPart::slotNotExecuteTargetAfterBuildFailed(const QString& command)
{
    kdDebug(9020) << "executeTargetAfterBuildFailed" << endl;
    if ( constructMakeCommandLine(m_executeTargetAfterBuild.first.path(), m_executeTargetAfterBuild.second->name) == command )
    {
        m_executeAfterBuild=false;
        disconnect( makeFrontend(), SIGNAL(commandFinished(const QString&)), this, SLOT(slotExecuteTargetAfterBuild()) );
        disconnect( makeFrontend(), SIGNAL(commandFailed(const QString&)), this, SLOT(slotNotExecuteTargetAfterBuildFailed()) );
    }
}


/* Get the run environment variables pairs into the environstr string
 * in the form of: "ENV_VARIABLE=ENV_VALUE"
 * Note that we quote the variable value due to the possibility of
 * embedded spaces. */
QString AutoProjectPart::environString() const
{
    DomUtil::PairList envvars = runEnvironmentVars();
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

/** Executes the currently selected main program.
  * If no main Program was selected in the Run Options dialog
  * the currently active target is executed instead.
  */
void AutoProjectPart::slotExecute2()
{
    disconnect(appFrontend(), SIGNAL(processExited()), this, SLOT(slotExecute2()));

    if (m_runProg.isEmpty()){
        // Do not execute non executable targets
        return;
    }

    QString program = environString();
    // Adds the ./ that is necessary to execute the program in bash shells
    if (!m_runProg.startsWith("/")){
        program += "./";
    }
    program += m_runProg;
    program += " " + runArguments();

    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevautoproject/run/terminal");

    kdDebug(9020) << "slotExecute2: runDirectory: <" << runDirectory() << ">" <<endl;
    kdDebug(9020) << "slotExecute2: environstr  : <" << environString() << ">" <<endl;
    kdDebug(9020) << "slotExecute2: mainProgram : <" << mainProgram() << ">" <<endl;
    kdDebug(9020) << "slotExecute2: runArguments: <" << runArguments() << ">" <<endl;
    kdDebug(9020) << "slotExecute2: program     : <" << program << ">" <<endl;

    appFrontend()->startAppCommand(runDirectory(), program, inTerminal);
    m_executeAfterBuild=false;
    m_runProg.truncate(0);
}


void AutoProjectPart::slotAddTranslation()
{
    AddTranslationDialog dlg(this, m_widget);
    dlg.exec();
}


void AutoProjectPart::slotBuildConfigChanged(const QString &config)
{
    DomUtil::writeEntry(*projectDom(), "/kdevautoproject/general/useconfiguration", config);
    kdDebug(9020) << "Changed used configuration to " << config << endl;
}


void AutoProjectPart::slotBuildConfigAboutToShow()
{
    QStringList l = allBuildConfigs();
    buildConfigAction->setItems(l);
    buildConfigAction->setCurrentItem(l.findIndex(currentBuildConfig()));
}

void AutoProjectPart::restorePartialProjectSession ( const QDomElement* el )
{
    m_widget->restoreSession ( el );
}

void AutoProjectPart::savePartialProjectSession ( QDomElement* el )
{
    QDomDocument domDoc = el->ownerDocument();

    KMessageBox::information ( 0, "Hallo, Welt!" );

    kdDebug ( 9020 ) << k_funcinfo << "1" << endl;

    if ( domDoc.isNull() )
    {
        kdDebug ( 9020 ) << k_funcinfo << "2" << endl;
        return;
    }

    kdDebug ( 9020 ) << k_funcinfo << "3" << endl;

    m_widget->saveSession ( el );
}

void AutoProjectPart::slotCommandFinished( const QString& command )
{
    kdDebug(9020) << k_funcinfo << endl;

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

    // reset the "last compilation has failed" flag
    m_lastCompilationFailed = false;

    if( m_executeAfterBuild ){
        slotExecute();
    }
}

void AutoProjectPart::slotCommandFailed( const QString& /*command*/ )
{
    kdDebug(9020) << "slotCommandFinished " << k_funcinfo << endl;

    m_lastCompilationFailed = true;
    m_executeAfterBuild=false;
}

bool AutoProjectPart::isDirty()
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

void AutoProjectPart::needMakefileCvs( )
{
    m_needMakefileCvs = true;
}

bool AutoProjectPart::isKDE() const
{
    return m_isKDE;
}

KDevProject::Options AutoProjectPart::options() const
{
    return UsesAutotoolsBuildSystem;
}

QStringList recursiveATFind( const QString &currDir, const QString &baseDir )
{
    kdDebug(9020) << "Dir " << currDir << endl;
    QStringList fileList;

    if( !currDir.contains( "/..") && !currDir.contains("/.") )
    {
        QDir dir(currDir);
        QStringList dirList = dir.entryList(QDir::Dirs );
        QStringList::Iterator idx = dirList.begin();
        for( ; idx != dirList.end(); ++idx )
        {
            fileList += recursiveATFind( currDir + "/" + (*idx),baseDir );
        }
        QStringList newFiles = dir.entryList("*.am *.in");
        idx = newFiles.begin();
        for( ; idx != newFiles.end(); ++idx )
        {
            QString file = currDir + "/" + (*idx);
            fileList.append( file.remove( baseDir ) );
        }
    }


    return fileList;
}

/*!
    \fn AutoProjectPart::distFiles() const
 */
QStringList AutoProjectPart::distFiles() const
{
    QStringList sourceList = allFiles();
    // Scan current source directory for any .pro files.
    QString projectDir = projectDirectory();
    QDir dir(projectDir);
    QDir admin(projectDir +"/admin");
    QStringList files = dir.entryList( "Makefile.cvs Makefile.am configure* INSTALL README NEWS TODO ChangeLog COPYING AUTHORS stamp-h.in acinclude.m4 config.h.in Makefile.in install-sh config.sub config.guess mkinstalldirs missing ltmain.sh depcomp");
    QStringList adminFiles = admin.entryList(QDir::Files);
    QStringList::Iterator idx = adminFiles.begin();
    for( ; idx != adminFiles.end(); ++idx)
    {
        files.append( "admin/" + (*idx) );
    }
    QStringList srcDirs = dir.entryList(QDir::Dirs);
    idx = srcDirs.begin();
    for(; idx != srcDirs.end(); ++idx)
    {
        sourceList += recursiveATFind( projectDirectory() + "/" + (*idx), projectDirectory());
    }
    return sourceList + files;
}

void AutoProjectPart::startSimpleMakeCommand( const QString & dir, const QString & command, bool withKdesu )
{
    if (partController()->saveAllFiles()==false)
        return; //user cancelled

    //	m_buildCommand = constructMakeCommandLine(dir, target);

    QString cmdline = command;
    cmdline.prepend(makeEnvironment());

    QString dircmd = "cd ";
    dircmd += KProcess::quote(dir);
    dircmd += " && ";

    m_buildCommand = dircmd + cmdline;

    if (withKdesu)
        m_buildCommand = "kdesu -t -c '" + m_buildCommand + "'";

    if (!m_buildCommand.isNull())
        makeFrontend()->queueCommand(dir, m_buildCommand);
}

QString AutoProjectPart::getAutoConfFile(const QString& dir){

    QFile inFile(dir + "/configure.in");
    QFile acFile(dir + "/configure.ac");
    if ( inFile.exists()){
        return inFile.name();
    }else if (acFile.exists()){
        return acFile.name();
    }
    return acFile.name();;
}

#include "autoprojectpart.moc"

// kate: space-indent on; indent-width 4;
