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
#include <qfiledialog.h>
#include <qdir.h>
#include <qtoolbutton.h>
#include <qtextview.h>
#include <ktrader.h>
#include <qvariant.h>
#include <qheader.h>
#include <kmessagebox.h>
#include "plugin.h"
#include "newprojectdlg.h"
#include "appwizard.h"
#include "pluginloader.h"
#include "ctoolclass.h"



NewProjectDlg::NewProjectDlg(QWidget *parent, const char *name,bool modal )
  : NewProjectDlgBase(parent,name,modal){
  
  m_current_appwizard_plugin=0; // no current
  m_project_name_modified = false;
  m_project_location_modified=false;
  m_projectspace_location_modified=false;
  m_default_location = QDir::homeDirPath() + "/testprj";
  m_pixmap = new QPixmap();
  
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
  connect(m_prjname_linedit, SIGNAL(textChanged(const QString &)),
          SLOT(slotProjectNameEdit(const QString &)));
  connect(m_prjlocation_linedit, SIGNAL(textChanged(const QString &)),
          SLOT(slotProjectLocationEdit(const QString &)));
  
  // init the first one
  //  prjspace_listbox->setSelected(0,true);
  new_radio_button->setChecked(true);
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
}
void NewProjectDlg::slotProjectLocationEdit(const QString &){
  if(m_prjlocation_linedit->hasFocus()){
    m_project_location_modified = true;
  }
}
void NewProjectDlg::slotProjectSpaceSelected (QListViewItem *item){
  if(item->depth() != 1){
    cerr << item->depth();
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
  description_textview->setText(appwizard_plg->getPluginDescription());
  m_pixmap->load(appwizard_plg->getPreviewPicture());
  preview_widget->setPixmap(*m_pixmap);
  cerr << "Picture:" << appwizard_plg->getPreviewPicture() << endl;
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
  bool new_projectspace = true;
  ProjectSpace* selected_prjspace;
  if(current_radio_button->isChecked()) new_projectspace = false;
  
  
  QString constraint = QString("[Name] == '%1'").arg(m_current_prjspace_name);
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/ProjectSpace", constraint);
  KService *service = *offers.begin();
  kdDebug(9000) << "Found ProjectSpace Component " << service->name() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  QObject *obj = factory->create(this, service->name().latin1(),
				 "ProjectSpace");
        
  if (!obj->inherits("ProjectSpace")) {
    kdDebug(9000) << "Component does not inherit ProjectSpace" << endl;
    return;
  }

  selected_prjspace = (ProjectSpace*) obj;
    
  // name, path
  selected_prjspace->setAbsolutePath(m_prjspace_location_linedit->text());
  selected_prjspace->setName(m_prjspace_name_linedit->text());
  
  m_current_appwizard_plugin->init(new_projectspace,selected_prjspace,m_prjname_linedit->text());
  Project* prj = m_current_appwizard_plugin->getProject();
  kdDebug(9000) << "NewProjectDlg: set some project props" << endl;
    
  prj->setName(m_prjname_linedit->text());
  QString relProjectPath = CToolClass::getRelativePath(m_prjspace_location_linedit->text(),
							m_prjlocation_linedit->text());
  prj->setRelativePath(relProjectPath);
  prj->setAbsolutePath(m_prjlocation_linedit->text());
  kdDebug(9000) << "relative ProjectPath:" <<relProjectPath  << endl;
  
  m_current_appwizard_plugin->exec(); // exec the dialog
  kdDebug(9000) << "ApplicationWizard exited" << endl;
  
  accept();
}
void NewProjectDlg::slotProjectSpaceDirClicked(){
  QString dir = QFileDialog::getExistingDirectory ( m_prjspace_location_linedit->text(),this,"dirdlg",
						    "Projectspace Location" );
  m_prjspace_location_linedit->setText(dir);
}


void NewProjectDlg::initDialog(){
  prjspace_listview->header()->hide();
}




#include "newprojectdlg.moc"
