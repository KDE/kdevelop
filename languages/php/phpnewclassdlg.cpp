/***************************************************************************
                          phpnewclassdlg.cpp  -  description
                             -------------------
    begin                : Sat Aug 11 2001
    copyright            : (C) 2001 by Sandy Meier
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

#include "phpnewclassdlg.h"
#include <klineedit.h>
#include <kcompletion.h>
#include <kfiledialog.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <QTextStream>
#include <iostream>
#include <qregexp.h>
#include <q3textedit.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kdebug.h>

using namespace std;

PHPNewClassDlg::PHPNewClassDlg(const QStringList& baseClassNames,const QString& directory,QWidget *parent, const char *name) : PHPNewClassDlgBase(parent,name,true) {
  m_filenameModified = false;
  KCompletion *comp = new KCompletion();
  comp->setItems(baseClassNames);
  m_dirEdit->setText(directory);

  // load the class template if available
  QString templateFile = KGlobal::instance()->dirs()->findResource("data","kdevphpsupport/newclasstemplate.txt");
  if(!templateFile.isNull()){
    QFile file(templateFile);
    QTextStream stream(&file);
    if(file.open(QIODevice::ReadOnly)){ 
      m_classTemplate->setText(stream.read());
      file.close();
    }
  }
 
  
  m_baseClassEdit->setCompletionObject( comp ); /// @todo change it to KLineEdit
  connect(m_baseClassEdit,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&)));
  connect(m_classNameEdit,SIGNAL(textChanged(const QString&)),
	  this,SLOT(classNameTextChanged(const QString&)));
  connect(m_fileNameEdit,SIGNAL(textChanged(const QString&)),
	  this,SLOT(fileNameTextChanged(const QString&)));
  connect(m_dirButton,SIGNAL(clicked()),
	  this,SLOT(slotDirButtonClicked()));
}
PHPNewClassDlg::~PHPNewClassDlg(){
}

void PHPNewClassDlg::slotDirButtonClicked(){
  QString dir = KFileDialog::getExistingDirectory(m_dirEdit->text(),this);
  if(!dir.isEmpty()){
    m_dirEdit->setText(dir);
  }
}
void PHPNewClassDlg::classNameTextChanged(const QString& str){
  if(!m_filenameModified){
    m_fileNameEdit->setText(str.lower() + ".inc");  
  }
}

void PHPNewClassDlg::fileNameTextChanged(const QString&){
  if(m_fileNameEdit->hasFocus()){
    m_filenameModified = true;
  }
}
void PHPNewClassDlg::accept(){
  PHPNewClassDlgBase::accept(); // hide the dialog
 
  QString text = m_classTemplate->text();
  QString classDir = m_dirEdit->text();
  if(!classDir.endsWith("/")) classDir += "/"; // append /
  QString absFileName = classDir + m_fileNameEdit->text();
  
  // save the template for the next time
  QString templateDir = KGlobal::instance()->dirs()->saveLocation("data") + "/kdevphpsupport/";
  QString templateFile = templateDir + "newclasstemplate.txt";
  QDir dir(templateDir);
  if(!dir.exists()){
    if(!dir.mkdir(templateDir)){
      kdWarning() << "Error on creating directory for the classtemplate" << templateDir << endl;
    }
  }
  QFile file(templateFile); 
  QTextStream stream(&file);
  
  if(file.open(QIODevice::WriteOnly)){
    stream << text; // write
    file.close();
  }

  // generate the sourcecode for the class
  if(m_baseClassEdit->text().isEmpty()){
    text = text.replace(QRegExp("extends BASECLASS"),"");
    text = text.replace(QRegExp("BASECLASS\\:\\:BASECLASS\\(\\);"),"");
  }else{
    text = text.replace(QRegExp("BASECLASS"),m_baseClassEdit->text());
  }
  text = text.replace(QRegExp("CLASSNAME"),m_classNameEdit->text());
  text = text.replace(QRegExp("FILENAME"),m_fileNameEdit->text().upper());
  text = text.replace(QRegExp("AUTHOR"),"not implemented");
 
  file.setName(absFileName);
  if(file.open(QIODevice::WriteOnly)){
    stream << text; // write
    file.close();
  }
}

#include "phpnewclassdlg.moc"
