/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
*   Copyright (C) 2002 by Victor Röder                                    *
*   victor_roeder@gmx.de                                                  *
*                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "autoprojectpart.h"

#include <qdom.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qwhatsthis.h>

#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/part.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "makeoptionswidget.h"
#include "runoptionswidget.h"
#include "configureoptionswidget.h"
#include "addtranslationdlg.h"
#include "addicondlg.h"
#include "autoprojectwidget.h"
#include "config.h"


K_EXPORT_COMPONENT_FACTORY( libkdevautoproject, AutoProjectFactory( "kdevautoproject" ) );

AutoProjectPart::AutoProjectPart(QObject *parent, const char *name, const QStringList &args)
    : KDevProject(parent, name ? name : "AutoProjectPart")
{
    setInstance(AutoProjectFactory::instance());

    setXMLFile("kdevautoproject.rc");
    
    bool kde = (args[0] == "kde");

    m_widget = new AutoProjectWidget(this, kde);
    m_widget->setIcon(SmallIcon("make"));
    m_widget->setCaption(i18n("Automake Manager"));
    QWhatsThis::add(m_widget, i18n("Automake Manager\n\n"
                                   "The project tree consists of two parts. The 'overview' "
                                   "in the upper half shows the subprojects, each one having a "
                                   "Makefile.am. The 'details' view in the lower half shows the "
                                   "targets for the subproject selected in the overview."));
    
    mainWindow()->embedSelectViewRight(m_widget, i18n("Automake Manager"), i18n("Automake Manager"));

    KAction *action;

    action = new KAction( i18n("Add Translation..."), 0,
                          this, SLOT(slotAddTranslation()),
                          actionCollection(), "project_addtranslation" );

/*	action = new KAction ( i18n("&Import existing files and directories..."), "", 0,
							this, SLOT ( slotImportExisting() ),
							actionCollection(), "project_importexisting" );
	action->setStatusText ( i18n ( "Import existing files and directories to the currently loaded project" ) );
*/
	if (!kde)
        action->setEnabled(false);

    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );

    action = new KAction( i18n("Compile &File"), "make_kdevelop",
                          this, SLOT(slotCompileFile()),
                          actionCollection(), "build_compilefile" );
    
    action = new KAction( i18n("Run Configure"), 0,
                          this, SLOT(slotConfigure()),
                          actionCollection(), "build_configure" );

    action = new KAction( i18n("Run automake && friends"), 0,
                          this, SLOT(slotMakefilecvs()),
                          actionCollection(), "build_makefilecvs" );

    action = new KAction( i18n("Install"), 0,
                          this, SLOT(slotInstall()),
                          actionCollection(), "build_install" );
    
    action = new KAction( i18n("&Clean Project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "build_clean" );
    
    action = new KAction( i18n("&Distclean"), 0,
                          this, SLOT(slotDistClean()),
                          actionCollection(), "build_distclean" );

    action = new KAction( i18n("Make Messages && Merge"), 0,
                          this, SLOT(slotMakeMessages()),
                          actionCollection(), "build_messages" );
    if (!kde)
        action->setEnabled(false);

    buildConfigAction = new KSelectAction( i18n("Build Configuration"), 0,
                                           actionCollection(), "project_configuration" );

    QDomDocument &dom = *projectDom();
    if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/run/disable_default")) {
        //ok we handle the execute in this kpart
        action = new KAction( i18n("Execute Program"), "exec", 0,
                              this, SLOT(slotExecute()),
                              actionCollection(), "build_execute" );
    }
    
    connect( buildConfigAction, SIGNAL(activated(const QString&)),
             this, SLOT(slotBuildConfigChanged(const QString&)) );
    connect( buildConfigAction->popupMenu(), SIGNAL(aboutToShow()),
             this, SLOT(slotBuildConfigAboutToShow()) );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    setWantautotools();
}


AutoProjectPart::~AutoProjectPart()
{
    if (m_widget)
        mainWindow()->removeView(m_widget);
    delete m_widget;
}

/*void AutoProjectPart::slotImportExisting()
{
	ImportExistingDlg( this, m_widget, "import_existing", true ).exec();
}*/

void AutoProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Configure Options"));
    ConfigureOptionsWidget *w2 = new ConfigureOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w2, SLOT(accept()) );
    QDomDocument &dom = *projectDom();
    if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/run/disable_default")) {
        //ok we handle the execute in this kpart
        vbox = dlg->addVBoxPage(i18n("Run Options"));
        RunOptionsWidget *w3 = new RunOptionsWidget(*projectDom(), "/kdevautoproject",projectDirectory(), vbox);
        connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );
    }
    vbox = dlg->addVBoxPage(i18n("Make Options"));
    MakeOptionsWidget *w4 = new MakeOptionsWidget(*projectDom(), "/kdevautoproject", vbox);
    connect( dlg, SIGNAL(okClicked()), w4, SLOT(accept()) );
}


void AutoProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_widget->openProject(dirName);
    m_projectName = projectName;
    m_projectPath =dirName;

    QDomDocument &dom = *projectDom();
    QString activeTarget = DomUtil::readEntry(dom, "/kdevautoproject/general/activetarget");
    kdDebug(9020) << "activeTarget " << activeTarget << endl;
    if (!activeTarget.isEmpty())
        m_widget->setActiveTarget(activeTarget);
}


void AutoProjectPart::closeProject()
{
    m_widget->closeProject();
}


QString AutoProjectPart::projectDirectory()
{
    return m_widget->projectDirectory();
}


QString AutoProjectPart::projectName()
{
    return m_projectName;
}


QString AutoProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevautoproject/run/mainprogram");
}


QString AutoProjectPart::activeDirectory()
{
    return m_widget->activeDirectory();
}


QStringList AutoProjectPart::allFiles()
{
    return m_widget->allFiles();
}


void AutoProjectPart::setWantautotools()
{
    QDomDocument &dom = *projectDom();
    QDomElement el  = DomUtil::elementByPath(dom, "/kdevautoproject");
    if ( el.namedItem("makeenvvars").isNull() ) {
        DomUtil::PairList list;
        list << DomUtil::Pair("WANT_AUTOCONF_2_5", "1");
        list << DomUtil::Pair("WANT_AUTOMAKE_1_6", "1");
        DomUtil::writePairListEntry(dom, "/kdevautoproject/makeenvvars", "envvar", "name", "value", list);
    }
}


QString AutoProjectPart::makeEnvironment()
{
    DomUtil::PairList envvars = 
        DomUtil::readPairListEntry(*projectDom(), "/kdevautoproject/makeenvvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
        environstr += (*it).second;
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
				KMessageBox::information(m_widget, i18n("It seems that you don't have an Active Target specified!\n"
														"Automake Manager supports this feature to 'activate' the target your currently working on.\n"
														"Just right-click a target and choose 'Make Target Active'."),
														i18n ( "No Active Target found" ) );
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
	// FIXME: m_widget->removeFiles does nothing!
	m_widget->removeFiles ( fileList );
	
	emit removedFilesFromProject ( fileList );
}

QStringList AutoProjectPart::allBuildConfigs()
{
    QDomDocument &dom = *projectDom();

    QStringList allConfigs;
    allConfigs.append("default");
    
    QDomNode node = dom.documentElement().namedItem("kdevautoproject").namedItem("configurations");
    QDomElement childEl = node.firstChild().toElement();
    while (!childEl.isNull()) {
        QString config = childEl.tagName();
        kdDebug(9020) << "Found config " << config << endl;
        if (config != "default")
            allConfigs.append(config);
        childEl = childEl.nextSibling().toElement();
    }

    return allConfigs;
}


QString AutoProjectPart::currentBuildConfig()
{
    QDomDocument &dom = *projectDom();

    QString config = DomUtil::readEntry(dom, "/kdevautoproject/general/useconfiguration");
    if (config.isEmpty() || !allBuildConfigs().contains(config))
        config = "default";

    return config;
}


QString AutoProjectPart::buildDirectory()
{
    QDomDocument &dom = *projectDom();
    QString prefix = "/kdevautoproject/configurations/" + currentBuildConfig() + "/";

    QString builddir = DomUtil::readEntry(dom, prefix + "builddir");
    if (builddir.isEmpty())
        return topsourceDirectory();
    else if (builddir.startsWith("/"))
        return builddir;
    else
        return projectDirectory() + "/" + builddir;
}

QString AutoProjectPart::topsourceDirectory()
{
    QDomDocument &dom = *projectDom();
    QString prefix = "/kdevautoproject/configurations/" + currentBuildConfig() + "/";

    QString topsourcedir = DomUtil::readEntry(dom, prefix + "topsourcedir");
    if (topsourcedir.isEmpty())
        return projectDirectory();
    else if (topsourcedir.startsWith("/"))
        return topsourcedir;
    else
        return projectDirectory() + "/" + topsourcedir;
}

void AutoProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QString preCommand;
    QFileInfo fi1();
    if ( !QFile::exists(dir + "/GNUmakefile") && !QFile::exists(dir + "/makefile")
         && ! QFile::exists(dir + "/Makefile") ) {
        if (!QFile::exists(buildDirectory() + "/configure")) {
            int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory\n"
                                                              "and no configure script for this project.\n"
                                                              "Run automake & friends and configure first?"));
            if (r == KMessageBox::No)
                return;
            preCommand = makefileCvsCommand();
            if (preCommand.isNull())
                return;
            preCommand += " && ";
            preCommand += configureCommand() + " && ";
        } else {
            int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory. Run configure first?"));
            if (r == KMessageBox::No)
                return;
            preCommand = configureCommand() + " && ";
        }
    }
    QDomDocument &dom = *projectDom();

    QString cmdline = DomUtil::readEntry(dom, "/kdevautoproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/make/abortonerror"))
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry(dom, "/kdevautoproject/make/numberofjobs");
    if (jobs != 0) {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(dom, "/kdevautoproject/make/dontact"))
        cmdline += " -n";

    cmdline += " ";
    cmdline += target;
    cmdline.prepend(makeEnvironment());

    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    makeFrontend()->queueCommand(dir, preCommand + dircmd + cmdline);
}


void AutoProjectPart::slotBuild()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1(""));
}


void AutoProjectPart::slotCompileFile()
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

QString AutoProjectPart::configureCommand()
{
    QDomDocument &dom = *projectDom();
    QString prefix = "/kdevautoproject/configurations/" + currentBuildConfig() + "/";

    QString cmdline = topsourceDirectory();
    cmdline += "/configure";
    QString cc = DomUtil::readEntry(dom, prefix + "ccompilerbinary");
    if (!cc.isEmpty())
        cmdline.prepend(QString("CC=%1 ").arg(cc));
    QString cflags = DomUtil::readEntry(dom, prefix + "cflags");
    if (!cflags.isEmpty())
        cmdline.prepend(QString("CFLAGS=%1 ").arg(cflags));
    QString cxx = DomUtil::readEntry(dom, prefix + "cxxcompilerbinary");
    if (!cxx.isEmpty())
        cmdline.prepend(QString("CXX=%1 ").arg(cxx));
    QString cxxflags = DomUtil::readEntry(dom, prefix + "cxxflags");
    if (!cxxflags.isEmpty())
        cmdline.prepend(QString("CXXFLAGS=%1 ").arg(cxxflags));
    QString f77 = DomUtil::readEntry(dom, prefix + "f77compilerbinary");
    if (!f77.isEmpty())
        cmdline.prepend(QString("F77=%1 ").arg(f77));
    QString fflags = DomUtil::readEntry(dom, prefix + "f77flags");
    if (!fflags.isEmpty())
        cmdline.prepend(QString("FFLAGS=%1 ").arg(fflags));

    QString configargs = DomUtil::readEntry(dom, prefix + "configargs");
    if (!configargs.isEmpty()) {
        cmdline += " ";
        cmdline += configargs;
    }

    QString builddir = buildDirectory();

    QString dircmd = "cd ";
    dircmd += builddir;
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

QString AutoProjectPart::makefileCvsCommand()
{
    QString cmdline = DomUtil::readEntry(*projectDom(), "/kdevautoproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;

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

    cmdline.prepend(makeEnvironment());

    QString dircmd = "cd ";
    dircmd += topsourceDirectory();
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


void AutoProjectPart::slotExecute()
{
    QString program = buildDirectory() + "/" + mainProgram();
    program += " " + DomUtil::readEntry(*projectDom(), "/kdevautoproject/run/programargs");
    
    DomUtil::PairList envvars = 
        DomUtil::readPairListEntry(*projectDom(), "/kdevautoproject/envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
        environstr += (*it).second;
        environstr += " ";
    }
    program.prepend(environstr);

    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevautoproject/run/terminal");
    appFrontend()->startAppCommand(program, inTerminal);
}


void AutoProjectPart::slotAddTranslation()
{
    AddTranslationDialog dlg(this, m_widget);
    dlg.exec();
}


void AutoProjectPart::slotBuildConfigChanged(const QString &config)
{
    QDomDocument &dom = *projectDom();
    DomUtil::writeEntry(dom, "/kdevautoproject/general/useconfiguration", config);
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
	
	kdDebug ( 9000 ) << "*********************************************** 1) AutoProjectPart::savePartialProjectSession()" << endl;
	
	if ( domDoc.isNull() )
	{
		kdDebug ( 9000 ) << "*********************************************** 2) AutoProjectPart::savePartialProjectSession()" << endl;
		return;
	}
	
	kdDebug ( 9000 ) << "*********************************************** 3) AutoProjectPart::savePartialProjectSession()" << endl;
	
	m_widget->saveSession ( el );
}

#include "autoprojectpart.moc"
