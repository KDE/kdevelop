/***************************************************************************
                          newprojectdlg.cpp  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.de
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

#include "plugin.h"
#include "newprojectdlg.h"
#include "appwizard.h"
#include "pluginloader.h"

// from the JanusWidget

class IconListItem : public QListBoxItem
{
  public:
    IconListItem( QListBox *listbox, const QPixmap &pixmap,
		   const QString &text );
    virtual int height( const QListBox *lb ) const;
    virtual int width( const QListBox *lb ) const;
    int expandMinimumWidth( int width );

  protected:
    const QPixmap &defaultPixmap();
    void paint( QPainter *painter );

  private:
    QPixmap mPixmap;
    int mMinimumWidth;
};


NewProjectDlg::NewProjectDlg(QWidget *parent, const char *name,bool modal )
  : KDialogBase(Plain,"jk",Ok|Cancel,Ok,parent,name,modal){
  
  m_current_appwizard_plugin=0; // no current
  m_project_name_modified = false;
  m_project_location_modified=false;
  m_projectspace_location_modified=false;
  m_default_location = QDir::homeDirPath() + "/testprj";
  m_pixmap = new QPixmap();
  
  QStringList::Iterator it;		
  KStandardDirs* std_dirs = KGlobal::dirs();
  QStringList plugin_list;
  ProjectSpace* space;
  AppWizard* appwizard_plg;
  
  m_prjspace_list = new QList<ProjectSpace>;
  m_appwizard_list = new QList<AppWizard>;
  initDialog();
  
  m_prjspace_list = PluginLoader::getAllProjectSpaces(this);
  m_appwizard_list = PluginLoader::getAllAppWizards(this);
  
  // test output
  for(space = m_prjspace_list->first();space != 0;space = m_prjspace_list->next()){
    cerr << endl << "Name:" << space->getPluginName() << endl;
    cerr << "Version:" << space->getPluginVersion() << endl;
    cerr << "Copyright:" << space->getPluginCopyright() << endl;
  }
  
				// test output
  for(appwizard_plg = m_appwizard_list->first();appwizard_plg != 0;
      appwizard_plg = m_appwizard_list->next()){
    
    cerr << endl << "Name:" << appwizard_plg->getPluginName() << endl;
    cerr << "Version:" << appwizard_plg->getPluginVersion() << endl;
    cerr << "Copyright:" << appwizard_plg->getPluginCopyright() << endl;
    cerr << "Picture:" << appwizard_plg->getPreviewPicture() << endl;
  }
  
  cerr << "PI" << std_dirs->findResource("appdata","appwizard_pics/normalApp.bmp");
  plugin_list = std_dirs->allTypes();
  for(it = plugin_list.begin();it != plugin_list.end();it++){
    cerr << "RE:" << *it << endl;
				}
  // fill the project_space list
  for(space = m_prjspace_list->first();space != 0;space = m_prjspace_list->next()){
    new IconListItem(prjspace_listbox,space->getPluginIcon(),space->getPluginName());	 	
  }
  
  connect(prjspace_listbox,SIGNAL(highlighted( const QString & )),
	  SLOT(slotProjectSpaceSelected  ( const QString & )));
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
  prjspace_listbox->setSelected(0,true);
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
void NewProjectDlg::slotProjectSpaceSelected ( const QString& name){
  AppWizard* appwizard_plg;
  appwizard_iconview->clear();
  for(appwizard_plg = m_appwizard_list->first();appwizard_plg != 0;
      appwizard_plg = m_appwizard_list->next()){
    // add all related appwizard plugins
    if (appwizard_plg->getProjectSpaceName() == name){
      new QIconViewItem(appwizard_iconview,appwizard_plg->getPluginName(),
			appwizard_plg->getPluginIcon());
    }
  }
}

void NewProjectDlg::slotAppwizardSelected (QIconViewItem* item){
  AppWizard* appwizard_plg;
  
  for(appwizard_plg = m_appwizard_list->first();appwizard_plg != 0;
      appwizard_plg = m_appwizard_list->next()){
    // find the selected appwizard plugins
    if (appwizard_plg->getPluginName() == item->text()){
      m_current_appwizard_plugin = appwizard_plg; // set current plugin
      description_label->setText(appwizard_plg->getPluginDescription());
      m_pixmap->load(appwizard_plg->getPreviewPicture());
      preview_widget->setBackgroundPixmap(*m_pixmap);
      cerr << "Picture:" << appwizard_plg->getPreviewPicture() << endl;
    }
  }
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
  ProjectSpace* prjspace;
  ProjectSpace* selected_prjspace;
  QString name = prjspace_listbox->currentText();
  
  
  if(current_radio_button->isChecked()) new_projectspace = false;
  
  for(prjspace = m_prjspace_list->first();prjspace != 0;prjspace = m_prjspace_list->next()){
    if(prjspace->getPluginName() == name){
      selected_prjspace = prjspace;
    }
  }
  // name, path
  selected_prjspace->setAbsolutePath(m_prjspace_location_linedit->text());
  selected_prjspace->setName(m_prjspace_name_linedit->text());
  
  
  m_current_appwizard_plugin->init(new_projectspace,selected_prjspace);
  Project* prj = m_current_appwizard_plugin->getProject();
  //  prj->setName(m_prjname_linedit->text());
  //  prj->setAbsolutePath(m_prjlocation_linedit->text());
  
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

  prjspace_listbox= new QListBox(this,"NoName");
  prjspace_listbox->setGeometry(10,10,110,460);
  prjspace_listbox->setMinimumSize(0,0);
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


IconListItem::IconListItem( QListBox *listbox, const QPixmap &pixmap,
			    const QString &text )
  : QListBoxItem( listbox )
{
  mPixmap = pixmap;
  setText( text );
  mMinimumWidth = 0;
}


void IconListItem::paint( QPainter *painter )
{
  QFontMetrics fm = painter->fontMetrics();
  int wt = fm.boundingRect(text()).width();
  int wp = mPixmap.width();
  int ht = fm.lineSpacing();
  int hp = mPixmap.height();

  painter->drawPixmap( wp, 5, mPixmap );
  if( text().isEmpty() == false )
  {
    painter->save();
    QFont f( painter->font() );
    f.setBold( true );
    painter->setFont(f);
    painter->drawText(wt, hp+ht+5, text() );
    painter->restore();
  }
}

