/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appwizarddlg.h"

#include <qvbox.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qgrid.h>
#include <qheader.h>
#include <qmap.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtextview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <klistview.h>
#include <kiconview.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kfile.h>
#include <kapplication.h>
#include <kpopupmenu.h>

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kio/netaccess.h>
#include <qfile.h>
#include <kmacroexpander.h>
#include <karchive.h>
#include <ktar.h>
#include <ktempdir.h>
#include <kfileitem.h>
#include <kio/chmodjob.h>

#include <qlayout.h>

#include "domutil.h"
#include "kdevversioncontrol.h"
#include "kdevmakefrontend.h"
#include "kdevpartcontroller.h"
#include "kdevappfrontend.h"
#include "kdevplugininfo.h"
#include "kdevlicense.h"
#include "kdevcore.h"
#include "appwizardfactory.h"
#include "appwizardpart.h"
#include "filepropspage.h"
#include "misc.h"
#include "dataform.h"
#include "profilesupport.h"


AppWizardDialog::AppWizardDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : AppWizardDialogBase(parent, name,true), m_pCurrentAppInfo(0), 
	m_profileSupport(new ProfileSupport(part))
{
	kdDebug( 9000 ) << "  ** AppWizardDialog::AppWizardDialog()" << endl;

	m_customOptions = 0L;
	loadLicenses();
    connect( this, SIGNAL( selected( const QString & ) ), this, SLOT( pageChanged() ) );

	helpButton()->hide();
    templates_listview->header()->hide();
	templates_listview->setColumnWidthMode(0, QListView::Maximum);	//to provide horiz scrollbar.

	m_templatesMenu = new KPopupMenu(templates_listview);
	m_templatesMenu->insertItem(i18n("&Add to Favorites"), this, SLOT(addTemplateToFavourites()));

	m_favouritesMenu = new KPopupMenu(favourites_iconview);
	m_favouritesMenu->insertItem(i18n("&Remove Favorite"), this, SLOT(removeFavourite()));

    m_pathIsValid=false;
    m_part = part;
    m_projectLocationWasChanged=false;
    m_appsInfo.setAutoDelete(true);
    m_tempFiles.setAutoDelete(true);

    KConfig *config = kapp->config();

	//config->setGroup("AppWizard");
	//templates_tabwidget->setCurrentPage(config->readNumEntry("CurrentTab", 0));

	config->setGroup("General Options");
    QString defaultProjectsDir = config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/");

    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    QStringList m_templateNames = dirs->findAllResources("apptemplates", QString::null, false, true);

    kdDebug(9010) << "Templates: " << endl;
    QStringList categories;

    QStringList::Iterator it;
    for (it = m_templateNames.begin(); it != m_templateNames.end(); ++it) {
        kdDebug(9010) << (*it) << endl;

        ApplicationInfo *info = new ApplicationInfo;
		info->templateFile = KGlobal::dirs()->findResource("apptemplates", *it);
        info->templateName = (*it);

		KConfig templateConfig(info->templateFile);
        templateConfig.setGroup("General");

        info->name = templateConfig.readEntry("Name");
        info->icon = templateConfig.readEntry("Icon");
        info->comment = templateConfig.readEntry("Comment");
        info->fileTemplates = templateConfig.readEntry("FileTemplates");
        info->openFilesAfterGeneration = templateConfig.readListEntry("ShowFilesAfterGeneration");
        QString destDir = templateConfig.readPathEntry("DefaultDestinatonDir", defaultProjectsDir);
        destDir.replace(QRegExp("HOMEDIR"), QDir::homeDirPath());
        info->defaultDestDir = destDir;
        QString category = templateConfig.readEntry("Category");
        // format category to a unique status
        if (category.right(1) == "/")
            category.remove(category.length()-1, 1); // remove /
        if (category.left(1) != "/")
            category.prepend("/"); // prepend /
        categories.append(category);
        info->category = category;
		info->sourceArchive = templateConfig.readEntry("Archive");

		// Grab includes list
		QStringList groups = templateConfig.groupList();
		groups.remove("General");
		QStringList::Iterator group = groups.begin();
		for(  ; group != groups.end(); ++group)
		{
			templateConfig.setGroup( (*group) );
			QString type = templateConfig.readEntry("Type").lower();
			if( type == "include" )  // Add value
			{
				info->includes.append( templateConfig.readEntry( "File" ) );
				kdDebug(9010) << "Adding: " << templateConfig.readEntry( "File" ) << endl;
			}
		}

		// Build builtins map to bootstrap.
		QString source = kdevRoot( info->templateName );
		info->subMap.insert("kdevelop", source );

		// Add includes to the main template...
		QStringList::Iterator include = info->includes.begin();
		for( ; include != info->includes.end(); ++include)
		{
			if( !(*include).isEmpty() )
			{
				QString file = KMacroExpander::expandMacros( ( *include ), info->subMap);
				KConfig tmpCfg( file );
				tmpCfg.copyTo( "", &templateConfig);
				kdDebug(9010) << "Merging: " << tmpCfg.name() << endl;
			}
		}

		groups = templateConfig.groupList();  // Must get this again since its changed!
		group = groups.begin();
		for(  ; group != groups.end(); ++group)
		{
			templateConfig.setGroup( (*group) );
			QString type = templateConfig.readEntry("Type", "value").lower();
			kdDebug(9010) << "Reading " <<  (*group) << " of type " << type << endl;
			if( type == "value" )  // Add value
			{
				QString name = templateConfig.readEntry( "Value" );
				QString label = templateConfig.readEntry( "Comment" );
				QString type = templateConfig.readEntry( "ValueType", "String" );
				autoKey key( name, label);
				QVariant value = templateConfig.readPropertyEntry( "Default", QVariant::nameToType( type.latin1() ) );
				value.cast( QVariant::nameToType( type.latin1() ) );  // fix this in kdelibs...
				info->subValues.insert( key, value );
			}
			else if( type == "install" ) // copy dir
			{
				installFile file;
				file.source = templateConfig.readPathEntry("Source");
				file.dest = templateConfig.readPathEntry("Dest");
				file.process = templateConfig.readBoolEntry("Process",true);
				file.isXML = templateConfig.readBoolEntry("EscapeXML",false);
				file.option = templateConfig.readEntry("Option");
				info->fileList.append(file);
			}
			else if( type == "install archive" )
			{
				installArchive arch;
				arch.source = templateConfig.readPathEntry("Source");
				arch.dest = templateConfig.readPathEntry("Dest");
				arch.process = templateConfig.readBoolEntry("Process",true);
				arch.option = templateConfig.readEntry("Option", "" );
				info->archList.append(arch);
			}
			else if( type == "mkdir" )
			{
				installDir dir;
				dir.dir = templateConfig.readPathEntry("Dir");
				dir.option = templateConfig.readEntry("Option", "" );
				dir.perms = templateConfig.readNumEntry("Perms", 0777 );
				info->dirList.append(dir);
			}
			else if( type == "finishcmd" )
			{
                                info->finishCmd=templateConfig.readPathEntry("Command");
                                info->finishCmdDir=templateConfig.readPathEntry("Directory");
			}
			else if( type == "ui")
			{
				QString name = templateConfig.readPathEntry("File");
				info->customUI = name;
			}
			else if( type == "message" )
			{
				info->message = templateConfig.readEntry( "Comment" );
			}
		}


        m_appsInfo.append(info);
    }

    // Insert categories into list view
    categories.sort();
    for (it = categories.begin(); it != categories.end(); ++it)
        insertCategoryIntoTreeView(*it);

    // Insert items into list view
    QPtrListIterator<ApplicationInfo> ait(m_appsInfo);
    for (; ait.current(); ++ait) {
        QListViewItem *item = m_categoryMap.find(ait.current()->category);
        if (item)
		{
            item = new KListViewItem(item, ait.current()->name);
			item->setPixmap(0, SmallIcon("kdevelop"));
		}
        else
            kdDebug(9010) << "Error can't find category in categoryMap: "
                          << ait.current()->category << endl;
        ait.current()->item = item;
    }

	//Load favourites from config
	populateFavourites();

	QString author, email;
    AppWizardUtil::guessAuthorAndEmail(&author, &email);
    author_edit->setText(author);
    email_edit->setText(email);
    QToolTip::add( dest_edit->button(), i18n("Choose projects directory") );
    dest_edit->setURL(defaultProjectsDir);
    dest_edit->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    loadVcs();

    //    addPage(m_sdi_fileprops_page,"Class/File Properties");

    //    licenseChanged();

    setNextEnabled(generalPage, false);

//    QRegExp appname_regexp ("[a-zA-Z][a-zA-Z0-9_]*"); //Non-Unicode version
    /* appname will start with a letter, and will contain letters,
       digits or underscores. */
    QRegExp appname_regexp ("[a-zA-Z][a-zA-Z0-9_]*");
    // How about names like "__" or "123" for project name? Are they legal?
    QRegExpValidator *appname_edit_validator;
    appname_edit_validator = new QRegExpValidator (appname_regexp,
                                                   appname_edit,
                                                   "AppNameValidator");
    appname_edit->setValidator(appname_edit_validator);

    // insert the licenses into the license_combo
    QDict< KDevLicense > lics( licenses() );
    QDictIterator< KDevLicense > dit(lics);
    int idx=1;
    for( ; dit.current(); ++dit )
    {
        license_combo->insertItem( dit.currentKey(), idx++ );
        if( dit.currentKey() == "GPL" )
            license_combo->setCurrentItem( idx - 1 );
    }

	m_custom_options_layout = new QHBoxLayout( custom_options );
	m_custom_options_layout->setAutoAdd(true);
	
	showTemplates(true);
}

AppWizardDialog::~AppWizardDialog()
{}

void AppWizardDialog::loadVcs()
{
	m_vcsForm = new VcsForm();

	int i=0;
	m_vcsForm->combo->insertItem( i18n("no version control system", "None"), i );
	m_vcsForm->stack->addWidget( 0, i++ );

	// We query for all vcs integrators for KDevelop
	KTrader::OfferList offers = KTrader::self()->query("KDevelop/VCSIntegrator", "");
	KTrader::OfferList::const_iterator serviceIt = offers.begin();
	for (; serviceIt != offers.end(); ++serviceIt)
	{
		KService::Ptr service = *serviceIt;
		kdDebug(9010) << "AppWizardDialog::loadVcs: creating vcs integrator "
			<< service->name() << endl;

		KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
		if (!factory) {
			QString errorMessage = KLibLoader::self()->lastErrorMessage();
			kdDebug(9010) << "There was an error loading the module " << service->name() << endl <<
			"The diagnostics is:" << endl << errorMessage << endl;
			exit(1);
		}
		QStringList args;
		QObject *obj = factory->create(0, service->name().latin1(),
									"KDevVCSIntegrator", args);
		KDevVCSIntegrator *integrator = (KDevVCSIntegrator*) obj;

		if (!integrator)
			kdDebug(9010) << "    failed to create vcs integrator " << service->name() << endl;
		else
		{
			kdDebug(9010) << "    success" << endl;

			QString vcsName = service->property("X-KDevelop-VCS").toString();
			m_vcsForm->combo->insertItem(vcsName, i);
			m_integrators.insert(vcsName, integrator);

			VCSDialog *vcs = integrator->integrator(m_vcsForm->stack);
			if (vcs)
			{
				m_integratorDialogs[i] = vcs;
				QWidget *w = vcs->self();
				if (w)
					m_vcsForm->stack->addWidget(w, i++);
				else
					kdDebug(9010) << "    integrator widget is 0" << endl;
			}
			else
				kdDebug(9010) << "    integrator is 0" << endl;
		}
	}

	addPage(m_vcsForm, i18n("Version Control System"));
}

void AppWizardDialog::updateNextButtons()
{
	bool validGeneralPage = m_pCurrentAppInfo
							&& !appname_edit->text().isEmpty()
							&& m_pathIsValid;
	bool validPropsPage = !version_edit->text().isEmpty()
							&& !author_edit->text().isEmpty();

	setFinishEnabled(m_lastPage, validGeneralPage && validPropsPage);
	nextButton()->setEnabled(
		currentPage() == generalPage ? validGeneralPage : validPropsPage );
}

void AppWizardDialog::textChanged()
{
    licenseChanged();

	updateNextButtons();
}

void AppWizardDialog::licenseChanged()
{
	QValueList<AppWizardFileTemplate>::Iterator it;
	if( license_combo->currentItem() == 0 )
	{
		for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it)
		{
			QMultiLineEdit *edit = (*it).edit;
			edit->setText( QString::null );
		}
	} else {
		KDevLicense* lic = licenses()[ license_combo->currentText() ];
		for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it) {
			QString style = (*it).style;
			QMultiLineEdit *edit = (*it).edit;

			KDevFile::CommentingStyle commentStyle = KDevFile::CPPStyle;
			if (style == "PStyle") {
				commentStyle = KDevFile::PascalStyle;
			} else if (style == "AdaStyle") {
				commentStyle = KDevFile::AdaStyle;
			} else if (style == "ShellStyle") {
				commentStyle = KDevFile::BashStyle;
			}

			QString text;
			text = lic->assemble( commentStyle, author_edit->text(), email_edit->text() , 0 );
			edit->setText(text);
		}
	}
}

QString AppWizardDialog::kdevRoot(const QString &templateName ) const
{
	QString source;
    QFileInfo finfo(templateName);
    QDir dir(finfo.dir());
    dir.cdUp();
    return dir.absPath();
}

void AppWizardDialog::accept()
{
    QFileInfo fi(finalLoc_label->text());
    // check /again/ whether the dir already exists; maybe users create it in the meantime
    if (fi.exists()) {
        KMessageBox::sorry(this, i18n("The directory you have chosen as the location for "
                                      "the project already exists."));
        showPage(generalPage);
        appname_edit->setFocus();
        projectLocationChanged();
        return;
    }

	QString source = kdevRoot( m_pCurrentAppInfo->templateName );

	// Unpack template archive to temp dir, and get the name

	KTempDir archDir;
	archDir.setAutoDelete(true);
	KTar templateArchive( source + "/" + m_pCurrentAppInfo->sourceArchive, "application/x-gzip" );
	if( templateArchive.open( IO_ReadOnly ) )
	{
		//templateArchive.directory()->copyTo(archDir.name(), true);
		unpackArchive(templateArchive.directory(), archDir.name(), false);
	}
	else
	{
		KMessageBox::sorry(this, i18n("The template %1 cannot be opened.").arg( source + "/" + m_pCurrentAppInfo->sourceArchive ) );
		templateArchive.close();
		return;
	}
	templateArchive.close();

	// Build KMacroExpander map
	m_customOptions->dataForm()->fillPropertyMap(&m_pCurrentAppInfo->subMap);

	m_pCurrentAppInfo->subMap.insert("src", archDir.name() );
	m_pCurrentAppInfo->subMap.insert("dest", finalLoc_label->text() );
	m_pCurrentAppInfo->subMap.insert("APPNAME", appname_edit->text() );
	m_pCurrentAppInfo->subMap.insert("APPNAMELC", appname_edit->text().lower() );
	m_pCurrentAppInfo->subMap.insert("APPNAMESC", QString(appname_edit->text()[0]).upper() + appname_edit->text().mid(1));
	m_pCurrentAppInfo->subMap.insert("APPNAMEUC", appname_edit->text().upper() );
	m_pCurrentAppInfo->subMap.insert("AUTHOR", author_edit->text() );
	m_pCurrentAppInfo->subMap.insert("EMAIL", email_edit->text() );
	m_pCurrentAppInfo->subMap.insert("VERSION", version_edit->text());
	m_pCurrentAppInfo->subMap.insert("LICENSE", license_combo->currentText() );
	m_pCurrentAppInfo->subMap.insert( "I18N", "i18n" );

	QStringList cleanUpSubstMap;
	cleanUpSubstMap << "src" << "I18N" << "kdevelop";

	// Add template files to the fileList
	installDir templateDir;
	templateDir.dir = "%{dest}/templates";
	m_pCurrentAppInfo->dirList.prepend(templateDir);

	installDir baseDir;
	baseDir.dir = "%{dest}";
	m_pCurrentAppInfo->dirList.prepend( baseDir );

    QValueList<AppWizardFileTemplate>::Iterator it;
    for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it) {
        KTempFile *tempFile = new KTempFile();
        m_tempFiles.append(tempFile);

		QString templateText( (*it).edit->text() );
		QFile f;
		f.open(IO_WriteOnly, tempFile->handle());
		QTextStream temps(&f);
		temps << templateText;
		f.flush();
		QString templateName( QString( "%1_TEMPLATE" ).arg( (*it).suffix ).upper() );
		cleanUpSubstMap << templateName;
		m_pCurrentAppInfo->subMap.insert( templateName, KMacroExpander::expandMacros(templateText , m_pCurrentAppInfo->subMap)  );

		installFile file;
		file.source = tempFile->name();
		file.dest = QString( "%{dest}/templates/%1" ).arg( (*it).suffix );
		file.process = true;
		file.isXML = false;
		m_pCurrentAppInfo->fileList.append( file );
    }

	// Add license file to the file list
	QString licenseFile, licenseName = i18n("Custom");

    if( license_combo->currentItem() != 0 )
    {
        licenseName = license_combo->currentText();
        KDevLicense* lic = licenses()[ licenseName ];
        if( lic )
        {
            QStringList files( lic->copyFiles() );
			QStringList::Iterator it = files.begin();
			for( ; it != files.end(); ++it )
			{
				installFile file;
				file.source = QString( "%{kdevelop}/template-common/%1" ).arg( *it );
				file.dest = QString("%{dest}/%1").arg( *it );
				file.process = true;
				file.isXML = false;
				m_pCurrentAppInfo->fileList.append( file );
			}

			m_pCurrentAppInfo->subMap.insert("LICENSEFILE", files.first()  );
        }
    }

	// Run macro expander on both the dir map and file maps
	QValueList<installFile>::Iterator fileIt = m_pCurrentAppInfo->fileList.begin();
	for( ; fileIt != m_pCurrentAppInfo->fileList.end(); ++fileIt)
	{
		(*fileIt).source = KMacroExpander::expandMacros((*fileIt).source , m_pCurrentAppInfo->subMap);
		(*fileIt).dest = KMacroExpander::expandMacros((*fileIt).dest , m_pCurrentAppInfo->subMap);
	}

	QValueList<installArchive>::Iterator archIt = m_pCurrentAppInfo->archList.begin();
	for( ; archIt != m_pCurrentAppInfo->archList.end(); ++archIt)
	{
		(*archIt).source = KMacroExpander::expandMacros((*archIt).source , m_pCurrentAppInfo->subMap);
		(*archIt).dest = KMacroExpander::expandMacros((*archIt).dest , m_pCurrentAppInfo->subMap);
	}

	QValueList<installDir>::Iterator dirIt = m_pCurrentAppInfo->dirList.begin();
	for( ; dirIt != m_pCurrentAppInfo->dirList.end(); ++dirIt)
	{
		(*dirIt).dir = KMacroExpander::expandMacros((*dirIt).dir , m_pCurrentAppInfo->subMap);
	}

	QMap<QString,QString>::Iterator mapIt( m_pCurrentAppInfo->subMap.begin() );
	for( ; mapIt != m_pCurrentAppInfo->subMap.end(); ++mapIt )
	{
		QString escaped( mapIt.data() );
		escaped.replace( "&", "&amp;" );
		escaped.replace( "<", "&lt;" );
		escaped.replace( ">", "&gt;" );
		m_pCurrentAppInfo->subMapXML.insert( mapIt.key(), escaped );
	}

	// Create dirs
	dirIt = m_pCurrentAppInfo->dirList.begin();
	for( ; dirIt != m_pCurrentAppInfo->dirList.end(); ++dirIt)
	{
		kdDebug( 9000 ) << "Process dir " << (*dirIt).dir  << endl;
		if( m_pCurrentAppInfo->subMap[(*dirIt).option] != "false" )
		{
			if( ! KIO::NetAccess::mkdir( (*dirIt).dir, this ) )
			{
				KMessageBox::sorry(this, i18n("The directory %1 cannot be created.").arg( (*dirIt).dir ) );
				return;
			}
		}
	}
	// Unpack archives
	archIt = m_pCurrentAppInfo->archList.begin();
	for( ; archIt != m_pCurrentAppInfo->archList.end(); ++archIt)
	{
		if( m_pCurrentAppInfo->subMap[(*archIt).option] != "false" )
		{
			kdDebug( 9010 ) << "unpacking archive " << (*archIt).source << endl;
			KTar archive( (*archIt).source, "application/x-gzip" );
			if( archive.open( IO_ReadOnly ) )
			{
				unpackArchive( archive.directory(), (*archIt).dest, (*archIt).process );
			}
			else
			{
				KMessageBox::sorry(this, i18n("The archive %1 cannot be opened.").arg( (*archIt).source ) );
				archive.close();
				return;
			}
			archive.close();
		}

	}

	// Copy files & Process
	fileIt = m_pCurrentAppInfo->fileList.begin();
	for( ; fileIt != m_pCurrentAppInfo->fileList.end(); ++fileIt)
	{
		kdDebug( 9010 ) << "Process file " << (*fileIt).source << endl;
		if( m_pCurrentAppInfo->subMap[(*fileIt).option] != "false" )
		{
			if( !copyFile( *fileIt ) )
			{
				KMessageBox::sorry(this, i18n("The file %1 cannot be created.").arg( (*fileIt).dest) );
				return;
			}
			setPermissions(*fileIt);
		}
	}
    // if dir still does not exist
    if (!fi.dir().exists()) {
      KMessageBox::sorry(this, i18n("The directory above the chosen location does not exist and cannot be created."));
      showPage(generalPage);
      dest_edit->setFocus();
      return;
    }

//	KMessageBox::information(this, KMacroExpander::expandMacros(m_pCurrentAppInfo->message, m_pCurrentAppInfo->subMap));

	QStringList::Iterator cleanIt = cleanUpSubstMap.begin();
	for(;cleanIt != cleanUpSubstMap.end(); ++cleanIt )
	{
		m_pCurrentAppInfo->subMap.remove( *cleanIt );
	}

        if  (!m_pCurrentAppInfo->finishCmd.isEmpty())
           if (KDevAppFrontend *appFrontend = m_part->extension<KDevAppFrontend>("KDevelop/AppFrontend"))
              appFrontend->startAppCommand(KMacroExpander::expandMacros(m_pCurrentAppInfo->finishCmdDir, m_pCurrentAppInfo->subMap), KMacroExpander::expandMacros(m_pCurrentAppInfo->finishCmd, m_pCurrentAppInfo->subMap), false);

	openAfterGeneration();

	int id = m_vcsForm->stack->id(m_vcsForm->stack->visibleWidget());
	if (id)
	{
		VCSDialog *vcs = m_integratorDialogs[id];
		if (vcs)
		{
			kdDebug(9010) << "vcs integrator dialog is ready" << endl;
			vcs->accept();
		}
		else
			kdDebug(9010) << "no vcs integrator dialog" << endl;
	}
	else
		kdDebug(9010) << "vcs integrator wasn't selected" << endl;

	QWizard::accept();
}

bool AppWizardDialog::copyFile( const installFile& file )
{
	return
		copyFile( file.source, file.dest, file.isXML, file.process );
}

bool AppWizardDialog::copyFile( const QString &source, const QString &dest, bool isXML, bool process )
{
	kdDebug( 9010 ) << "Copy: " << source << " to " << dest << endl;
	if( process )
	{
		// Process the file and save it at the destFile location
		QFile inputFile( source);
		QFile outputFile( dest );
		const QMap<QString,QString> &subMap = isXML ?
			m_pCurrentAppInfo->subMapXML : m_pCurrentAppInfo->subMap;
		if( inputFile.open( IO_ReadOnly ) && outputFile.open(IO_WriteOnly) )
		{
			QTextStream input( &inputFile );
			QTextStream output( &outputFile );
			while( !input.atEnd() )
				output << KMacroExpander::expandMacros(input.readLine(), subMap) << "\n";
		}
		else
		{
			inputFile.close();
			outputFile.close();
			return false;
		}
	}
	else
	{
		// Copy the source file to the destFile.
		return KIO::NetAccess::copy( source, dest, this );
	}
	return true;
}

void AppWizardDialog::unpackArchive( const KArchiveDirectory *dir, const QString &dest, bool process )
{
	KIO::NetAccess::mkdir( dest , this );
	kdDebug(9010) << "Dir : " << dir->name() << " at " << dest << endl;
	QStringList entries = dir->entries();
	kdDebug(9010) << "Entries : " << entries.join(",") << endl;

	KTempDir tdir;

	QStringList::Iterator entry = entries.begin();
	for( ; entry != entries.end(); ++entry )
	{

		if( dir->entry( (*entry) )->isDirectory()  )
		{
			const KArchiveDirectory *file = (KArchiveDirectory *)dir->entry( (*entry) );
			unpackArchive( file , dest + "/" + file->name(), process);
		}
		else if( dir->entry( (*entry) )->isFile()  )
		{
			const KArchiveFile *file = (KArchiveFile *) dir->entry( (*entry) );
			if( !process )
			{
				file->copyTo( dest );
				setPermissions(file, dest + "/" + file->name());
			}
			else
			{
				file->copyTo(tdir.name());
				// assume that an archive does not contain XML files
				// ( where should we currently get that info from? )
				if ( !copyFile( QDir::cleanDirPath(tdir.name()+"/"+file->name()), dest + "/" + file->name(), false, process ) )
				{
					KMessageBox::sorry(this, i18n("The file %1 cannot be created.").arg( dest) );
					return;
				}
				setPermissions(file, dest + "/" + file->name());
			}
		}
	}
	tdir.unlink();
}

void AppWizardDialog::templatesTreeViewClicked(QListViewItem *item)
{
	if( m_customOptions )
		delete m_customOptions;

    // Delete old file template pages
    while (!m_fileTemplates.isEmpty()) {
        QMultiLineEdit *edit = m_fileTemplates.first().edit;
        removePage(edit);
        delete edit;
        m_fileTemplates.remove(m_fileTemplates.begin());
    }
    m_lastPage = 0;

    ApplicationInfo *info = templateForItem(item);
    if (info) {
        m_pCurrentAppInfo = info;
        if (!info->icon.isEmpty()) {
            QFileInfo fi(info->templateName);
            QDir dir(fi.dir());
            dir.cdUp();
            QPixmap pm;
            pm.load(dir.filePath(info->icon));
            icon_label->setPixmap(pm);
        } else {
            icon_label->clear();
        }
        desc_textview->setText(info->comment);
//        dest_edit->setURL(info->defaultDestDir);
        m_projectLocationWasChanged = false;
        //projectNameChanged(); // set the dest new

		// Populate new custom options form
		m_customOptions = new AutoForm( &m_pCurrentAppInfo->subValues, custom_options );

		custom_options->adjustSize();

        // Create new file template pages
        QStringList l = QStringList::split(",", info->fileTemplates);
        if (l.empty()) //if the app template doesn't show file templates, we need to set another m_lastPage, aleXXX
           m_lastPage=m_vcsForm;

        QStringList::ConstIterator it = l.begin();
        while (it != l.end()) {
            AppWizardFileTemplate fileTemplate;
            fileTemplate.suffix = *it;
            ++it;
            if (it != l.end()) {
                fileTemplate.style = *it;
                ++it;
            } else
                fileTemplate.style = "";

            QMultiLineEdit *edit = new QMultiLineEdit(this);
            edit->setWordWrap(QTextEdit::NoWrap);
            edit->setFont(KGlobalSettings::fixedFont());
            if (it == l.end())
                m_lastPage = edit;
            fileTemplate.edit = edit;
            addPage(edit, i18n("Template for .%1 Files").arg(fileTemplate.suffix));
            m_fileTemplates.append(fileTemplate);
        }
        textChanged(); // calls licenseChanged() && update Next button state
    } else {
	m_customOptions=0;
        m_pCurrentAppInfo=0;
        icon_label->clear();
        desc_textview->clear();
        nextButton()->setEnabled(false);
    }
}


void AppWizardDialog::destButtonClicked(const QString& dir)
{
    if(!dir.isEmpty()) {

        // set new location as default project dir?
        KConfig *config = kapp->config();
        config->setGroup("General Options");
        QDir defPrjDir( config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()) );
        QDir newDir (dir);
        kdDebug(9010) << "DevPrjDir == newdir?: " << defPrjDir.absPath() << " == " << newDir.absPath() << endl;
        if (defPrjDir != newDir) {
            if (KMessageBox::questionYesNo(this, i18n("Set default project location to: %1?").arg( newDir.absPath() ),
                                           i18n("New Project")) == KMessageBox::Yes)
            {
                config->writePathEntry("DefaultProjectsDir", newDir.absPath() + "/");
                config->sync();
            }
        }
    }
}


void AppWizardDialog::projectNameChanged()
{
    // Location was already edited by hand => don't change
}


void AppWizardDialog::projectLocationChanged()
{
  // Jakob Simon-Gaarde: Got tired of the anoying bug with the appname/location confussion.
  // This version insures WYSIWYG and checks pathvalidity
  finalLoc_label->setText(dest_edit->url() + (dest_edit->url().right(1)=="/" ? "":"/") + appname_edit->text().lower());
  QDir qd(dest_edit->url());
  QFileInfo fi(dest_edit->url() + "/" + appname_edit->text().lower());
  if (!qd.exists() || appname_edit->displayText().isEmpty()||fi.exists())
  {
    if (!fi.exists() || appname_edit->displayText().isEmpty()) {
      finalLoc_label->setText(finalLoc_label->text() + i18n("invalid location", " (invalid)"));
    } else {
      finalLoc_label->setText(finalLoc_label->text() + i18n(" (dir/file already exist)"));
    }
    m_pathIsValid=false;
  } else {
    m_pathIsValid=true;
  }
	updateNextButtons();
}


void AppWizardDialog::insertCategoryIntoTreeView(const QString &completeCategoryPath)
{
    kdDebug(9010) << "TemplateCategory: " << completeCategoryPath << endl;
    QStringList categories = QStringList::split("/", completeCategoryPath);
    QString category ="";
    QListViewItem* pParentItem=0;

    QStringList::ConstIterator it;
    for (it = categories.begin(); it != categories.end(); ++it) {
        category = category + "/" + *it;
        QListViewItem *item = m_categoryMap.find(category);
        if (!item) { // not found, create it
            if (!pParentItem)
                pParentItem = new KListViewItem(templates_listview,*it);
            else
                pParentItem = new KListViewItem(pParentItem,*it);

            pParentItem->setPixmap(0, SmallIcon("folder"));
            //pParentItem->setOpen(true);
            kdDebug(9010) << "Category: " << category << endl;
            m_categoryMap.insert(category,pParentItem);
        } else {
            pParentItem = item;
        }
    }
}


ApplicationInfo *AppWizardDialog::templateForItem(QListViewItem *item)
{
    QPtrListIterator<ApplicationInfo> it(m_appsInfo);
    for (; it.current(); ++it)
        if (it.current()->item == item)
            return it.current();

    return 0;
}

void AppWizardDialog::openAfterGeneration()
{
	QString projectFile( finalLoc_label->text() + "/" + appname_edit->text().lower() + ".kdevelop" );

	// Read the DOM of the newly created project
	QFile file( projectFile );
	if( !file.open( IO_ReadOnly ) )
		return;
	QDomDocument projectDOM;
	
	int errorLine, errorCol;
	QString errorMsg;
	bool success = projectDOM.setContent( &file, &errorMsg, &errorLine, &errorCol);
	file.close();
	if ( !success )
	{
		KMessageBox::sorry( 0, i18n("This is not a valid project file.\n"
				"XML error in line %1, column %2:\n%3")
				.arg(errorLine).arg(errorCol).arg(errorMsg));
		return;
	}

	// DOM Modifications go here
	DomUtil::writeMapEntry( projectDOM, "substmap", m_pCurrentAppInfo->subMap );

	//save the selected vcs
	KTrader::OfferList offers = KTrader::self()->query("KDevelop/VCSIntegrator", QString("[X-KDevelop-VCS]=='%1'").arg(m_vcsForm->combo->currentText()));
	if (offers.count() == 1)
	{
		KService::Ptr service = offers.first();
		DomUtil::writeEntry(projectDOM, "/general/versioncontrol", service->property("X-KDevelop-VCSPlugin").toString());
	}


//FIXME PROFILES!!!!!!!!
//BEGIN Plugin Profile

/*	QString category = m_pCurrentAppInfo->category;
	if ( category.left( 1 ) == "/" )
	{
		category = category.right( category.length() -1 );
	}

	QString profile = Profiles::getProfileForCategory( category );
	QStringList loadList = Profiles::getPluginsForProfile( profile );

	QStringList ignoreList;

	KTrader::OfferList offers = KTrader::self()->query("KDevelop/Plugin", "[X-KDevelop-Scope] == 'Project'");
	KTrader::OfferList::ConstIterator itt = offers.begin();
	while( itt != offers.end() )
	{
		if ( !loadList.contains( (*itt)->name() ) )
		{
			ignoreList << (*itt)->name();
		}
		++itt;
	}

	DomUtil::writeListEntry( projectDOM, "/general/ignoreparts", "part", ignoreList );*/

//END Plugin Profile

	// write the dom back
	if( !file.open( IO_WriteOnly ) )
		return;
	QTextStream ts( &file );
	ts << projectDOM.toString(2);
	file.close();

	// open the new project
	m_part->core()->openProject( projectFile );

	// open files to open
	QStringList::Iterator it = m_pCurrentAppInfo->openFilesAfterGeneration.begin();
	for( ; it != m_pCurrentAppInfo->openFilesAfterGeneration.end(); ++it )
	{
		QString fileName( *it );
		if ( !fileName.isNull() )
		{
			fileName = KMacroExpander::expandMacros(fileName, m_pCurrentAppInfo->subMap);
			m_part->partController()->editDocument( fileName );
		}
	}
}

void AppWizardDialog::pageChanged()
{
	kdDebug(9010) << "AppWizardDialog::pageChanged()" << endl;
	projectLocationChanged();

	//it is possible that project name was changed - we need to update all vcs integrator dialogs
	for (QMap<int, VCSDialog*>::iterator it = m_integratorDialogs.begin();
		it != m_integratorDialogs.end(); ++it)
		(*it)->init(getProjectName(), getProjectLocation());
}

void AppWizardDialog::addTemplateToFavourites()
{
	addFavourite(templates_listview->currentItem());
}

void AppWizardDialog::addFavourite(QListViewItem* item, QString favouriteName)
{
	if(item->childCount())
		return;

	ApplicationInfo* info = templateForItem(item);

	if(!info->favourite)
	{
		info->favourite = new KIconViewItem(favourites_iconview,
											((favouriteName=="")?info->name:favouriteName),
											DesktopIcon("kdevelop"));

		info->favourite->setRenameEnabled(true);
	}
}

ApplicationInfo* AppWizardDialog::findFavouriteInfo(QIconViewItem* item)
{
    QPtrListIterator<ApplicationInfo> info(m_appsInfo);
    for (; info.current(); ++info)
        if (info.current()->favourite == item)
            return info.current();

	return 0;
}

void AppWizardDialog::favouritesIconViewClicked( QIconViewItem* item)
{
	ApplicationInfo* info = findFavouriteInfo(item);
	templatesTreeViewClicked(info->item);
}

void AppWizardDialog::removeFavourite()
{
	QIconViewItem* curFavourite = favourites_iconview->currentItem();

	//remove reference to favourite from associated appinfo
	QPtrListIterator<ApplicationInfo> info(m_appsInfo);
	for (; info.current(); ++info)
	{
        if(info.current()->favourite && info.current()->favourite == curFavourite)
		{
			info.current()->favourite = 0;
		}
	}

	//remove favourite from iconview
	delete curFavourite;
	curFavourite=0;
	favourites_iconview->sort();	//re-arrange all items.
}

void AppWizardDialog::populateFavourites()
{
	KConfig* config = kapp->config();
	config->setGroup("AppWizard");

	//favourites are stored in config as a list of templates and a seperate
	//list of icon names.
	QStringList templatesList = config->readPathListEntry("FavTemplates");
	QStringList iconNamesList = config->readListEntry("FavNames");

	QStringList::Iterator curTemplate = templatesList.begin();
	QStringList::Iterator curIconName = iconNamesList.begin();
	while(curTemplate != templatesList.end())
	{
		QPtrListIterator<ApplicationInfo> info(m_appsInfo);
		for (; info.current(); ++info)
		{
			if(info.current()->templateName == *curTemplate)
			{
				addFavourite(info.current()->item, *curIconName);
				break;
			}
		}
		curTemplate++;
		curIconName++;
	}
}

void AppWizardDialog::done(int r)
{
	//need to save the template for each favourite and
	//it's icon name.  We have a one list for the templates
	//and one for the names.

	QStringList templatesList;
	QStringList iconNamesList;

	//Built the stringlists for each template that has a favourite.
	QPtrListIterator<ApplicationInfo> it(m_appsInfo);
	for (; it.current(); ++it)
	{
        if(it.current()->favourite)
		{
			templatesList.append(it.current()->templateName);
			iconNamesList.append(it.current()->favourite->text());
		}
	}

	KConfig* config = kapp->config();
	config->setGroup("AppWizard");
	config->writePathEntry("FavTemplates", templatesList);
	config->writeEntry("FavNames", iconNamesList);
	//config->writeEntry("CurrentTab", templates_tabwidget->currentPageIndex());
	config->sync();

	QDialog::done(r);
}

void AppWizardDialog::templatesContextMenu(QListViewItem* item, const QPoint& point, int)
{
	if(item && !item->childCount())
		m_templatesMenu->popup(point);
}

void AppWizardDialog::favouritesContextMenu(QIconViewItem* item, const QPoint& point)
{
	if(item)
		m_favouritesMenu->popup(point);
}

void AppWizardDialog::setPermissions(const KArchiveFile *source, QString dest)
{
	kdDebug(9010) << "AppWizardDialog::setPermissions(const KArchiveFile *source, QString dest)" << endl;
	kdDebug(9010) << "	dest: " << dest << endl;

	if (source->permissions() & 00100)
	{
		kdDebug(9010) << "source is executable" << endl;
		KIO::UDSEntry entry;
		KURL kurl = KURL::fromPathOrURL(dest);
		if (KIO::NetAccess::stat(kurl, entry, 0))
		{
			KFileItem it(entry, kurl);
			int mode = it.permissions();
			kdDebug(9010) << "stat shows permissions: " << mode << endl;
			KIO::chmod(KURL::fromPathOrURL(dest), mode | 00100 );
		}
	}
}

void AppWizardDialog::setPermissions(const installFile &file)
{
	kdDebug(9010) << "AppWizardDialog::setPermissions(const installFile &file)" << endl;
	kdDebug(9010) << "	dest: " << file.dest << endl;

	KIO::UDSEntry sourceentry;
	KURL sourceurl = KURL::fromPathOrURL(file.source);
	if (KIO::NetAccess::stat(sourceurl, sourceentry, 0))
	{
		KFileItem sourceit(sourceentry, sourceurl);
		int sourcemode = sourceit.permissions();
		if (sourcemode & 00100)
		{
			kdDebug(9010) << "source is executable" << endl;
			KIO::UDSEntry entry;
			KURL kurl = KURL::fromPathOrURL(file.dest);
			if (KIO::NetAccess::stat(kurl, entry, 0))
			{
				KFileItem it(entry, kurl);
				int mode = it.permissions();
				kdDebug(9010) << "stat shows permissions: " << mode << endl;
				KIO::chmod(KURL::fromPathOrURL(file.dest), mode | 00100 );
			}
		}
	}
}

QDict<KDevLicense> AppWizardDialog::licenses()
{
	return m_licenses;
}

void AppWizardDialog::loadLicenses()
{
	// kdDebug(9010) << "======================== Entering loadLicenses" << endl;
	KStandardDirs* dirs = KGlobal::dirs();
	dirs->addResourceType( "licenses", KStandardDirs::kde_default( "data" ) + "kdevelop/licenses/" );
	QStringList licNames = dirs->findAllResources( "licenses", QString::null, false, true );

	QStringList::Iterator it;
	for (it = licNames.begin(); it != licNames.end(); ++it)
	{
		QString licPath( dirs->findResource( "licenses", *it ) );
		// kdDebug(9000) << "Loading license file: " << licPath << endl;
		QString licName = licPath.mid( licPath.findRev('/') + 1 );
		KDevLicense* lic = new KDevLicense( licName, licPath );
		m_licenses.insert( licName, lic );
	}
	// kdDebug(9000) << "======================== Done loadLicenses" << endl;
}

void AppWizardDialog::showTemplates(bool all)
{
	if (all)
	{
		QListViewItemIterator it(templates_listview);
		while ( it.current() ) {
			it.current()->setVisible(true);
			++it;
		}
	}
	else
	{
		QPtrListIterator<ApplicationInfo> ait(m_appsInfo);
		for (; ait.current(); ++ait) 
		{
			ait.current()->item->setVisible(m_profileSupport->isInTemplateList(ait.current()->templateName));
		}
		
		QDictIterator<QListViewItem> dit(m_categoryMap);
		for (; dit.current(); ++dit) 
		{
			//checking whether all children are not visible
			kdDebug(9010) << "check: " << dit.current()->text(0) << endl;
			bool visible = false;
			QListViewItemIterator it(dit.current());
			while ( it.current() ) {
				if ((it.current()->childCount() == 0) && it.current()->isVisible())
				{
					kdDebug(9010) << "	visible: " << it.current()->text(0) << endl;
					visible = true;
					break;
				}
				++it;
			}
			dit.current()->setVisible(visible);
		}
	}
}


#include "appwizarddlg.moc"

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
