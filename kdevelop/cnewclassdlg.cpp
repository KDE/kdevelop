/***************************************************************************
                     cnewclassdlg.cpp - 
                             -------------------                                         

    version              :                                   
    begin                : 4 Oct 1998                                        
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

#include "cnewclassdlg.h"
#include <kquickhelp.h>

// SETUP DIALOG
CNewClassDlg::CNewClassDlg( QWidget *parent, const char *name,CProject* prj )
    : QDialog( parent, name,TRUE )
{
  prj_info = prj;
  header_modified =false;
  impl_modified = false;
  setCaption(i18n("Class Generator" ));
 


  classname_group = new QButtonGroup( this, "classname_group" );
  classname_group->setGeometry( 15, 10, 230, 60 );
  classname_group->setTitle(i18n("Classname") );
	
  classname_edit = new QLineEdit( this, "classname_edit" );
  classname_edit->setGeometry( 25, 30, 210, 30 );
	KQuickHelp::add(classname_group,
	KQuickHelp::add(classname_edit, i18n("Insert your new classname here.\n"
	                                    "Please try to keep some standards\n"
	                                    "like using an upper `K` as the first\n"
	                                    "letter for your new class, if it is\n"
	                                    "a class for a KDE-project.")));


  baseclass_group = new QButtonGroup( this, "baseclass_group" );
  baseclass_group->setGeometry( 260, 10, 230, 60 );
  baseclass_group->setTitle(i18n( "Baseclass") );
  baseclass_group->setAlignment( 1 );

  baseclass_edit = new QLineEdit( this, "baseclass_edit" );
  baseclass_edit->setGeometry( 275, 30, 205, 30 );
  KQuickHelp::add(baseclass_edit, i18n("Insert the baseclass from which your new class\n"
                                    "will be derived from. If you have checked\n"
                                    "`generate a QWidget-Childclass`, the new class will\n"
                                    "be derived from QWidget. If no baseclass is named, the\n"
                                    "new class will not have a parent class."));

  files_group = new QButtonGroup( this, "files_group" );
  files_group->setGeometry( 15, 80, 230, 120 );
  files_group->setTitle( i18n("Files") );
  files_group->setAlignment( 1 );
  

  header_label = new QLabel( this, "header_label" );
  header_label->setGeometry( 25, 100, 50, 30 );
  header_label->setText( i18n("Header") );
	
  header_edit = new QLineEdit( this, "header_edit" );
  header_edit->setGeometry( 85, 100, 150, 30 );

  KQuickHelp::add(header_label,
  KQuickHelp::add(header_edit, i18n("Insert your header filename here.\n"
                                    "It is automatically inserted while\n"
                                    "you select the classname, but you can\n"
                                    "still edit it afterwards.")));

  impl_label = new QLabel( this, "impl_label" );
  impl_label->setGeometry( 25, 155, 60, 30 );
  impl_label->setText( i18n("Impl.") );

  impl_edit = new QLineEdit( this, "impl_edit" );
  impl_edit->setGeometry( 85, 155, 150, 30 );

  KQuickHelp::add(impl_label,
  KQuickHelp::add(impl_edit, i18n("Insert your implementation filename here.\n"
                                  "It is automatically inserted while\n"
                                  "you select the classname, but you can\n"
                                  "still edit it afterwards.")));

  doc_group = new QButtonGroup( this, "doc_group" );
  doc_group->setGeometry( 15, 210, 230, 120 );
  doc_group->setTitle( i18n("Documentation") );


  doc_edit = new QMultiLineEdit( this, "doc_edit" );
  doc_edit->setGeometry( 25, 230, 210, 90 );
  KQuickHelp::add(doc_group,
  KQuickHelp::add(doc_edit, i18n("Insert a short description for your new class\n"
                                "here for documenation purposes. This will be used\n"
                                "for the API-documentation you can create or update\n"
                                "after the class is created.")));
	
  inher_group = new QButtonGroup( this, "inher_group" );
  inher_group->setGeometry( 260, 80, 230, 120 );
  inher_group->setTitle( i18n("Inheritence") );


  public_button = new QRadioButton( this, "public_button" );
  public_button->setGeometry( 275, 100, 100, 30 );
  public_button->setText( i18n("public") );
  public_button->setChecked(true);
  
  protected_button = new QRadioButton( this, "protected_button" );
  protected_button->setGeometry( 275, 130, 90, 30 );
  protected_button->setText( i18n("protected") );
  
  private_button = new QRadioButton( this, "private_button" );
  private_button->setGeometry( 275, 160, 100, 30 );
  private_button->setText( i18n("private") );
	

  add_group = new QButtonGroup( this, "add_group" );
  add_group->setGeometry( 260, 210, 230, 120 );
  add_group->setTitle(i18n("Additional Options") );

  template_check = new QCheckBox( this, "template_check" );
  template_check->setGeometry( 275, 240, 185, 30 );
  template_check->setText( i18n("use header/cpp-templates") );
  template_check->setChecked(true);
  
  qwidget_check = new QCheckBox( this, "qwidget_check" );
  qwidget_check->setGeometry( 275, 280, 210, 30 );
  qwidget_check->setText( i18n("generate a QWidget-Childclass") );
  
  add_group->insert( template_check );
  add_group->insert( qwidget_check );
  KQuickHelp::add(add_group,
  KQuickHelp::add(template_check,
  KQuickHelp::add(qwidget_check, i18n("This offers additional options to your new class.\n"
                                    "If you check `use header/cpp-templates`, your name and\n"
                                    "email adress will be added into the new generated files\n"
                                    "If you check `generate a QWidget-Childclass`, the new class\n"
                                    "will use QWidget as the baseclass. This is also useful for\n"
                                    "using baseclasses derived from QWidget, e.g. QDialog."))));


  inher_group->insert( public_button );
  inher_group->insert( protected_button );
  inher_group->insert( private_button );

  KQuickHelp::add(inher_group,
  KQuickHelp::add(protected_button,
  KQuickHelp::add(public_button,
  KQuickHelp::add(private_button, i18n("Check here your inheritance from the chosen baseclass.\n"
                                      "You can derive from a baseclass as public,protected\n"
                                      "or private.")))));



  ok_button = new QPushButton( this, "ok_button" );
  ok_button->setGeometry( 130, 340, 100, 30 );
  ok_button->setText("OK");
	ok_button->setDefault( TRUE );

  cancel_button = new QPushButton( this, "cancel_button" );
  cancel_button->setGeometry( 275, 340, 100, 30 );
  cancel_button->setText(i18n("Cancel")); 
  setFixedSize( 505,380 );

  connect(classname_edit,SIGNAL(textChanged(const char*)),SLOT(slotClassEditChanged(const char*)));
  connect(header_edit,SIGNAL(textChanged(const char*)),SLOT(slotHeaderEditChanged(const char*)));
  connect(impl_edit,SIGNAL(textChanged(const char*)),SLOT(slotImplEditChanged(const char*)));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
  connect(ok_button,SIGNAL(clicked()),SLOT(ok()));
  
  //setOkButton(i18n("OK"));
  // setCancelButton(i18n("Cancel"));
  //connect( this, SIGNAL(applyButtonPressed()), SLOT(ok()) );
}



  

void CNewClassDlg::ok(){
  QString text = classname_edit->text(); 
  if (text.isEmpty() ){
    KMsgBox::message(this,i18n("Error..."),i18n("You must enter a classname!"),KMsgBox::EXCLAMATION);
    return;
   }
  text = header_edit->text(); 
  if (text.isEmpty() ){
    KMsgBox::message(this,i18n("Error..."),i18n("You must enter a name for the header-file!"),KMsgBox::EXCLAMATION);
    return;
   }
  text = impl_edit->text(); 
  if (text.isEmpty() ){
    KMsgBox::message(this,i18n("Error..."),i18n("You must enter a name for the implementation-file!")
		     ,KMsgBox::EXCLAMATION);
    return;
  }
  // text = baseclass_edit->text(); 
//   if (text.isEmpty() && qwidget_check->isChecked()){
//     KMsgBox::message(this,i18n("Error..."),i18n("You must enter a name for the baseclass!")
// 		     ,KMsgBox::EXCLAMATION);
//     return;
//   }

 
  QString classname = classname_edit->text();
  QString headername = header_edit->text();
  QString implname = impl_edit->text();
  QString basename = baseclass_edit->text();
  if(basename.isEmpty() && qwidget_check->isChecked()){
    basename = "QWidget";
  }
  QString doc = doc_edit->text();
  

  CGenerateNewFile generator;
  // generate the sourcecode
  
  if(template_check->isChecked()){
    headername = prj_info->getProjectDir() + prj_info->getSubDir() + headername;
    implname = prj_info->getProjectDir() + prj_info->getSubDir() + implname;
    headername = generator.genHeaderFile(headername,prj_info);
    implname = generator.genCPPFile(implname,prj_info);
  }
  else{
    headername = prj_info->getProjectDir() + prj_info->getSubDir() + headername;
    implname = prj_info->getProjectDir() + prj_info->getSubDir() + implname;
    QFile file(headername);
    file.open(IO_ReadWrite);
    file.close();
    file.setName(implname);
    file.open(IO_ReadWrite);
    file.close();
  }
  // modify the header
  QStrList list;
  QFile file(headername);
  QTextStream stream(&file);
  QString str;
  
  if(file.open(IO_ReadOnly)){ // 
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str != 0;str = list.next()){
      stream << str << "\n";
    }
    stream << "\n#ifndef " + classname.upper() + "_H\n";
    stream << "#define "+ classname.upper() + "_H\n\n";
    if (qwidget_check->isChecked() && basename != "QWidget"){
      stream << "#include <qwidget.h>\n";
    }
    if(!basename.isEmpty()){
      stream << "#include <" + basename.lower() + ".h>\n";
    }
    stream << "\n/**" + doc + "\n";
    stream << "  *@author "+ prj_info->getAuthor() + "\n";
    stream << "  */\n\n";
    stream << "class " + classname;
    if(!basename.isEmpty()){
      stream << " : ";
      if(public_button->isChecked()){
	stream << "public ";
      }
      if(protected_button->isChecked()){
	stream << "protected ";
      }
      if(private_button->isChecked()){
	stream << "private ";
      }
      stream << basename + " ";
    }
    stream << " {\n";
    if(qwidget_check->isChecked()){
      stream << "   Q_OBJECT\n";
    }
    stream << "public: \n";
    stream << "\t" + classname+"(";
    if (qwidget_check->isChecked()){
      stream << "QWidget *parent=0, const char *name=0";
    }
    stream << ");\n";
    stream << "\t~" + classname +"();\n";
    stream << "};\n\n#endif\n";

      
  }
  file.close();

  // modify the implementation
  list.clear();
  file.setName(implname);
  if(file.open(IO_ReadOnly)){ // 
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str != 0;str = list.next()){
      stream << str << "\n";
    }
    stream << "\n#include \"" + QString(header_edit->text()) + "\"\n\n" ;
    stream << classname + "::" + classname +"(" ;
    if (qwidget_check->isChecked()){
      stream << "QWidget *parent, const char *name ) : ";
      stream << basename + "(parent,name) {\n";
    }
    else{
      stream << "){\n";
    }
    stream << "}\n";
    stream << classname + "::~" + classname +"(){\n";
    stream << "}\n";
  }
  file.close();

  m_header_file = headername;
  m_impl_file = implname;
  accept();
}
QString CNewClassDlg::getHeaderFile(){
  return m_header_file;
}
QString CNewClassDlg::getImplFile(){
  return m_impl_file;
}

void CNewClassDlg::slotClassEditChanged(const char* text){
  QString str = text;
  if(!header_modified){
    header_edit->setText(str.lower() + ".h");
  }
  if(!impl_modified){
    impl_edit->setText(str.lower() +".cpp");
  }
  
}
void CNewClassDlg::slotHeaderEditChanged(const char*){
  if(header_edit->hasFocus()){
    header_modified = true;
  }
}
void CNewClassDlg::slotImplEditChanged(const char*){
  if(impl_edit->hasFocus()){
    impl_modified = true;
  }
}


