/***************************************************************************
                          haskellproject_part.cpp  -  description
                             -------------------
    begin                : Mon Aug 11 2003
    copyright            : (C) 2003 Peter Robinson
    email                : listener@thaldyron.com
 ***************************************************************************/

/***************************************************************************
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
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <kservice.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kprocess.h>
#include <ktrader.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevcompileroptions.h"
#include "runoptionswidget.h"
#include "haskellprojectoptionsdlg.h"
#include "haskellproject_part.h"

typedef KGenericFactory<HaskellProjectPart> HaskellProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevhaskellproject, HaskellProjectFactory( "kdevhaskellproject" ) )


HaskellProjectPart::HaskellProjectPart(QObject *parent, const char *name, const QStringList& )
  : KDevProject("KDevPart", "kdevpart", parent, name ? name : "HaskellProjectPart" )
{
  	setInstance(HaskellProjectFactory::instance());
  	setXMLFile("kdevhaskellproject.rc");

 // _widget = new HaskellProjectWidget(this);

  	_buildAction = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
    		                      this, SLOT(slotBuild()),
            		              actionCollection(), "build_build" );
	_runAction = new KAction( i18n("Execute Program"), "exec", 0,
    	                      this, SLOT(slotExecute()),
        	                  actionCollection(), "build_execute" );

  	connect( core(), SIGNAL( projectConfigWidget( KDialogBase* ) ),
             this, SLOT( projectConfigWidget( KDialogBase* ) ) );

  	connect( core(), SIGNAL( configWidget( KDialogBase* ) ),
     	     this, SLOT( configWidget( KDialogBase* ) ) );                          
}


HaskellProjectPart::~HaskellProjectPart()
{
  //delete _widget;
}


void HaskellProjectPart::openProject(const QString &dirName, const QString &projectName)
{
  	_buildDir = dirName;
    _projectDir = dirName;
    _projectName = projectName;
    loadProjectConfig();
  	QDomDocument &dom = *projectDom();
    QString directoryRadioString = DomUtil::readEntry( dom,
  	  								"/kdevhaskellproject/run/directoryradio" );
  	QString mainProgram = DomUtil::readEntry( dom,
  									"/kdevhaskellproject/run/mainprogram" );
	QString customDir = DomUtil::readEntry( dom,
  									"/kdevhaskellproject/run/customdirectory" );
}

void HaskellProjectPart::closeProject()
{
}


QString HaskellProjectPart::projectDirectory() const
{
    return _projectDir;
}

QString HaskellProjectPart::projectName() const
{
    return _projectName;
}

/** Retuns a PairList with the run environment variables */
DomUtil::PairList HaskellProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry(*projectDom(),
    								  "/kdevhaskellproject/run/envvars",
              						  "envvar", "name", "value");
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
QString HaskellProjectPart::mainProgram( bool relative ) const
{
	QString progName;
  	QDomDocument &dom = *projectDom();
  	QString directoryRadioString = DomUtil::readEntry( dom,
  										"/kdevhaskellproject/run/directoryradio" );
  	QString mainProgram = DomUtil::readEntry( dom,
  										"/kdevhaskellproject/run/mainprogram" );

  	if( mainProgram.isEmpty() ) {
    	QFileInfo fileInfo( mainSource() );
	    progName = buildDirectory() + "/" + fileInfo.baseName();
    	if( relative ) {
	     	return fileInfo.baseName();
    	}
    	else {
     		return buildDirectory() + "/" + fileInfo.baseName();
    	}
  	}
  	else {
  		if( directoryRadioString == "custom" ) {
   			if( relative ) {
     			return( mainProgram );
      		}
      		else {
				QString customDir = DomUtil::readEntry( dom,
  										"/kdevhaskellproject/run/customdirectory" );
      			return( customDir + '/' + mainProgram );
      		}
		}
		else if( directoryRadioString == "build" ) {
	  		if( relative ) {
	   			return( mainProgram );
	    	}
	  	}
	}
	return QString::null;      
}

QString HaskellProjectPart::runDirectory() const
{
    QDomDocument &dom = *projectDom();

    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevhaskellproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevhaskellproject/run/mainprogram");

    if ( directoryRadioString == "build" )
        return buildDirectory();

    if ( directoryRadioString == "custom" )
        return DomUtil::readEntry(dom, "/kdevhaskellproject/run/customdirectory");

    int pos = DomMainProgram.findRev('/');
    if (pos != -1)
        return buildDirectory() + "/" + DomMainProgram.left(pos);

    return buildDirectory() + "/" + DomMainProgram;
}

/** Retuns a QString with the run command line arguments */
QString HaskellProjectPart::runArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevhaskellproject/run/programargs");
}

QString HaskellProjectPart::activeDirectory() const
{
    QFileInfo fi(mainSource());
   			// @todo shouldn't that be false = relative path?
    return fi.dirPath(true).replace(QRegExp(projectDirectory()),"");
}

QString HaskellProjectPart::buildDirectory() const
{
    QFileInfo fi(mainSource());
    return fi.dirPath(true);
}

QStringList HaskellProjectPart::allFiles() const
{
    return _sourceFiles;
}

void HaskellProjectPart::addFiles(const QStringList& /*fileList*/)
{
}


void HaskellProjectPart::addFile(const QString& /*fileName*/)
{
}

void HaskellProjectPart::removeFiles(const QStringList& /*fileList*/)
{
}

void HaskellProjectPart::removeFile(const QString& /*fileName*/)
{
}


/*QString HaskellProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();
    QString configMainProg = DomUtil::readEntry(dom, "/kdevhaskellproject/run/mainprogram", "");
    if (configMainProg.isEmpty())
    {
        QFileInfo fi(mainSource());
        return buildDirectory() + "/" + fi.baseName();
    }
    else
        return QDir::cleanDirPath(projectDirectory() + "/" + configMainProg);
}*/

QString HaskellProjectPart::mainSource() const
{
    return projectDirectory() + "/" + _mainSource;
}

void HaskellProjectPart::setMainSource(QString fullPath)
{
    _mainSource = fullPath.replace(QRegExp(QString(projectDirectory() + QString("/"))),"");
}



void HaskellProjectPart::listOfFiles(QStringList &result, QString path)
{
    QDir d(path);
    if (!d.exists())
        return;

    QFileInfoList *entries = const_cast<QFileInfoList*>(d.entryInfoList(QDir::Dirs |
    														QDir::Files | QDir::Hidden));
    for (QFileInfo *it = entries->first(); it; it = entries->next()) {
        if ((it->isDir()) && (!(it->filePath() == path))) {
            listOfFiles(result, it->dirPath());
        }
        else
        {
            result << it->filePath();
        }
    }
}

QString HaskellProjectPart::createPackageString()
{
	// @todo create "-package network -package concurrent" etc.
	return "";
}

QString HaskellProjectPart::createCmdLine( QString srcFile)
{
	// @todo test which haskell comp/interpreter is used and build cmdLine accordingly
	// at the moment only ghc is supported
  	QString cmdLine = _compilerExec + " " + createPackageString() + " " + srcFile + " "
   									+ _compilerOpts + " -o " + mainProgram();
  	QString dirCmd = "cd ";
  	dirCmd += KProcess::quote(buildDirectory());
  	dirCmd += " && ";
	return dirCmd + cmdLine;
}

void HaskellProjectPart::slotBuild()
{
    partController()->saveAllFiles();

    if (_compilerExec.isEmpty()) {
      	KMessageBox::sorry(0, i18n("Could not find the Haskell Translator.\nCheck if your settings are correct."));
      	return;
    }

    QString cmdline = createCmdLine( QFileInfo( mainSource() ).fileName() );
    makeFrontend()->queueCommand( buildDirectory(), cmdline );
}

void HaskellProjectPart::slotExecute()
{
    partController()->saveAllFiles();

    QDomDocument &dom = *(projectDom());
    bool runInTerminal = DomUtil::readBoolEntry(dom, "/kdevhaskellproject/run/terminal", true);

    // Get the run environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars =	DomUtil::readPairListEntry(	*projectDom(),
			         				"/kdevhaskellproject/run/envvars",
			             			"envvar", "name", "value");
    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
#if (KDE_VERSION > 305)
        environstr += KProcess::quote((*it).second);
#else
        environstr += KShellProcess::quote((*it).second);
#endif
        environstr += " ";
    }

    QString program = mainProgram();
    program.prepend( environstr );
    program += " " + DomUtil::readEntry(*projectDom(),
    																"/kdevhaskellproject/run/programargs" );

    appFrontend()->startAppCommand(buildDirectory(), program, runInTerminal);
}

void HaskellProjectPart::changedFiles( const QStringList & fileList )
{
    KDevProject::changedFiles( fileList );
}

void HaskellProjectPart::changedFile( const QString & fileName )
{
    KDevProject::changedFile( fileName );
}

void HaskellProjectPart::projectConfigWidget( KDialogBase * dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage( i18n("Haskell Options") );
    HaskellProjectOptionsDlg *optionsDlg = new HaskellProjectOptionsDlg( this, vbox );
    
    connect( dlg, SIGNAL( okClicked() ),
    				 optionsDlg, SLOT( accept() ) );
         
    connect( dlg, SIGNAL( okClicked() ),
    				 this, SLOT( loadProjectConfig() ) );

    vbox = dlg->addVBoxPage( i18n( "Run Options" ) );
    RunOptionsWidget *w3 = new RunOptionsWidget( *projectDom(),
    								"/kdevhaskellproject", buildDirectory(), vbox );
    w3->mainprogram_label->setText( i18n( "Main program (relative to project directory):" ) );
    connect( dlg, SIGNAL(okClicked()),
    		 w3, SLOT( accept() ) );
}

void HaskellProjectPart::loadProjectConfig()
{
  	QDomDocument &dom = *(projectDom());

  	QString config = DomUtil::readEntry(dom,
   								"/kdevhaskellproject/general/useconfiguration",
           						"default");
  	_mainSource = DomUtil::readEntry(dom,
   								QString("/kdevhaskellproject/configurations/") +
           						config + QString("/mainsource") );
  	_compilerOpts = DomUtil::readEntry(dom,
   								QString("/kdevhaskellproject/configurations/") +
           						config + QString("/compileroptions"));
  	_compilerExec = DomUtil::readEntry(dom,
   								QString("/kdevhaskellproject/configurations/") +
           						config + QString("/compilerexec"));

  	if ( _compilerExec.isEmpty() ) {
		KTrader::OfferList offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Haskell'");
    	QValueList<KService::Ptr>::ConstIterator it;
    	for (it = offers.begin(); it != offers.end(); ++it) {
      		if ( (*it)->property( "X-KDevelop-Default" ).toBool() ) {
      			_compilerExec = (*it)->exec();
        		break;
      		}
    	}
  	}
  // no entries found set standard to Glasgow Haskell Compiler
//  if( _compilerExec.isEmpty() ) {
//  	_compilerExec = "ghc";
//    _compilerOpts = "--make";
//  }
}

void HaskellProjectPart::configWidget( KDialogBase * dlg )
{
	// @todo create config page for KDevelop Settings
    /*QVBox *vbox;
    vbox = dlg->addVBoxPage( i18n( "Haskell Options" ) );
    HaskellGlobalOptionsDlg *w = new HaskellGlobalOptionsDlg( this, vbox );
    connect( dlg, SIGNAL( okClicked() ), w, SLOT( accept() ) );*/
}

KDevCompilerOptions *HaskellProjectPart::createCompilerOptions(const QString &name)
{
    KService::Ptr service = KService::serviceByName( name );
    if ( !service ) {
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
    KDevCompilerOptions *dlg = (KDevCompilerOptions*)obj;

    return dlg;
}

QString HaskellProjectPart::defaultOptions( const QString compiler )
{
    KConfig *config = KGlobal::config();
    config->setGroup("Haskell Compiler");
kdDebug(9000) << "*********** " << config->readPathEntry( compiler ) << "::" << endl;
    return config->readPathEntry(compiler);
}

QString HaskellProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();
    QString configMainProg = DomUtil::readEntry(dom, "/kdevhaskellproject/run/mainprogram", "");
    if (configMainProg.isEmpty())
    {
        QFileInfo fi(mainSource());
        return buildDirectory() + "/" + fi.baseName();
    }
    else
        return QDir::cleanDirPath(projectDirectory() + "/" + configMainProg);
}


/*!
    \fn HaskellProjectPart::distFiles() const
 */
QStringList HaskellProjectPart::distFiles() const
{
   	QStringList sourceList = allFiles();
	// Scan current source directory for any .pro files.
	QString projectDir = projectDirectory();
	QDir dir(projectDir);
	QStringList files = dir.entryList( "*README*");
	return sourceList + files;
}

#include "haskellproject_part.moc"
