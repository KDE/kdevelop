/***************************************************************************
                          ctoolsconfigdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Apr 15 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlined.h>
#include <qfileinfo.h>

#include <kfiledialog.h>
#include <kmessagebox.h>
#include <qwhatsthis.h>

#include "ctoolsconfigdlg.h"
#include "ctoolclass.h"

#include <kstddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <qgrid.h>
#include <qlayout.h>

#include <kdebug.h>
#include <iostream>
using namespace std;

CToolsConfigDlg::CToolsConfigDlg(QWidget *parent, const char *name ) : QDialog(parent,name,this) {
	setCaption(i18n("Tools-Menu Configuration"));

  QGridLayout *grid1 = new QGridLayout(this,8,3,15,7);
  tools_listbox = new QListBox( this, "tools_listbox" );
  grid1->addMultiCellWidget(tools_listbox,0,3,0,1);
  connect( tools_listbox, SIGNAL(highlighted(int)), SLOT(slotShowToolProp(int)) );
  tools_listbox->setFrameStyle( 51 );
  tools_listbox->setLineWidth( 2 );
  readConfig();

  add_button = new QPushButton( this, "add_button" );
  connect( add_button, SIGNAL(clicked()), SLOT(slotToolAdd()) );
  add_button->setText(i18n( "&Add" ));
  add_button->setAutoRepeat( FALSE );
  add_button->setAutoResize( FALSE );
  QWhatsThis::add(add_button,i18n("Click here to add a tool specified below."));
  grid1->addWidget(add_button,0,2);

  delete_button = new QPushButton( this, "delete_button" );
  connect( delete_button, SIGNAL(clicked()), SLOT(slotToolDelete()) );
  delete_button->setText(i18n( "De&lete" ));
  delete_button->setAutoRepeat( FALSE );
  delete_button->setAutoResize( FALSE );
  delete_button->setEnabled( FALSE );
  QWhatsThis::add(delete_button,i18n("Click here to delete an entry from the list."));
  grid1->addWidget(delete_button,1,2);

  move_up_button = new QPushButton( this, "move_up_button" );
  connect( move_up_button, SIGNAL(clicked()), SLOT(slotToolMoveUp()) );
  move_up_button->setText(i18n( "Move &Up" ));
  move_up_button->setAutoRepeat( FALSE );
  move_up_button->setAutoResize( FALSE );
  move_up_button->setEnabled( FALSE );
  QWhatsThis::add(move_up_button,i18n("Click here to move up the selected entry."));
  grid1->addWidget(move_up_button,2,2);

  move_down_button = new QPushButton( this, "move_down_button" );
  connect( move_down_button, SIGNAL(clicked()), SLOT(slotToolMoveDown()) );
  move_down_button->setText(i18n( "Move &Down" ));
  move_down_button->setAutoRepeat( FALSE );
  move_down_button->setAutoResize( FALSE );
  move_down_button->setEnabled( FALSE );
  QWhatsThis::add(move_down_button,i18n("Click here to move down the selected entry."));
  grid1->addWidget(move_down_button,3,2);

  executable_label = new QLabel( this, "command_label" );
  executable_label->setText(i18n( "Executable:" ));
  executable_label->setAlignment( 289 );
  executable_label->setMargin( -1 );
  grid1->addWidget(executable_label,4,0);

  executable_edit = new QLineEdit( this, "LineEdit_1" );
  executable_edit->setText( "" );
  executable_edit->setMaxLength( 32767 );
  executable_edit->setEchoMode( QLineEdit::Normal );
  executable_edit->setFrame( TRUE );
  grid1->addWidget(executable_edit,4,1);

  executable_button = new QPushButton( this, "executable_button" );
  QWhatsThis::add(executable_button,i18n("Here you can browse through the disc to select an executable file."));
  connect( executable_button, SIGNAL(clicked()), SLOT(slotToolsExeSelect()) );
  QPixmap pix = SmallIcon("fileopen");
  executable_button->setPixmap(pix);
  executable_button->setAutoRepeat( FALSE );
  executable_button->setAutoResize( FALSE );
  QString executable_help = i18n("Enter the name of the executable file here.");
  QWhatsThis::add(executable_edit,executable_help);
  QWhatsThis::add(executable_button,executable_help);
  grid1->addWidget(executable_button,4,2);

  menu_text_label = new QLabel( this, "Label_3" );
  menu_text_label->setText(i18n( "Menu Text:" ));
  menu_text_label->setAlignment( 289 );
  menu_text_label->setMargin( -1 );
  grid1->addWidget(menu_text_label,5,0);

  menu_text_edit = new QLineEdit( this, "LineEdit_2" );
  menu_text_edit->setText( "" );
  menu_text_edit->setMaxLength( 32767 );
  menu_text_edit->setEchoMode( QLineEdit::Normal );
  menu_text_edit->setFrame( TRUE );
  QString menu_help=i18n("Enter a menu text for the executable here.");
  QWhatsThis::add(menu_text_label, menu_help);
  QWhatsThis::add(menu_text_edit,menu_help);
  grid1->addWidget(menu_text_edit,5,1);

  arguments_label = new QLabel( this, "Label_4" );
  arguments_label->setText(i18n( "Arguments:" ));
  arguments_label->setAlignment( 289 );
  arguments_label->setMargin( -1 );
  grid1->addWidget(arguments_label,6,0);

  arguments_edit = new QLineEdit( this, "LineEdit_3" );
  arguments_edit->setText( "" );
  arguments_edit->setMaxLength( 32767 );
  arguments_edit->setEchoMode( QLineEdit::Normal );
  arguments_edit->setFrame( TRUE );
  QString arguments_help = i18n("Enter arguments for the executable here.\n"
                                "Use %S for current open file and %D for the project directory");
  QWhatsThis::add(arguments_label, arguments_help);
  QWhatsThis::add(arguments_edit,arguments_help);
  grid1->addWidget(arguments_edit,6,1);

  QHBoxLayout* hbox = new QHBoxLayout(grid1, 5);
  output_check = new QCheckBox( this, "output_check" );
  output_check->setText( i18n("Capture output") );
  QWhatsThis::add( output_check, i18n("Display the output (STDOUT, STDERR) of this Tool in the message pane") );
  hbox->addWidget(output_check);
  grid1->addMultiCellLayout(hbox, 7, 7, 0, 1);

  ok_button = new QPushButton( this, "ok_button" );
  connect( ok_button, SIGNAL(clicked()), SLOT(slotOK()) );
  ok_button->setText(i18n( "OK" ));
  ok_button->setAutoRepeat( FALSE );
  ok_button->setAutoResize( FALSE );
  ok_button->setDefault( TRUE );
  grid1->addWidget(ok_button,0,3);

  cancel_button = new QPushButton( this, "cancel_button" );
  connect( cancel_button, SIGNAL(clicked()), SLOT(reject()) );
  cancel_button->setText(i18n( "Cancel" ));
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  grid1->addWidget(cancel_button,1,3);


  help_button = new QPushButton( this, "PushButton_7" );
  connect( help_button, SIGNAL(clicked()), SLOT(slotHelp()) );
  help_button->setText(i18n( "&Help" ));
  help_button->setAutoRepeat( FALSE );
  help_button->setAutoResize( FALSE );
  grid1->addWidget(help_button,2,3);

  resize( 510, 380 );
}

CToolsConfigDlg::~CToolsConfigDlg(){
}



void CToolsConfigDlg::slotToolAdd()
{
  QString executable=executable_edit->text();
  QString menutext=menu_text_edit->text();
  QString exe_file=QFileInfo(executable).fileName();

  if(executable.isEmpty()){
  	KMessageBox::error(this, i18n("You have to set an executable to add to the Tools-Menu first."),i18n("Executable not Set!"));
  	return;
  }
  if(menutext.isEmpty()){
    KMessageBox::error(this, i18n("You have to insert a menuentry text to add the selected program to the Tools-Menu."),i18n("Menu Text not Set!"));
    return;
  }

  if(!QFileInfo(CToolClass::findProgram(executable)).isExecutable()){
    KMessageBox::sorry(this,i18n("The selected file is not an executable. Please choose an executable filename."));
    return;
  }

//  if(!CToolClass::searchInstProgram(exe_file) ){
//    KMessageBox::sorry(this,i18n("The selected executable is not in your Path. Please update your $PATH environment variable to execute the selected program as a tool."));
//    return;
//  }

  tools_listbox->insertItem(menutext);
  toolList.append(CToolApp(menutext, executable, arguments_edit->text(), output_check->isChecked()));		
}

void CToolsConfigDlg::slotToolDelete()
{
  int current=tools_listbox->currentItem();
  if (current == -1)
    return;
  toolList.remove(toolList.at(current));
  tools_listbox->removeItem(current);

}

void CToolsConfigDlg::slotToolMoveUp()
{
  int current = tools_listbox->currentItem();
  if (current == 0  || current == -1) return ;

  swap(current-1,current);
  tools_listbox->clear();
  ToolAppList::Iterator it;
  for (it = toolList.begin(); it != toolList.end(); ++it)
    tools_listbox->insertItem( (*it).getLabel() );
  tools_listbox->setCurrentItem(current-1);
  if (current - 1 == 0)
    move_up_button->setEnabled(false);
  move_down_button->setEnabled(true);
}

void CToolsConfigDlg::slotToolMoveDown()
{
  uint current = tools_listbox->currentItem();
  // if (current == (int)tools_listbox->count()-1 || current == -1) return;
  swap(current,current+1);
  tools_listbox->clear();
  ToolAppList::Iterator it;
  for (it = toolList.begin(); it != toolList.end(); ++it)
    tools_listbox->insertItem( (*it).getLabel() );
  tools_listbox->setCurrentItem(current+1);
  if (current + 1 == tools_listbox->count() - 1) {
    move_down_button->setEnabled(false);
  }
  move_up_button->setEnabled(true);
}


void CToolsConfigDlg::slotToolsExeSelect()
{
  QString exe_file_name;
  exe_file_name = KFileDialog::getOpenFileName();
  if (!exe_file_name.isEmpty()){
    QString exe_file=QFileInfo(exe_file_name).fileName();
    if(!QFileInfo(exe_file_name).isExecutable()){
      KMessageBox::sorry(this,i18n("The selected file is not an executable. Please choose an executable filename."));
      return;
    }
    else if(!CToolClass::searchInstProgram(exe_file) ){
      KMessageBox::sorry(this,i18n("The selected executable is not in your Path. Please update your $PATH environment variable"
						 "to execute the selected program as a tool."));
      return;
    }
    else{
      executable_edit->setText(exe_file);
    }
  }
}

void CToolsConfigDlg::slotShowToolProp(int index){

  //listbox is empty if index < 0
  if ( index < 0 ) {
    executable_edit->clear();
    menu_text_edit->clear();
    arguments_edit->clear();
    output_check->setChecked( FALSE );
    delete_button->setEnabled( FALSE );
    move_up_button->setEnabled( FALSE );
    move_down_button->setEnabled( FALSE );
    return;
  }

  CToolApp toolApp = toolList[index];

  executable_edit->setText( toolApp.getExeName() );
  menu_text_edit->setText( toolApp.getLabel() );
  arguments_edit->setText( toolApp.getArgs() );
  output_check->setChecked( toolApp.isOutputCaptured() );

//  kdDebug() << toolApp.getExeName() << " " << toolApp.isOutputCaptured() << endl;

  delete_button->setEnabled( TRUE );

  if(index != 0){
    move_up_button->setEnabled( TRUE );
  }
  else{
    move_up_button->setEnabled( FALSE );
  }
  if(index+1 != (int)tools_listbox->count()){
    move_down_button->setEnabled( TRUE );
  }
  else{
    move_down_button->setEnabled( FALSE );
  }
}

void CToolsConfigDlg::readConfig(){
  
  CToolClass::readToolConfig(toolList);
  ToolAppList::Iterator it;
  for (it = toolList.begin(); it != toolList.end(); ++it)
    tools_listbox->insertItem( (*it).getLabel() );
}

void CToolsConfigDlg::writeConfig(){
  CToolClass::writeToolConfig(toolList);
}

void CToolsConfigDlg::slotOK()
{
  writeConfig();
  close();
}

void CToolsConfigDlg::slotHelp()
{
  kapp->invokeHTMLHelp("kdevelop/index-12.html", "ss12.1" );
}

void CToolsConfigDlg::swap(int item1,int item2)
{
  
  CToolApp t1, t2;
  t1 = toolList[item1];
  t2 = toolList[item2];
  toolList.remove(toolList.at(item1));
  toolList.insert(toolList.at(item1), t2);
  toolList.remove(toolList.at(item2));
  toolList.insert(toolList.at(item2),t1);
  
}

#include "ctoolsconfigdlg.moc"
