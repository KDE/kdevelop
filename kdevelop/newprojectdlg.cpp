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
#include <ktrader.h>
#include <qvariant.h>
#include <qheader.h>
#include <kmessagebox.h>
#include "plugin.h"
#include "newprojectdlg.h"
#include "appwizard.h"
#include "pluginloader.h"



NewProjectDlg::NewProjectDlg(QWidget *parent, const char *name,bool modal )
  : KDialogBase(Plain,"jk",Ok|Cancel,Ok,parent,name,modal){
  
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
  
  connect(prjspace_listview,SIGNAL(executed(QListViewItem*)),
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
  description_label->setText(appwizard_plg->getPluginDescription());
  m_pixmap->load(appwizard_plg->getPreviewPicture());
  preview_widget->setBackgroundPixmap(*m_pixmap);
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
  
  m_current_appwizard_plugin->init(new_projectspace,selected_prjspace);
  Project* prj = m_current_appwizard_plugin->getProject();
  kdDebug(9000) << "NewProjectDlg: set some project props" << endl;
    
  prj->setName(m_prjname_linedit->text());
  prj->setAbsolutePath(m_prjlocation_linedit->text());
  
  m_current_appwizard_plugin->exec(); // exec the dialog
  
  accept();
}
void NewProjectDlg::slotProjectSpaceDirClicked(){
  QString dir = QFileDialog::getExistingDirectory ( m_prjspace_location_linedit->text(),this,"dirdlg",
						    "Projectspace Location" );
  m_prjspace_location_linedit->setText(dir);
}


void NewProjectDlg::initDialog(){
  appwizard_iconview= new KIconView(this,"NoName");
  appwizard_iconview->setGeometry(130,10,270,200);
  appwizard_iconview->setMinimumSize(0,0);
  
  m_seperator= new KSeparator(this,"NoName");
  m_seperator->setGeometry(130,210,490,30);
  m_seperator->setMinimumSize(0,0);
  m_seperator->setOrientation(KSeparator::HLine);
  
  m_prjspace_location_linedit= new QLineEdit(this,"NoName");
  m_prjspace_location_linedit->setGeometry(290,340,290,30);
  m_prjspace_location_linedit->setMinimumSize(0,0);

  m_prjspace_dir_button= new QPushButton(this,"NoName");
  m_prjspace_dir_button->setGeometry(590,340,40,30);
  m_prjspace_dir_button->setMinimumSize(0,0);
  m_prjspace_dir_button->setText(i18n("..."));

  button_group= new QButtonGroup(this,"NoName");
  button_group->setGeometry(130,240,490,50);
  button_group->setMinimumSize(0,0);

  current_radio_button= new QRadioButton(button_group,"NoName");
  current_radio_button->setGeometry(250,10,190,30);
  current_radio_button->setMinimumSize(0,0);
  current_radio_button->setText(i18n("add to current Projectspace"));

  new_radio_button= new QRadioButton(button_group,"NoName");
  new_radio_button->setGeometry(10,10,190,30);
  new_radio_button->setMinimumSize(0,0);
  new_radio_button->setText(i18n("create new Projectspace"));

  preview_widget= new QWidget(this,"NoName");
  preview_widget->setGeometry(410,10,210,140);
  preview_widget->setMinimumSize(0,0);
  description_label= new QLabel(this,"NoName");
  description_label->setGeometry(410,150,210,60);
  description_label->setMinimumSize(0,0);
  description_label->setText(i18n("Description"));
  description_label->setMargin(0);

  prjspace_listview= new KListView(this,"NoName");
  prjspace_listview->setGeometry(10,10,110,460);
  prjspace_listview->setMinimumSize(0,0);
  prjspace_listview->addColumn("");
  prjspace_listview->header()->hide();
  prjspace_listview->setRootIsDecorated(true);
  m_prjspace_name_linedit= new QLineEdit(this,"NoName");
  m_prjspace_name_linedit->setGeometry(290,300,200,30);
  m_prjspace_name_linedit->setMinimumSize(0,0);

  m_prjspace_location_label= new QLabel(this,"NoName");
  m_prjspace_location_label->setGeometry(130,340,160,30);
  m_prjspace_location_label->setMinimumSize(0,0);
  m_prjspace_location_label->setText(i18n("Projectspace Location:"));

  m_prjspace_name_label= new QLabel(this,"ProjecNoName");
  m_prjspace_name_label->setGeometry(130,300,150,30);
  m_prjspace_name_label->setMinimumSize(0,0);
  m_prjspace_name_label->setText(i18n("Projectspace Name:"));

  m_prj_name_label= new QLabel(this,"NoName");
  m_prj_name_label->setGeometry(130,380,160,30);
  m_prj_name_label->setMinimumSize(0,0);
  m_prj_name_label->setText(i18n("Project Name"));

  m_prjname_linedit= new QLineEdit(this,"NoName");
  m_prjname_linedit->setGeometry(290,380,200,30);
  m_prjname_linedit->setMinimumSize(0,0);

  m_prj_location_label= new QLabel(this,"NoName");
  m_prj_location_label->setGeometry(130,420,160,30);
  m_prj_location_label->setMinimumSize(0,0);
  m_prj_location_label->setText(i18n("Project Location"));

  m_prjlocation_linedit= new QLineEdit(this,"NoName");
  m_prjlocation_linedit->setGeometry(290,420,290,30);
  m_prjlocation_linedit->setMinimumSize(0,0);

  m_project_dir_button= new QPushButton(this,"NoName");
  m_project_dir_button->setGeometry(590,420,40,30);
  m_project_dir_button->setMinimumSize(0,0);
  m_project_dir_button->setText(i18n("..."));
}




#include "newprojectdlg.moc"
