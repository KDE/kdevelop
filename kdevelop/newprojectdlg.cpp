/***************************************************************************
                          newprojectdlg.cpp  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kiconloader.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qiconview.h>
#include <qstringlist.h>
#include <iostream.h>
#include <kstddirs.h>
#include <klibloader.h>
#include <qfileinfo.h>
#include <projectspace.h>
#include <kfiledialog.h>
#include <qdir.h>
#include <qtoolbutton.h>
#include <qtextview.h>
#include <ktrader.h>
#include <qvariant.h>
#include <qheader.h>
#include <kmessagebox.h>
#include "newprojectdlg.h"
#include "appwizard.h"
#include "ctoolclass.h"
#include "KDevComponentManager.h"
#include "kdevelopcore.h"


NewProjectDlg::NewProjectDlg(KDevelopCore* kdc, ProjectSpace* projectSpace,QWidget *parent, const char *name,bool modal )
  : NewProjectDlgBase(parent,name,modal){
  
  m_current_appwizard_plugin=0; // no current
  m_project_name_modified = false;
  m_project_location_modified=false;
  m_projectspace_location_modified=false;
  m_default_location = QDir::homeDirPath() + "/testprj";
  m_pixmap = new QPixmap();
  m_pProjectSpace = projectSpace;
  m_newProjectSpace = true;
  m_pSelectedProjectSpace =0;
  m_pKDevelopCore = kdc;


  //  QStringList::Iterator it;		
  KStandardDirs* std_dirs = KGlobal::dirs();
  QStringList plugin_list;
  ProjectSpace* space;
  AppWizard* appwizard_plg;
  
  m_prjspace_list = new QList<ProjectSpace>;
  m_appwizard_list = new QList<AppWizard>;
  initDialog();

  // all offers for ProjectSpace
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/ProjectSpace");
  if (offers.isEmpty()){
    kdDebug(9000) << "No KDevelop projectspace found" << endl;
  }
  KTrader::OfferList::ConstIterator offer_it;
  QString name;
  QString lang;
  QStringList lang_list;
  QVariant prop;
  // colltect all languages, (toplevel folders)
  for (offer_it = offers.begin(); offer_it != offers.end(); ++offer_it) {
    kdDebug(9000) << "Found Projectspace " << (*offer_it)->name() << endl;
    prop = (*offer_it)->property("X-KDevelop-Language");
    if (prop.isValid()){
      lang = prop.toString();
      lang_list.append(lang);
      kdDebug(9000) << "added Language " << lang << endl;
    }
  }
  QStringList::Iterator lang_it;
  QListViewItem* lang_item;
  QListViewItem* tmp_item;
  for (lang_it = lang_list.begin(); lang_it != lang_list.end(); ++lang_it) { // for every lang
    // new lang
    lang_item = new QListViewItem( prjspace_listview,*lang_it);
    lang_item->setPixmap(0, KGlobal::iconLoader()->loadIcon("folder", 
							    KIcon::Desktop, KIcon::SizeSmall));
    lang_item->setOpen(true);

    for (offer_it = offers.begin(); offer_it != offers.end(); ++offer_it) {
      prop = (*offer_it)->property("X-KDevelop-Language");
      if(prop.toString() == *lang_it){
	prop = (*offer_it)->property("Name");
	// new projectspace
	name = prop.toString();
	tmp_item =  new QListViewItem( lang_item,name);
	tmp_item->setPixmap(0, KGlobal::iconLoader()->loadIcon((*offer_it)->icon(), 
							    KIcon::Desktop, KIcon::SizeSmall));
	kdDebug(9000) << "Name " << name << endl;
      }
    }
  }
  
  connect(prjspace_listview,SIGNAL(clicked(QListViewItem*)),
	  SLOT(slotProjectSpaceSelected(QListViewItem*)));
  connect(appwizard_iconview,SIGNAL(selectionChanged (QIconViewItem*)),
	  SLOT(slotAppwizardSelected (QIconViewItem*) ) );

  connect(m_prjspace_dir_button,SIGNAL(clicked()),SLOT(slotProjectSpaceDirClicked()));	 		  
  connect(m_prjspace_name_linedit, SIGNAL(textChanged(const QString &)),
          SLOT(slotProjectSpaceNameEdit(const QString &)));
  connect(m_prjspace_location_linedit, SIGNAL(textChanged(const QString &)),
          SLOT(slotProjectSpaceLocationEdit(const QString &)));

  connect(m_project_dir_button,SIGNAL(clicked()),SLOT(slotProjectDirClicked()));
  connect(m_prjname_linedit, SIGNAL(textChanged(const QString &)),
          SLOT(slotProjectNameEdit(const QString &)));
  connect(m_prjlocation_linedit, SIGNAL(textChanged(const QString &)),
          SLOT(slotProjectLocationEdit(const QString &)));
  
  // init the first one
  //  prjspace_listbox->setSelected(0,true);
  resize(637,489);
  
}
NewProjectDlg::~NewProjectDlg(){
  delete m_pixmap;
}
void NewProjectDlg::slotProjectSpaceLocationEdit(const QString &){
  if(m_prjspace_location_linedit->hasFocus()){
    m_projectspace_location_modified = true;
  }
}
void NewProjectDlg::slotProjectNameEdit(const QString &){
  if(m_prjname_linedit->hasFocus()){
    m_project_name_modified = true;
  }
  if(!m_project_location_modified){
    m_prjlocation_linedit->setText(m_default_location + "/" + m_prjname_linedit->text().lower());
  } 
}
void NewProjectDlg::slotProjectLocationEdit(const QString &){
  if(m_prjlocation_linedit->hasFocus()){
    m_project_location_modified = true;
  }
}
void NewProjectDlg::slotProjectSpaceSelected (QListViewItem *item){
	
	if(! item) return; // not a valid item? do nothing!
	
  if(item->depth() != 1){
    kdDebug(9000) << item->depth() << endl;
    return; // no Projectspace selected
  }
  m_current_prjspace_name = item->text(0);
  QString constraint = QString("[X-KDevelop-ProjectSpace-Name] == '%1'").arg(m_current_prjspace_name);
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/AppWizard", constraint);
    if (offers.isEmpty()) {
        KMessageBox::sorry(this,
                           i18n("No Applicationwizards for %1 found!").arg(m_current_prjspace_name));
        return;
    }

    KTrader::OfferList::ConstIterator offer_it;
    appwizard_iconview->clear();
    QVariant prop;
    QString name;
    for (offer_it = offers.begin(); offer_it != offers.end(); ++offer_it) {
      prop = (*offer_it)->property("Name");
	name = prop.toString();
	new QIconViewItem(appwizard_iconview,name,KGlobal::iconLoader()->loadIcon((*offer_it)->icon(), 
							    KIcon::Desktop));
      }
}

void NewProjectDlg::slotAppwizardSelected (QIconViewItem* item){
  AppWizard* appwizard_plg;
  QString constraint = QString("([Name] == '%1') and [X-KDevelop-ProjectSpace-Name] == '%2'").arg(item->text()).arg(m_current_prjspace_name);

  KTrader::OfferList offers = KTrader::self()->query("KDevelop/AppWizard",constraint );
  KService *service = *offers.begin();
  kdDebug(9000) << "Found Appwizard Component " << service->name() << endl;
  
  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  if(m_current_appwizard_plugin != 0){
    delete m_current_appwizard_plugin; // just remove the current one
  }
  QObject *obj = factory->create(this, service->name().latin1(),
				 "AppWizard");
        
  if (!obj->inherits("AppWizard")) {
    kdDebug(9000) << "Component does not inherit AppWizard" << endl;
    return;
  }
  
  appwizard_plg = (AppWizard*) obj; 
  m_current_appwizard_plugin = appwizard_plg; // set current plugin
  description_textview->setText(appwizard_plg->applicationDescription());
  m_pixmap->load(appwizard_plg->previewPicture());
  preview_widget->setPixmap(*m_pixmap);
  kdDebug(9000) << "Picture: " << appwizard_plg->previewPicture() << endl;
  ok_button->setEnabled(true);
}

void NewProjectDlg::slotProjectSpaceNameEdit(const QString &text){
  QString str = text;
  if(!m_project_name_modified){
    m_prjname_linedit->setText(str);
  }
  if(!m_project_location_modified){
    m_prjlocation_linedit->setText(m_default_location + "/" + str.lower() + "/" + str.lower());
  }
  if(!m_projectspace_location_modified){
    m_prjspace_location_linedit->setText(m_default_location + "/" + str.lower());
  }
  
}
void NewProjectDlg::slotOk(){
  QString relProjectPath;
  if(current_radio_button->isChecked()){
    m_newProjectSpace = false;
  }
  if (m_newProjectSpace) {
		m_pKDevelopCore->closeProjectSpace();
    (void) m_pKDevelopCore->m_pKDevComponentManager->loadByName(this, m_current_prjspace_name, "ProjectSpace");
    m_pSelectedProjectSpace = static_cast<ProjectSpace*>(m_pKDevelopCore->m_pKDevComponentManager->component("ProjectSpace"));
    if(m_pSelectedProjectSpace == 0){
      KMessageBox::sorry(0, i18n("Sorry, can't create ProjectSpace with type %1")
		       .arg(m_current_prjspace_name));
      return;
    }
    
    // name, path
    m_pSelectedProjectSpace->setAbsolutePath(m_prjspace_location_linedit->text());
    m_pSelectedProjectSpace->setName(m_prjspace_name_linedit->text());
    m_current_appwizard_plugin->init(true,m_pSelectedProjectSpace,m_prjname_linedit->text(),
				     m_prjlocation_linedit->text());
  }
  else { // use current ProjectSpace
    m_current_appwizard_plugin->init(false,m_pProjectSpace,m_prjname_linedit->text(),
				     m_prjlocation_linedit->text());
  }
  m_current_appwizard_plugin->exec(); // exec the dialog
  kdDebug(9000) << "ApplicationWizard exited" << endl;
  accept();
}
void NewProjectDlg::slotProjectSpaceDirClicked(){
  QString dir = KFileDialog::getExistingDirectory ( m_prjspace_location_linedit->text(),this,
						    "Projectspace Location" );
  if(!dir.isEmpty()){
    m_prjspace_location_linedit->setText(dir);
    m_projectspace_location_modified = true;
  }
 
}
void NewProjectDlg::slotProjectDirClicked(){
  QString dir = KFileDialog::getExistingDirectory ( m_prjspace_location_linedit->text(),this,
						    "Projectspace Location" );
  if(!dir.isEmpty()){
    m_prjlocation_linedit->setText(dir);
    m_project_location_modified = true;
  }
}


void NewProjectDlg::initDialog(){
  prjspace_listview->header()->hide();
  if(m_pProjectSpace == 0){
    slotNewProjectSpaceClicked();
    current_radio_button->setEnabled(false);
    new_radio_button->setChecked(true);
  }
  else{
    slotAddToCurrentProjectSpaceClicked();
    current_radio_button->setChecked(true);
    m_prjspace_location_linedit->setText(m_pProjectSpace->absolutePath());
    m_prjspace_name_linedit->setText(m_pProjectSpace->name());
  }
  ok_button->setEnabled(false);

}
void NewProjectDlg::slotAddToCurrentProjectSpaceClicked(){
  m_prjspace_name_linedit->setEnabled(false);
  m_prjspace_dir_button->setEnabled(false);
  m_prjspace_location_linedit->setEnabled(false);
}
void NewProjectDlg::slotNewProjectSpaceClicked(){
  m_prjspace_name_linedit->setEnabled(true);
  m_prjspace_dir_button->setEnabled(true);
  m_prjspace_location_linedit->setEnabled(true);
}
bool NewProjectDlg::newProjectSpaceCreated(){
  return m_newProjectSpace;
}
QString NewProjectDlg::projectSpaceFile() {
  if(m_pSelectedProjectSpace != 0){
    return m_pSelectedProjectSpace->projectSpaceFile();
  }
  return "";
}




#include "newprojectdlg.moc"
