/***************************************************************************
                     cnewfiledlg.cpp - the new file dialog
                             -------------------                                         

    begin                : 17 Aug 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#define LAYOUT_BORDER (10)
#include "cnewfiledlg.h"

#include "cgeneratenewfile.h"
#include "cproject.h"

#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <ktabctl.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>

#include <iostream>
using namespace std;

CNewFileDlg::CNewFileDlg(CProject* p_prj, QWidget* parent,const char* name,bool modal,WFlags f)
  : QDialog(parent,name,modal,f){
  
  prj = p_prj;
  setCaption(i18n("New File..."));    

	QVBoxLayout * main_layout = new QVBoxLayout( this, LAYOUT_BORDER );
	QHBoxLayout * hlayout = new QHBoxLayout();
	main_layout->addLayout( hlayout, 1 );

	// the tabview
	tab = new KTabCtl(this);
  tab->setBorder(false);
	

  list_linux = new QListBox( tab, "list_linux" );
  list_linux->insertItem(i18n("lsm File - Linux Software Map"));
  list_linux->insertItem(i18n("kdelnk-File - for the KDE-Menu"));
  list_linux->insertItem(i18n("desktop-File - for the KDE-Menu"));
  list_linux->insertItem(i18n("Icon (*.png, *.xpm)"));
  list_linux->setMultiSelection( FALSE );
  list_linux->setCurrentItem(0);

  list_manuals = new QListBox( tab, "list_manuals" );
  list_manuals->insertItem(i18n("LinuxDoc SGML (english) (*.sgml)"));
  list_manuals->insertItem(i18n("DocBook SGML (english) (*.docbook)"));
  list_manuals->setMultiSelection( FALSE );
  list_manuals->setCurrentItem(0);

  list_cpp = new QListBox( tab, "list_cpp" );
  list_cpp->insertItem(i18n("C/C++ Header (*.h,*.hxx)"));
  list_cpp->insertItem(i18n("C/C++ File (*.cpp,*.c,*.cc,*.C,*.m,*.mm,*.M ...)"));
  list_cpp->insertItem(i18n("Empty Textfile"));
  list_cpp->insertItem(i18n("Qt Designer File (*.ui)"));
  list_cpp->insertItem(i18n("Lexical File (*.l, *.ll, *.lxx, *.l++)"));
  list_cpp->setMultiSelection( FALSE );
  list_cpp->setCurrentItem(0);
  QWhatsThis::add(list_cpp, i18n("Choose the type of the new file here."));
  QWhatsThis::add(list_manuals, i18n("Choose the type of the new file here."));
  QWhatsThis::add(list_linux,i18n("Choose the type of the new file here."));

  tab->addTab(list_cpp,i18n("General"));
  tab->addTab(list_manuals,i18n("Manuals"));
  tab->addTab(list_linux,i18n("Linux/KDE"));

	/* Had some problems with the sizeHint() below... so it's fixed :(
	   It seems though that the minimum size is later dynamically reassigned
	   anyway, the values below are used only for initial setup of window. */
	tab->setMinimumSize( 100, 200 );
	hlayout->addWidget( tab, 0 );

	QVBoxLayout * vlayout = new QVBoxLayout();
	hlayout->addLayout( vlayout, 1 );

	label_filename = new QLabel( this, "label_filename" );
	label_filename->setFocusPolicy( QWidget::NoFocus );
	label_filename->setBackgroundMode( QWidget::PaletteBackground );
	label_filename->setText( i18n("Filename:") );
	label_filename->setMinimumSize( label_filename->sizeHint() );
	vlayout->addWidget( label_filename, 0 );
	
	edit = new QLineEdit( this, "edit" );
	edit->setFocusPolicy( QWidget::StrongFocus );
	edit->setBackgroundMode( QWidget::PaletteBase );
	edit->setText( "" );
	edit->setMaxLength( 32767 );
	edit->setEchoMode( QLineEdit::Normal );
	edit->setFrame( TRUE );
    QWhatsThis::add(label_filename,i18n("Enter a name for your new file here."));
    QWhatsThis::add(edit,i18n("Enter a name for your new file here."));
    edit->setMinimumSize( edit->sizeHint() );
	vlayout->addWidget( edit, 0 );
	
	check_use_template = new QCheckBox( this, "check_use_template" );
	check_use_template->setFocusPolicy( QWidget::TabFocus );
	check_use_template->setBackgroundMode( QWidget::PaletteBackground );
	check_use_template->setText(i18n("Use template") );
	check_use_template->setAutoRepeat( FALSE );
	check_use_template->setAutoResize( FALSE );
	QWhatsThis::add(check_use_template,i18n("Check this if you want to use a template."));
	check_use_template->setMinimumSize( check_use_template->sizeHint() );
	vlayout->addWidget( check_use_template, 0 );

	vlayout->addStretch( 1 );
	
	button_group = new QButtonGroup( this, "button_group" );
	button_group->setFocusPolicy( QWidget::NoFocus );
	button_group->setBackgroundMode( QWidget::PaletteBackground );
	button_group->setFrameStyle( 49 );
	button_group->setTitle( i18n("Project Options" ));
	button_group->setAlignment( 1 );
	main_layout->addWidget( button_group, 0 );

	QGridLayout* glayout = new QGridLayout( button_group, 4, 2, LAYOUT_BORDER );
  glayout->setRowStretch( 0, 0 );
  glayout->setRowStretch( 1, 0 );
  glayout->setRowStretch( 2, 0 );
  glayout->setRowStretch( 3, 0 );
  glayout->setColStretch( 0, 1 );
  glayout->setColStretch( 1, 0 );

	glayout->addRowSpacing( 0, LAYOUT_BORDER );

	check_add_project = new QCheckBox( button_group, "check_add_project" );
	check_add_project->setFocusPolicy( QWidget::TabFocus );
	check_add_project->setBackgroundMode( QWidget::PaletteBackground );
	check_add_project->setText(i18n("Add to project") );
	check_add_project->setAutoRepeat( FALSE );
	check_add_project->setAutoResize( FALSE );
	QWhatsThis::add(check_add_project,i18n("Check this if you want to add the new file to your project."));
	check_add_project->setMinimumSize( check_add_project->sizeHint() );
	glayout->addWidget( check_add_project, 1, 0 );	
	
	location_label = new QLabel( button_group, "location_label" );
	location_label->setFocusPolicy( QWidget::NoFocus );
	location_label->setBackgroundMode( QWidget::PaletteBackground );
	location_label->setText(i18n("Location:") );
	location_label->setMinimumSize( location_label->sizeHint() );
	glayout->addWidget( location_label, 2, 0 );

	prj_loc_edit = new QLineEdit( button_group, "prj_loc_edit" );
	prj_loc_edit->setFocusPolicy( QWidget::StrongFocus );
	prj_loc_edit->setBackgroundMode( QWidget::PaletteBase );
	prj_loc_edit->setText( prj->getProjectDir()+ prj->getSubDir());
	prj_loc_edit->setMaxLength( 32767 );
	prj_loc_edit->setEchoMode( QLineEdit::Normal );
	prj_loc_edit->setFrame( TRUE );
	QWhatsThis::add(prj_loc_edit,i18n("Enter the directory where the new file will be located."));
	prj_loc_edit->setMinimumSize( prj_loc_edit->sizeHint() );
	glayout->addWidget( prj_loc_edit, 3, 0 );

	loc_button = new QPushButton( button_group, "loc_button" );
	loc_button->setFocusPolicy( QWidget::TabFocus );
	loc_button->setBackgroundMode( QWidget::PaletteBackground );
	QPixmap pix = SmallIcon("fileopen");
	loc_button->setPixmap(pix);
	loc_button->setAutoRepeat( FALSE );
	loc_button->setAutoResize( FALSE );
	QWhatsThis::add(loc_button,i18n("Here you can choose a directory where the new file will be located."));
	loc_button->setMinimumSize( loc_button->sizeHint().height(), loc_button->sizeHint().height() );
	glayout->addWidget( loc_button, 3, 1 );

	glayout->activate();

	button_group->insert( check_add_project );
	button_group->insert( loc_button );

	hlayout = new QHBoxLayout();
	main_layout->addLayout( hlayout, 0 );

	hlayout->addStretch( 1 );

	ok = new QPushButton( this, "ok" );
	ok->setFocusPolicy( QWidget::TabFocus );
	ok->setBackgroundMode( QWidget::PaletteBackground );
	ok->setText(i18n("&OK"));
	ok->setAutoRepeat( FALSE );
	ok->setAutoResize( FALSE );
	ok->setDefault( TRUE );
		
	cancel = new QPushButton( this, "cancel" );
	cancel->setFocusPolicy( QWidget::TabFocus );
	cancel->setBackgroundMode( QWidget::PaletteBackground );
	cancel->setText(i18n("&Cancel") );
	cancel->setAutoRepeat( FALSE );
	cancel->setAutoResize( FALSE );

	ok->setFixedSize( cancel->sizeHint() );
	cancel->setFixedSize( cancel->sizeHint() );

	hlayout->addWidget( ok, 0 );
	hlayout->addWidget( cancel, 0 );

	main_layout->activate();
	adjustSize();

  connect(tab,SIGNAL(tabSelected(int)),SLOT(slotTabSelected(int)));
  connect(ok,SIGNAL(clicked()),SLOT(slotOKClicked()));
  connect(cancel,SIGNAL(clicked()),SLOT(reject()));
  connect(loc_button,SIGNAL(clicked()),SLOT(slotLocButtonClicked()));
  connect(check_add_project,SIGNAL(clicked()),SLOT(slotAddToProject()));
  connect(edit,SIGNAL(textChanged (const QString&)),SLOT(slotEditTextChanged(const QString&)));

  connect(list_linux,SIGNAL(highlighted(int)),SLOT(slotListHighlighted(int)));
  connect(list_cpp,SIGNAL(highlighted(int)),SLOT(slotListHighlighted(int)));
  connect(list_manuals,SIGNAL(highlighted(int)),SLOT(slotListHighlighted(int)));
  //always default, add to Project
  check_add_project->setChecked(true);

  autocompletion = true;
}

void CNewFileDlg::slotTabSelected(int item){
  if(current == 0){ //cpp
    slotListHighlighted(list_cpp->currentItem());
  }
  if(current == 1){ // manuals
    slotListHighlighted(list_manuals->currentItem());
  }
  if(current == 2){ // linux
    slotListHighlighted(list_linux->currentItem());
  }
  current = item;
}
void CNewFileDlg::slotOKClicked(){
  QString text = edit->text();
  if ( (fileType() == "CPP") &&
       !(text.right(4) == ".cpp" || text.right(3) == ".cc" || text.right(2) == ".m" || text.right(3) == ".mm" || text.right(2) == ".M"
	|| text.right(2) == ".C" || text.right(2) == ".c" || text.right(4) == ".cxx" || text.right(3) == ".ec" || text.right(5) == ".ecpp" )){
    KMessageBox::error(this,i18n("The filename must end with .cpp,.c,.cc,.C,.cxx,.ec,.m or .ecpp!"));
    return;
  }
  if ( (fileType() == "HEADER") && !(text.right(2) == ".h" || (text.right(4) == ".hxx"))){
    KMessageBox::error(this,i18n("The filename must end with .h or .hxx!"));
    return;
  }
  if ( (fileType() == "LSM") && (text.right(4) != ".lsm")){
    KMessageBox::error(this,i18n("The filename must end with .lsm !"));
    return;
   }
  if ( (fileType() == "KDELNK") && (text.right(7) != ".kdelnk")){
    KMessageBox::error(this,i18n("The filename must end with .kdelnk !"));
    return;
  }
  if ( (fileType() == "DESKTOP") && (text.right(8) != ".desktop")){
    KMessageBox::error(this,i18n("The filename must end with .desktop !"));
    return;
  }
  if ( (fileType() == "EN_SGML") && (text.right(5) != ".sgml")){
    KMessageBox::error(this,i18n("The filename must end with .sgml !"));
    return;
  }
  if ( (fileType() == "EN_DOCBOOK") && (text.right(8) != ".docbook")){
    KMessageBox::error(this,i18n("The filename must end with .docbook !"));
    return;
  }
  if ( (fileType() == "QT2DIALOG") && (text.right(3) != ".ui")){
    KMessageBox::error(this,i18n("The filename must end with .ui !"));
    return;
  }
  if ( (fileType() == "ICON") && (text.right(4) != ".png" || text.right(4) != ".xpm")){
    KMessageBox::error(this,i18n("The filename must end with .png or .xpm !"));
    return;
  }
  if ( (fileType() == "LEXICAL") && !(text.right(4) == ".l++" || text.right(4) == ".lxx" || text.right(3) == ".ll" || text.right(2) == ".l")){
    KMessageBox::error(this,i18n("The filename must end with .l, .ll, .lxx or .l++ !"));
    return;
  }
  if (text.isEmpty()){
    KMessageBox::error(this,i18n("You must enter a filename!"));
    return;
  }
  if(addToProject() == true && (location().contains(prj->getProjectDir())) == 0 ){
    KMessageBox::error(this,i18n("You must choose a location,that is in your project-dir!"));
    return;
  }
  QString filename = fileName();
  QString complete_filename;
  // contruct the complete_filename
  complete_filename = location() + filename;
  
  if(QFile::exists(complete_filename)){
    if(KMessageBox::questionYesNo(0,
		      i18n("You have added a file to the project that already exists.\nDo you want overwrite the old one?"),
		      i18n("Files exists!")) == KMessageBox::No){
      return;
    }
  }

  /*************----------generate the new File----------****************/
  
  QString section;
  QString type;
  QString filetype = fileType(); 
  CGenerateNewFile generator;
 
   // check if generate a empty file or generate one
  if (useTemplate() && (filetype != "TEXTFILE") && filetype != "ICON"){ // generate one,textfile always empty
    if (filetype == "HEADER"){
      generator.genHeaderFile(complete_filename,prj);
      type = "HEADER";
    }
    if (filetype == "CPP"){
      generator.genCPPFile(complete_filename,prj);
      type = "SOURCE";
    }
    if (filetype == "LSM"){
      generator.genLSMFile(complete_filename,prj);
      type = "DATA";
    }
    if (filetype == "KDELNK"){
      generator.genKDELnkFile(complete_filename,prj);
      type = "DATA";
    }
    if (filetype == "DESKTOP"){
      generator.genDEsktopFile(complete_filename,prj);
      type = "DATA";
    }
    if (filetype == "EN_SGML"){
      generator.genEngHandbook(complete_filename,prj);
      type = "DATA";
    }
    if (filetype == "EN_DOCBOOK"){
      generator.genEngDocBook(complete_filename,prj);
      type = "DATA";
    }
    if (filetype == "LEXICAL"){
      generator.genLEXICALFile(complete_filename,prj);
      type = "SOURCE";
      // Make sure that we link against -lfl or -ll as controlled by
      // autoconf.
      if ((prj->getLDADD()).contains( "@LEXLIB@", FALSE ) == 0){
        prj->setLDADD( prj->getLDADD() + " @LEXLIB@ ");
      }
      KMessageBox::information(this,
            i18n("Please make sure, that you have added\n\"AM_LEX_PROG\" to your configure.in!"));
    }
  }
  else { // no template, -> empty file or icon
    if(filetype == "ICON"){
      generator.genIcon(complete_filename);
    }
    else{
      
      QFile file(complete_filename);
      file.remove();
      file.open(IO_ReadWrite);
      file.close();
    }
  }
  if( filetype == "QT2DIALOG"){
    QFile file(complete_filename);
    file.remove();
    file.open(IO_ReadWrite);
    file.close();
  }
  accept();
}

QString CNewFileDlg::fileName(){
  return edit->text();
}
QString CNewFileDlg::fileType(){ 
  QString str;
  if (current == 0){ // cpp/header
    str = list_cpp->text(list_cpp->currentItem());
    if (str == i18n("C/C++ File (*.cpp,*.c,*.cc,*.C, *.m ...)")){
      return "CPP";
    }
    if (str == i18n("C/C++ Header (*.h,*.hxx)")){
      return "HEADER";
    }
    if (str == i18n("Empty Textfile")){
      return "TEXTFILE";
    }
    if(str == i18n("Qt Designer File (*.ui)")){
      return "QT2DIALOG";
    }
    if(str == i18n("Lexical File (*.l, *.ll, *.lxx, *.l++)")){
      return "LEXICAL";
    }
  }
  
  
  if (current == 1){ // manuals
    str = list_manuals->text(list_manuals->currentItem());
    if (str == i18n("LinuxDoc SGML (english) (*.sgml)")){
      return "EN_SGML";
    }
    if (str == i18n("DocBook SGML (english) (*.docbook)")){
      return "EN_DOCBOOK";
    }
  }
  if (current == 2){ // /linux/kde
    str = list_linux->text(list_linux->currentItem());
    if (str == i18n("kdelnk-File - for the KDE-Menu")){
      return "KDELNK";
    }
    if (str == i18n("desktop-File - for the KDE-Menu")){
      return "DESKTOP";
    }
    if (str == i18n("lsm File - Linux Software Map")){
      return "LSM";
    }
    if (str == i18n("Icon (*.png, *.xpm)")){
      return "ICON";
    }
    
  }
  return "TEST";
}
bool CNewFileDlg::useTemplate(){
  return check_use_template->isChecked();
}
bool CNewFileDlg::addToProject(){
  return check_add_project->isChecked();
}
void CNewFileDlg::setUseTemplate(){
  check_use_template->setChecked(true);
}
void CNewFileDlg::setAddToProject(){
  check_add_project->setChecked(true);
}
void CNewFileDlg::slotLocButtonClicked(){
  QString str=  KFileDialog::getExistingDirectory(prj_loc_edit->text());
  if(!str.isEmpty()){
    prj_loc_edit->setText(str);
  }
}
QString CNewFileDlg::location(){
	QString str = prj_loc_edit->text();
	if (str.right(1) != "/"){
	 	str = str + "/";
	}
  return str;
}
void CNewFileDlg::slotAddToProject(){
}

void CNewFileDlg::slotEditTextChanged(const QString& text)
{
 if(text.isEmpty())
   return;
  QString filetype = fileType();
  QString extension = (prj && prj->getProjectType()=="normal_c" || prj->getProjectType()=="normal_gnome") ? ".c" : ((prj && prj->isObjcProject()) ? ".m" : ".cpp");

  if(autocompletion){

    if (filetype != "TEXTFILE" ) 
      {
	autocompletion = false;
	
	if (filetype == "HEADER" ) 
	  {
	    edit->setText(text + QString(".h"));
	  }
	if (filetype == "CPP" ) 
	  {
	    edit->setText(text + extension);
	  }
	if (filetype == "LEXICAL") 
	  {
	    edit->setText(text + QString(".l"));
	  }
	if (filetype == "KDELNK" ) 
	  {
	    edit->setText(text + QString(".kdelnk"));
	  }
	if (filetype == "DESKTOP" ) 
	  {
	    edit->setText(text + QString(".desktop"));
	  }
	if (filetype == "EN_SGML" ) 
	  {
	    edit->setText(text + QString(".sgml"));
	  }
	if (filetype == "EN_DOCBOOK" ) 
	  {
	    edit->setText(text + QString(".docbook"));
	  }
	if (filetype == "LSM" ) 
	  {
	    edit->setText(text + QString(".lsm"));
	  }
	if (filetype == "ICON" ) 
	  {
	    edit->setText(text + QString(".png"));
	  }
	if (filetype == "DIALOG" ) 
	  {
	    edit->setText(text + QString(".kdevdlg"));
	  }
	if (filetype == "QT2DIALOG" ) 
	  {
	    edit->setText(text + QString(".ui"));
	  }
	edit->setCursorPosition(1);
      }
    
  }
}
void CNewFileDlg::slotListHighlighted(int){
  edit->clear();
  edit->setFocus();
  autocompletion = true;
}


/** sets the location for the dialog to start from */
void CNewFileDlg::setLocation(QString location){
	prj_loc_edit->setText(location);
}
#include "cnewfiledlg.moc"
