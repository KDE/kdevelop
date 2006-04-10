/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *   Copyright (C) 2004-2005 by Sascha Cunz                                *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//BEGIN almost 100 Includes

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <q3vbox.h>
#include <q3buttongroup.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <QHeaderView>
#include <qmap.h>
#include <QTextEdit>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <q3textview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <QPixmap>
#include <QHBoxLayout>
#include <QDate>

#include <k3listview.h>
#include <k3iconview.h>
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
#include <kmenu.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kio/netaccess.h>
#include <kmacroexpander.h>
#include <ktempdir.h>
#include <kfileitem.h>
#include <kio/chmodjob.h>

#include "domutil.h"
#include "kdevmakefrontend.h"
#include "kdevapptemplate.h"
#include "filetemplate.h"
#include "kdevdocumentcontroller.h"
#include "kdevplugininfo.h"
#include "kdevlicense.h"
#include "kdevcore.h"
#include "appwizardfactory.h"
#include "appwizarddlg.h"
#include "misc.h"
#include "profilesupport.h"

//END Includes

AppWizardDialog::AppWizardDialog(AppWizardPart *appwizardpart, QWidget *parent, const char *name)
    : Q3Wizard(parent, name, true),
      m_part(appwizardpart),
      m_currentTemplate(0),
      m_profileSupport(new ProfileSupport(appwizardpart))
{
    kDebug(9000) << "  ** AppWizardDialog::AppWizardDialog()" << endl;

    setupUi(this);

    loadLicenses();

    connect( this, SIGNAL( selected( const QString & ) ),
             this, SLOT( pageChanged() ) );
    connect( templates_listview, SIGNAL( pressed(const QModelIndex&) ),
             this, SLOT( templatesTreeViewActivated(const QModelIndex&) ) );
    connect( version_edit, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( updateNextButtons() ) );
    connect( author_edit, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( updateNextButtons() ) );
    connect( appname_edit, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( updateNextButtons() ) );
    connect( appname_edit, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( projectLocationChanged() ) );
    connect( dest_edit, SIGNAL( urlSelected( const QString & ) ),
             this, SLOT( destButtonClicked( const QString & ) ) );
    connect( dest_edit, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( projectLocationChanged() ) );
    connect( dest_edit, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( updateNextButtons() ) );
//  connect( favourites_iconview, SIGNAL( selectionChanged( Q3IconViewItem* ) ),
//           this, SLOT( favouritesIconViewClicked( Q3IconViewItem* ) ) );
//  connect( templates_listview, SIGNAL( contextMenuRequested( Q3ListViewItem*, const QPoint &, int ) ),
//           this, SLOT( templatesContextMenu( Q3ListViewItem* ,const QPoint & , int) ) );
//  connect( favourites_iconview, SIGNAL( contextMenuRequested(Q3IconViewItem*,const QPoint& ) ),
//           this, SLOT( favouritesContextMenu(Q3IconViewItem*,const QPoint & ) ) );
    connect( showAll_box, SIGNAL( toggled( bool ) ),
             this, SLOT( showTemplates( bool ) ) );
    connect( license_combo, SIGNAL( activated( int ) ),
             this, SLOT( licenseChanged() ) );

    helpButton()->hide();
    templates_listview->setModel( new KDevAppTemplateModel(this) );
    templates_listview->header()->hide();

/*
    m_templatesMenu = new KMenu(templates_listview);
    m_templatesMenu->insertItem(i18n("&Add to Favorites"), this, SLOT(addTemplateToFavourites()));

    m_favouritesMenu = new KMenu(favourites_iconview);
    m_favouritesMenu->insertItem(i18n("&Remove Favorite"), this, SLOT(removeFavourite()));
*/
    m_pathIsValid=false;
    m_projectLocationWasChanged=false;
//  populateFavourites();

    KConfig *config = KGlobal::config();
    config->setGroup("General Options");
    QString defaultProjectsDir = config->readPathEntry("DefaultProjectsDir", QDir::homePath()+"/");

    QString author, email;
    AppWizardUtil::guessAuthorAndEmail(&author, &email);
    author_edit->setText(author);
    email_edit->setText(email);
    QToolTip::add( dest_edit->button(), i18n("Choose projects directory") );
    dest_edit->setURL(defaultProjectsDir);
    dest_edit->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    loadVcs();

    setNextEnabled(generalPage, false);

    /* appname will start with a letter, and will contain letters,
       digits or underscores. */
    QRegExp appname_regexp ("[a-zA-Z][a-zA-Z0-9_]*");
    // How about names like "__" or "123" for project name? Are they legal?
    QRegExpValidator *appname_edit_validator;
    appname_edit_validator = new QRegExpValidator (appname_regexp,
                                                   appname_edit,
                                                   "AppNameValidator");
    appname_edit->setValidator(appname_edit_validator);

    m_custom_options_layout = new QHBoxLayout( custom_options );
    m_custom_options_layout->setAutoAdd(true);

    showTemplates(false);
}

AppWizardDialog::~AppWizardDialog()
{
    KTempFile* tf;
    foreach( tf, m_tempFiles )
        delete tf;
    // calling this, because of the m_fileTemplates
    clearTemplateSelection();
}

void AppWizardDialog::loadVcs()
{
}

void AppWizardDialog::updateNextButtons()
{
    bool validGeneralPage = m_currentTemplate
                            && !appname_edit->text().isEmpty()
                            && m_pathIsValid;
    bool validPropsPage = !version_edit->text().isEmpty()
                            && !author_edit->text().isEmpty();

    setFinishEnabled(m_lastPage, validGeneralPage && validPropsPage);
    nextButton()->setEnabled(
        currentPage() == generalPage ? validGeneralPage : validPropsPage );
}

void AppWizardDialog::licenseChanged()
{
    if( license_combo->currentItem() == 0 )
    {
        foreach( AppWizardFileTemplate* templ, m_fileTemplates )
            templ->edit->setText( QString() );
    } else
    {
        KDevLicense* lic = m_licenses[ license_combo->currentText() ];
        foreach( AppWizardFileTemplate* templ, m_fileTemplates )
        {
            KDevFile::CommentingStyle commentStyle = KDevFile::CPPStyle;
            if( templ->style == "PStyle" )
                commentStyle = KDevFile::PascalStyle;
            else if( templ->style == "AdaStyle" )
                commentStyle = KDevFile::AdaStyle;
            else if( templ->style == "ShellStyle" )
                commentStyle = KDevFile::BashStyle;

            QString text;
            text = lic->assemble( commentStyle, author_edit->text(), email_edit->text() , 0 );
            templ->edit->setText(text);
        }
    }
}

void AppWizardDialog::accept()
{
    if( !m_currentTemplate )    // safety
        return;

    // check /again/ whether the dir already exists;
    // maybe users create it in the meantime
    QFileInfo fi(finalLoc_label->text());
    if( fi.exists() )
    {
        KMessageBox::sorry(this, i18n("The directory you have chosen as the location for "
                                      "the project already exists."));
        showPage(generalPage);
        appname_edit->setFocus();
        projectLocationChanged();
        return;
    }

    m_currentTemplate->delayedLoadDetails();
    m_currentTemplate->unpackTemplateArchive();

    // Build KMacroExpander map
    //m_customOptions->dataForm()->fillPropertyMap(&m_currentTemplate->subMap());
//  PropertyLib::PropertyList::Iterator idx = m_currentTemplate->propValues->begin();
//  for( ; idx != m_currentTemplate->propValues->end(); ++idx)
//      m_currentTemplate->addToSubMap( idx.data()->name(), idx.data()->value().toString() );

    m_currentTemplate->addToSubMap("dest", finalLoc_label->text() );
    m_currentTemplate->addToSubMap("APPNAME", appname_edit->text() );
    m_currentTemplate->addToSubMap("APPNAMELC", appname_edit->text().lower() );
    m_currentTemplate->addToSubMap("APPNAMESC", QString(appname_edit->text()[0]).upper() + appname_edit->text().mid(1));
    m_currentTemplate->addToSubMap("APPNAMEUC", appname_edit->text().upper() );
    m_currentTemplate->addToSubMap("AUTHOR", author_edit->text() );
    m_currentTemplate->addToSubMap("EMAIL", email_edit->text() );
    m_currentTemplate->addToSubMap("VERSION", version_edit->text());
    m_currentTemplate->addToSubMap("I18N", "i18n" );
    m_currentTemplate->addToSubMap("YEAR", QString::number( QDate::currentDate().year() ) );

    // Add license file to the file list
    QString licenseFile, licenseName = i18n("Custom");

    KDevLicense* lic = 0;
    if( license_combo->currentItem() != 0 )
    {
        licenseName = license_combo->currentText();
        if( (lic = m_licenses[ licenseName ]) )
        {
            m_currentTemplate->addToSubMap("LICENSE", lic->aboutDataEnum() );
            QStringList files( lic->copyFiles() );
            foreach( QString fileName, files )
            {
                KDevAppTemplate::File file;
                file.source = QString("%{kdevelop}/template-common/") + fileName;
                file.dest = QString("%{dest}/") + fileName;
                file.process = true;
                file.isXML = false;
                m_currentTemplate->addFile( file );
            }
            m_currentTemplate->addToSubMap("LICENSEFILE", files.first()  );
        }
    }
    if( !lic )
        m_currentTemplate->addToSubMap("LICENSE", "Custom" );

    QStringList cleanUpSubstMap;
    cleanUpSubstMap << "src" << "I18N" << "kdevelop";

    // Add template files to the fileList
    KDevAppTemplate::Dir templateDir( "%{dest}/templates" );
    m_currentTemplate->addDir( templateDir );

    KDevAppTemplate::Dir baseDir( "%{dest}" );
    m_currentTemplate->addDir( baseDir );

    //BEGIN Silly DOM code
    // This is too silly for words, but it's either this or reimplementing FileTemplate
    QString tempProjectDomSource = "<!DOCTYPE kdevelop><kdevelop><general><author>%1</author><email>%2</email><version>%3</version></general></kdevelop>";
    tempProjectDomSource = tempProjectDomSource.arg( author_edit->text() ).arg( email_edit->text() ).arg( version_edit->text() );
    QDomDocument tempProjectDom;
    tempProjectDom.setContent( tempProjectDomSource );

    foreach( AppWizardFileTemplate* templ, m_fileTemplates )
    {
        KTempFile *tempFile = new KTempFile();
        m_tempFiles.append(tempFile);

        QString templateText( FileTemplate::makeSubstitutions( tempProjectDom, templ->edit->text() ) );
        QFile f;
        f.open(QIODevice::WriteOnly, tempFile->handle());
        QTextStream temps(&f);
        temps << templateText;
        f.flush();
        QString templateName( QString( "%1_TEMPLATE" ).arg( templ->suffix ).upper() );
        cleanUpSubstMap << templateName;
        m_currentTemplate->addToSubMap( templateName, KMacroExpander::expandMacros(templateText , m_currentTemplate->subMap())  );

        KDevAppTemplate::File file;
        file.source = tempFile->name();
        file.dest = QString( "%{dest}/templates/" ) + templ->suffix;
        file.process = true;
        file.isXML = false;
        m_currentTemplate->addFile( file );
    }
    //END

    kDebug(9010) << "expandLists" << endl;
    /* Crashes:
    m_currentTemplate->expandLists();
    */

    // Crashs as well:
    QList<KDevAppTemplate::File>::Iterator fileIt = m_currentTemplate->m_fileList.begin();
    for( ; fileIt != m_currentTemplate->m_fileList.end(); ++fileIt)
    {
        (*fileIt).source = KMacroExpander::expandMacros((*fileIt).source , m_currentTemplate->subMap());
        (*fileIt).dest = KMacroExpander::expandMacros((*fileIt).dest , m_currentTemplate->subMap());
    }

    QList<KDevAppTemplate::Archive>::Iterator archIt = m_currentTemplate->m_archList.begin();
    for( ; archIt != m_currentTemplate->m_archList.end(); ++archIt)
    {
        (*archIt).source = KMacroExpander::expandMacros((*archIt).source , m_currentTemplate->subMap());
        (*archIt).dest = KMacroExpander::expandMacros((*archIt).dest , m_currentTemplate->subMap());
    }

    QList<KDevAppTemplate::Dir>::Iterator dirIt = m_currentTemplate->m_dirList.begin();
    for( ; dirIt != m_currentTemplate->m_dirList.end(); ++dirIt)
    {
        (*dirIt).dir = KMacroExpander::expandMacros((*dirIt).dir , m_currentTemplate->subMap());
    }

    kDebug(9010) << "setSubMapXML" << endl;
    m_currentTemplate->setSubMapXML();
    kDebug(9010) << "installProject" << endl;
    m_currentTemplate->installProject( this );

    // if dir still does not exist
    if (!fi.dir().exists())
    {
      KMessageBox::sorry(this, i18n("The directory above the chosen location does not exist and cannot be created."));
      showPage(generalPage);
      dest_edit->setFocus();
      return;;
    }

    foreach( QString kill, cleanUpSubstMap )
        m_currentTemplate->removeFromSubMap( kill );

    /**
     * @todo   When implementing KNewStuff, allowing a template to contain
     *         commands that are executed on the computer might be a
     *         security impact!
     */
    m_currentTemplate->execFinishCommand( m_part );

    openAfterGeneration();

    /*
    int id = m_vcsForm->stack->id(m_vcsForm->stack->visibleWidget());
    if (id)
    {
        VCSDialog *vcs = m_integratorDialogs[id];
        if (vcs)
        {
            kDebug(9010) << "vcs integrator dialog is ready" << endl;
            vcs->accept();
        }
        else
            kDebug(9010) << "no vcs integrator dialog" << endl;
    }
    else
        kDebug(9010) << "vcs integrator wasn't selected" << endl;
    */
    Q3Wizard::accept();
}

void AppWizardDialog::clearTemplateSelection()
{
    //delete m_customOptions;

    // Delete old file template pages
    while( !m_fileTemplates.isEmpty() )
    {
        QTextEdit *edit = m_fileTemplates.first()->edit;
        removePage(edit);
        delete edit;
        m_fileTemplates.remove(m_fileTemplates.begin());
    }
    m_lastPage = 0;

    desc_textview->setText(QString());
    icon_label->clear();

    m_currentTemplate = 0;
}

void AppWizardDialog::templatesTreeViewActivated(const QModelIndex& index)
{
    clearTemplateSelection();

    kDebug(9010) << "templatesTreeViewActivated" << endl;
    const KDevAppTemplateModel *appModel = qobject_cast<const KDevAppTemplateModel*>( index.model() );
    KDevAppItem *item = reinterpret_cast<KDevAppItem*>( appModel->item( index ) );
    if( !item )
        return;
    KDevAppTemplate *templateItem = item->templateItem();
    if( !templateItem )
        return;

    m_currentTemplate = templateItem;
    if (!m_currentTemplate->iconName().isEmpty())
    {
        QString fn( m_currentTemplate->basePath() + '/' + m_currentTemplate->iconName() );
        //kDebug(9010) << "Loading icon: " << fn << endl;
        QPixmap pm;
        pm.load( fn );
        icon_label->setPixmap( pm );
    }
    desc_textview->setText(templateItem->whatsThis());
    m_projectLocationWasChanged = false;

    // Populate new custom options form
    //m_customOptions = new PropertyLib::PropertyEditor( custom_options );
    //m_customOptions->populateProperties(info->propValues);

    // Create new file template pages
    QStringList l = QStringList::split(",", templateItem->fileTemplates());
    QStringList::ConstIterator it = l.begin();
    QTextEdit *edit = 0;
    while (it != l.end())
    {
        AppWizardFileTemplate* fileTemplate = new AppWizardFileTemplate;
        fileTemplate->suffix = *it++;
        if( it != l.end() )
            fileTemplate->style = *it++;

        edit = new QTextEdit(this);
        edit->setFont(KGlobalSettings::fixedFont());
        fileTemplate->edit = edit;
        addPage(edit, i18n("Template for .%1 Files", fileTemplate->suffix));
        m_fileTemplates.append(fileTemplate);
    }

    // if the app template doesn't show file templates, we
    // need to set another m_lastPage, aleXXX
    m_lastPage = edit ? edit : 0 /*m_vcsForm*/;

    licenseChanged();
    updateNextButtons();
}

void AppWizardDialog::destButtonClicked(const QString& dir)
{
    // set new location as default project dir?
    if(dir.isEmpty())
        return;

    KConfig *config = KGlobal::config();
    config->setGroup("General Options");
    QDir defPrjDir( config->readPathEntry("DefaultProjectsDir", QDir::homePath()) );
    QDir newDir(dir);

    kDebug(9010) << "DevPrjDir == newdir?: " << defPrjDir.absPath() << " == " << newDir.absPath() << endl;
    if( defPrjDir == newDir )
        return;

    if( KMessageBox::questionYesNo(this, i18n("Set default project location to: %1?", newDir.absPath() ),
                                   i18n("New Project"), i18n("Set"), i18n("Do Not Set")) != KMessageBox::Yes )
        return;

    config->writePathEntry("DefaultProjectsDir", newDir.absPath() + "/");
    config->sync();
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
        if (!fi.exists() || appname_edit->displayText().isEmpty())
            finalLoc_label->setText(finalLoc_label->text() + i18nc("invalid location", " (invalid)"));
        else
            finalLoc_label->setText(finalLoc_label->text() + i18n(" (dir/file already exist)"));
        m_pathIsValid = false;
    } else
        m_pathIsValid = true;
    updateNextButtons();
}

void AppWizardDialog::openAfterGeneration()
{
    QString projectFile( finalLoc_label->text() + "/" + appname_edit->text().lower() + ".kdevelop" );

    // Read the DOM of the newly created project
    QFile file( projectFile );
    if( !file.open( QIODevice::ReadOnly ) )
        return;
    QDomDocument projectDOM;

    int errorLine, errorCol;
    QString errorMsg;
    bool success = projectDOM.setContent( &file, &errorMsg, &errorLine, &errorCol);
    file.close();
    if ( !success )
    {
        KMessageBox::sorry( 0, i18n("This is not a valid project file.\n"
                "XML error in line %1, column %2:\n%3",
                 errorLine, errorCol, errorMsg));
        return;
    }

    // DOM Modifications go here
    DomUtil::writeHashEntry( projectDOM, "substmap", m_currentTemplate->subMap() );

    /*
    //save the selected vcs
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/VCSIntegrator", QString("[X-KDevelop-VCS]=='%1'").arg(m_vcsForm->combo->currentText()));
    if (offers.count() == 1)
    {
        KService::Ptr service = offers.first();
        DomUtil::writeEntry(projectDOM, "/general/versioncontrol", service->property("X-KDevelop-VCSPlugin").toString());
    }
    */

    KConfig * config = KGlobal::config();
    config->setGroup("IgnorePerDefault");
    QStringList ignoreparts = config->readListEntry( "KDevelop" );
    DomUtil::writeListEntry( projectDOM, "/general/ignoreparts", "part", ignoreparts );


//FIXME PROFILES!!!!!!!!
//BEGIN Plugin Profile

/*  QString category = m_currentTemplate->category;
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
    if( !file.open( QIODevice::WriteOnly ) )
        return;
    QTextStream ts( &file );
    ts << projectDOM.toString(2);
    file.close();

    // open the new project
    m_part->core()->openProject( projectFile );

    // open files to open
    foreach( QString fileName, m_currentTemplate->openFilesAfterGeneration() )
    {
        if( !fileName.isNull() )
        {
            fileName = KMacroExpander::expandMacros(fileName, m_currentTemplate->subMap());
            m_part->documentController()->editDocument( fileName );
        }
    }
}

void AppWizardDialog::pageChanged()
{
    kDebug(9010) << "AppWizardDialog::pageChanged()" << endl;
    projectLocationChanged();   // Why?

/*
    //it is possible that project name was changed - we need to update all vcs integrator dialogs
    for (QMap<int, VCSDialog*>::iterator it = m_integratorDialogs.begin();
        it != m_integratorDialogs.end(); ++it)
        (*it)->init(getProjectName(), getProjectLocation());
*/
}

void AppWizardDialog::loadLicenses()
{
    KStandardDirs* dirs = KGlobal::dirs();
    dirs->addResourceType( "licenses", KStandardDirs::kde_default( "data" ) + "kdevelop/licenses/" );
    QStringList licNames = dirs->findAllResources( "licenses", QString::null, false, true );
    int idx = 0;

    foreach( QString fileName, licNames )
    {
        QString licPath( dirs->findResource( "licenses", fileName ) );
        kDebug(9010) << "Loading license file: " << licPath << endl;
        QString licName = licPath.mid( licPath.findRev('/') + 1 );
        KDevLicense* lic = new KDevLicense( licName, licPath );
        m_licenses.insert( licName, lic );
        license_combo->insertItem( licName, ++idx );
        if( licName == "GPL" )
            license_combo->setCurrentItem( idx );
    }
}

void AppWizardDialog::done(int r)
{
    // saveFavourites();
    QDialog::done(r);
}

void AppWizardDialog::showTemplates(bool /*all*/)
{
/*
    if (all)
    {
        Q3ListViewItemIterator it(templates_listview);
        while ( it.current() ) {
            it.current()->setVisible(true);
            ++it;
        }
    }
    else
    {
        Q3PtrListIterator<ApplicationInfo> ait(m_appsInfo);
        for (; ait.current(); ++ait)
        {
            ait.current()->item->setVisible(m_profileSupport->isInTemplateList(ait.current()->templateName));
        }

        Q3DictIterator<Q3ListViewItem> dit(m_categoryMap);
        for (; dit.current(); ++dit)
        {
            //checking whether all children are not visible
            kDebug(9010) << "check: " << dit.current()->text(0) << endl;
            bool visible = false;
            Q3ListViewItemIterator it(dit.current());
            while ( it.current() ) {
                if ((it.current()->childCount() == 0) && it.current()->isVisible())
                {
                    kDebug(9010) << "  visible: " << it.current()->text(0) << endl;
                    visible = true;
                    break;
                }
                ++it;
            }
            dit.current()->setVisible(visible);
        }
        checkAndHideItems(templates_listview);
    }
*/
}

//BEGIN unported
/*
void AppWizardDialog::addTemplateToFavourites()
{
    addFavourite(templates_listview->currentItem());
}

void AppWizardDialog::addFavourite(Q3ListViewItem* item, QString favouriteName)
{
    if(item->childCount())
        return;

    ApplicationInfo* info = templateForItem(item);

    if(!info->favourite)
    {
        info->favourite = new K3IconViewItem(favourites_iconview,
                                            ((favouriteName=="")?info->name:favouriteName),
                                            DesktopIcon("kdevelop"));

        info->favourite->setRenameEnabled(true);
    }
}

ApplicationInfo* AppWizardDialog::findFavouriteInfo(Q3IconViewItem* item)
{
    Q3PtrListIterator<ApplicationInfo> info(m_appsInfo);
    for (; info.current(); ++info)
        if (info.current()->favourite == item)
            return info.current();

    return 0;
}

void AppWizardDialog::favouritesIconViewClicked( Q3IconViewItem* item)
{
    ApplicationInfo* info = findFavouriteInfo(item);
    templatesTreeViewClicked(info->item);
}

void AppWizardDialog::removeFavourite()
{
    Q3IconViewItem* curFavourite = favourites_iconview->currentItem();

    //remove reference to favourite from associated appinfo
    Q3PtrListIterator<ApplicationInfo> info(m_appsInfo);
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
    favourites_iconview->sort();    //re-arrange all items.
}

void AppWizardDialog::populateFavourites()
{
    KConfig* config = KGlobal::config();
    config->setGroup("AppWizard");

    //favourites are stored in config as a list of templates and a seperate
    //list of icon names.
    QStringList templatesList = config->readPathListEntry("FavTemplates");
    QStringList iconNamesList = config->readListEntry("FavNames");

    QStringList::Iterator curTemplate = templatesList.begin();
    QStringList::Iterator curIconName = iconNamesList.begin();
    while(curTemplate != templatesList.end())
    {
        Q3PtrListIterator<ApplicationInfo> info(m_appsInfo);
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

void AppWizardDialog::saveFavourites()
{
    //need to save the template for each favourite and
    //it's icon name.  We have a one list for the templates
    //and one for the names.

    QStringList templatesList;
    QStringList iconNamesList;

    //Built the stringlists for each template that has a favourite.
    Q3PtrListIterator<ApplicationInfo> it(m_appsInfo);
    for (; it.current(); ++it)
    {
        if(it.current()->favourite)
        {
            templatesList.append(it.current()->templateName);
            iconNamesList.append(it.current()->favourite->text());
        }
    }

    KConfig* config = KGlobal::config();
    config->setGroup("AppWizard");
    config->writePathEntry("FavTemplates", templatesList);
    config->writeEntry("FavNames", iconNamesList);
    config->sync();
}

void AppWizardDialog::templatesContextMenu(Q3ListViewItem* item, const QPoint& point, int)
{
    if(item && !item->childCount())
        m_templatesMenu->popup(point);
}

void AppWizardDialog::favouritesContextMenu(Q3IconViewItem* item, const QPoint& point)
{
    if(item)
        m_favouritesMenu->popup(point);
}


void AppWizardDialog::checkAndHideItems(Q3ListView *view)
{
    Q3ListViewItem *item = view->firstChild();
    while (item)
    {
        if (!m_categoryItems.contains(item))
            continue;
        checkAndHideItems(item);
        item = item->nextSibling();
    }
}

bool AppWizardDialog::checkAndHideItems(Q3ListViewItem *item)
{
    if (!m_categoryItems.contains(item))
        return !item->isVisible();
    Q3ListViewItem *child = item->firstChild();
    bool hide = true;
    while (child)
    {
        hide = hide && checkAndHideItems(child);
        child = child->nextSibling();
    }
    kDebug(9010) << "check_: " << item->text(0) << " hide: " <<  hide << endl;
    if (hide)
    {
        item->setVisible(false);
        return true;
    }
    return false;
}
*/
//END unported
#include "appwizarddlg.moc"

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
