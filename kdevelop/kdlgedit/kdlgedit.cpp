/***************************************************************************
                          kdlgedit.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include "kdlgedit.h"
#include "../ckdevelop.h"
#include "kdlgeditwidget.h"
#include "kdlgitems.h"
#include "kdlgpropwidget.h"
#include "kdlgotherdlgs.h"
#include "kdlgnewdialogdlg.h"
#include "kdlgdialogs.h"
#include "../cgeneratenewfile.h"
#include "../cproject.h"
#include <kfiledialog.h>
#include <qstring.h>
#define DONTINC_ALL
#define INC_WIDGET
#include "items.h"
#include <kconfig.h>

KDlgEdit::KDlgEdit(QObject *parentz, const char *name) : QObject(parentz,name)
{
   connect(((CKDevelop*)parent())->kdlg_get_dialogs_view(),SIGNAL(kdlgdialogsSelected(QString)),
	   SLOT(slotOpenDialog(QString)));
   connect(((CKDevelop*)parent())->kdlg_get_dialogs_view(),SIGNAL(newDialog()),
	   SLOT(slotFileNew()));
}

KDlgEdit::~KDlgEdit()
{
}


void KDlgEdit::slotFileNew(){
  CProject* prj = ((CKDevelop*)parent())->getProject(); 
  TDialogFileInfo info;
  QString temp_dialog_file = dialog_file;
  if(prj != 0){
    if(slotFileClose()){
      KDlgNewDialogDlg dlg(((QWidget*) parent()),"I",prj);
      if( dlg.exec()){
	// get the location
	QString location = dlg.getLocation();
	if(location.right(1) != "/"){
	  location = location + "/";
	}
	info.rel_name = prj->getSubDir() + dlg.getClassname().lower() + ".kdevdlg";
	info.dist = true;
	info.install = false;
	info.classname = dlg.getClassname();
	info.baseclass = dlg.getBaseClass();
	info.header_file = getRelativeName(location + dlg.getHeaderName());
	info.source_file = getRelativeName(location + dlg.getSourceName());
	info.data_file = getRelativeName(location + dlg.getDataName());
	info.is_toplevel_dialog = true;
	
	QString l_dialog_file = prj->getProjectDir() + info.rel_name;
	
	if(prj->addDialogFileToProject(info.rel_name,info)){
	  ((CKDevelop*)parent())->newSubDir();
	}
	((CKDevelop*)parent())->kdlg_get_edit_widget()->newDialog();			
	((CKDevelop*)parent())->kdlg_get_edit_widget()->saveToFile(l_dialog_file);
	
	// registrate the source files
	((CKDevelop*)parent())->slotAddFileToProject(location + dlg.getHeaderName());
	((CKDevelop*)parent())->slotAddFileToProject(location + dlg.getSourceName());
	((CKDevelop*)parent())->slotAddFileToProject(location + dlg.getDataName());
	
	// generate the new files;
	// header
	generateInitialHeaderFile(info,dlg.getBaseClassHeader());
	generateInitialSourceFile(info);
	slotOpenDialog(l_dialog_file);
	slotBuildGenerate();
	((CKDevelop*)parent())->refreshTrees();
      }
      else{
	if(temp_dialog_file != ""){
	  slotOpenDialog(temp_dialog_file);
	}
      }
    } // end if(slotFileClose()
  }
}

void KDlgEdit::slotFileOpen()
{
  QString  name = KFileDialog::getOpenFileName("","*.kdevdlg",0);
  if (name.isNull()){
    return;
  }
  else{
		slotOpenDialog(name);
	}
}
void KDlgEdit::slotOpenDialog(QString file){
  if(file != dialog_file){
    if(slotFileClose()){
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->show();
      ((CKDevelop*)parent())->kdlg_get_prop_widget()->show();
      ((CKDevelop*)parent())->kdlg_get_tabctl()->setTabEnabled("widgets_view",true);
      ((CKDevelop*)parent())->kdlg_get_tabctl()->setTabEnabled("items_view",true);
      ((CKDevelop*)parent())->enableCommand(ID_KDLG_BUILD_GENERATE);
      ((CKDevelop*)parent())->enableCommand(ID_KDLG_FILE_CLOSE);
      ((CKDevelop*)parent())->enableCommand(ID_KDLG_FILE_OPEN);
      ((CKDevelop*)parent())->enableCommand(ID_KDLG_FILE_SAVE);
      ((CKDevelop*)parent())->enableCommand(ID_KDLG_FILE_SAVE_AS);
      
      dialog_file = file;
      
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->openFromFile(file);
      ((CKDevelop*)parent())->setCaption(i18n("KDevelop Dialog Editor: ")+file); 
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->mainWidget()->getProps()->setProp_Value("VarName","this");
    }
  }
}

bool KDlgEdit::slotFileClose(){
  int result = 1;
  if(((CKDevelop*)parent())->kdlg_get_edit_widget()->isModified()){
    result = KMsgBox::yesNoCancel(0,i18n("Question"),i18n("You have modified the current dialog. Do you want to save\nthe dialog and generate the sourcecodes?"));
  }
  if(result == 3) return false; // cancel
  if(result == 1) { // ok
    slotFileSave();
  }
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->hide();
  ((CKDevelop*)parent())->kdlg_get_prop_widget()->hide();
  ((CKDevelop*)parent())->kdlg_get_tabctl()->setTabEnabled("widgets_view",false);
  ((CKDevelop*)parent())->kdlg_get_tabctl()->setTabEnabled("items_view",false);
  ((CKDevelop*)parent())->disableCommand(ID_KDLG_BUILD_GENERATE);
  ((CKDevelop*)parent())->disableCommand(ID_KDLG_FILE_CLOSE);
  ((CKDevelop*)parent())->disableCommand(ID_KDLG_FILE_OPEN);
  ((CKDevelop*)parent())->disableCommand(ID_KDLG_FILE_SAVE);
  ((CKDevelop*)parent())->disableCommand(ID_KDLG_FILE_SAVE_AS);
  dialog_file = "";
  return true;
}

void KDlgEdit::slotFileSave(){
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->saveToFile(dialog_file);
}
void KDlgEdit::slotFileSaveAs(){
  QString  name = KFileDialog::getSaveFileName("","*.kdevdlg",0);
  if (name.isNull()){
    return;
  }
  else{
    ((CKDevelop*)parent())->kdlg_get_edit_widget()->saveToFile(name);
  }
}
	
void KDlgEdit::slotEditUndo(){
}

void KDlgEdit::slotEditRedo()
{
}

void KDlgEdit::slotEditCut(){
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_cutSelected();
}

void KDlgEdit::slotEditDelete(){
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_deleteSelected();
}

void KDlgEdit::slotEditCopy(){
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_copySelected();
}

void KDlgEdit::slotEditPaste(){
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->slot_pasteSelected();
}

void KDlgEdit::slotEditProperties()
{
} 	

void KDlgEdit::slotViewRefresh()
{
  ((CKDevelop*)parent())->kdlg_get_items_view()->refreshList();

  KDlgItem_Base* sel = ((CKDevelop*)parent())->kdlg_get_edit_widget()->selectedWidget();

  if (sel)
    ((CKDevelop*)parent())->kdlg_get_prop_widget()->refillList(sel);

  ((CKDevelop*)parent())->kdlg_get_edit_widget()->mainWidget()->recreateItem();
}

void KDlgEdit::slotBuildGenerate(){
  CProject* prj = ((CKDevelop*)parent())->getProject(); 
  TDialogFileInfo info = prj->getDialogFileInfo(getRelativeName(dialog_file));

  ///////////////////////////// datafile ///////////////////////////////////////////
	////////first generate the datafile and then the header for the datafile//////////

  if( ((CKDevelop*)parent())->isFileInBuffer(prj->getProjectDir() + info.data_file)){
      ((CKDevelop*)parent())->switchToFile(prj->getProjectDir() + info.data_file);
      ((CKDevelop*)parent())->slotFileSave();
      ((CKDevelop*)parent())->slotFileClose();
  }
  
  QFile file(prj->getProjectDir() + info.data_file);
  QTextStream stream( &file );
  if ( file.open(IO_WriteOnly) ){
    QFileInfo header_file_info(prj->getProjectDir()+info.header_file);
    stream << "#include \"" << header_file_info.fileName() << "\"\n\n";
    
    stream << "void  " << info.classname + "::initDialog(){\n";
    ((CKDevelop*)parent())->kdlg_get_edit_widget()->mainWidget()->getProps()->setProp_Value("VarName","this");
    variables.clear();
    includes.clear();
    local_includes.clear();
    if(((CKDevelop*)parent())->getProject()->isKDEProject()){
      local_includes.append("#include <kapp.h>");
    }
    generateWidget(((CKDevelop*)parent())->kdlg_get_edit_widget()->mainWidget(),&stream,"this");
    stream << "}\n";
  }
  file.close();

  
  // add the header to the datafile (ok, not the best sourcecode, but it works :-)
  // read the file
 
  QStrList file_lines;
  QString str;

  if (file.open(IO_ReadOnly)){
    while(!stream.eof()){
      file_lines.append(stream.readLine());
    }
  }
  file.close();
  // generate the complete datafile
  if ( file.open(IO_WriteOnly) ){
    
    stream << "/**********************************************************************\n";
    stream << "            --- KDevelop (KDlgEdit)  generated file ---                \n\n";
    stream << "            Last generated: " << QDateTime::currentDateTime().toString() << "\n\n";
    stream << "            DO NOT EDIT!!!  This file will be automatically\n";
    stream << "            regenerated by KDevelop.  All changes will be lost.\n\n";
    stream << "**********************************************************************/\n";

    // local_includes
    for(str = local_includes.first();str != 0;str = local_includes.next()){
      stream << str << "\n";
    }
    for(str = file_lines.first();str != 0;str = file_lines.next()){
      stream << str << "\n";
    }
  }

  ///////////////////////////headerfile////////////////////////
  
  if(((CKDevelop*)parent())->kdlg_get_edit_widget()->wasWidgetAdded() 
     || ((CKDevelop*)parent())->kdlg_get_edit_widget()->wasWidgetRemoved()
		 || ((CKDevelop*)parent())->kdlg_get_edit_widget()->wasVarnameChanged()){
    QString var;
    bool was_in_buffer = false;
    if( ((CKDevelop*)parent())->isFileInBuffer(prj->getProjectDir() + info.header_file)){
      ((CKDevelop*)parent())->switchToFile(prj->getProjectDir() + info.header_file);
      ((CKDevelop*)parent())->slotFileSave();
      ((CKDevelop*)parent())->slotFileClose();
      was_in_buffer = true;
    }
    
    file.setName(prj->getProjectDir() + info.header_file);
    QStrList list;
    bool ok = true;
    QString str;
    
    if(file.open(IO_ReadOnly)){ // read the header
      while(!stream.eof()){
	list.append(stream.readLine());
      }
    }
    file.close();
    
    if(file.open(IO_WriteOnly)){
      str = list.first();
      ////////////////////////////includes/////////////////////////////
      while(str != 0 && ok){
	if(str.contains("//Generated area. DO NOT EDIT!!!(begin)") != 0){
	  stream << str << "\n";
	  for(var = includes.first();var != 0;var = includes.next()){ // generate the includes
	    stream << var << endl;
	  }
	  ok = false; // finished
	}
	else{
	  stream << str << "\n";
	}
	str = list.next();
      }
      ok = true;
      while(str != 0 && ok){
	if(str.contains("//Generated area. DO NOT EDIT!!!(end)") != 0){
	  stream << str << "\n";
	  ok = false;
	}
	str = list.next();
      }
      ok = true;
      ////////////////////////////variables/////////////////////////////
      while(str != 0 && ok){
	if(str.contains("//Generated area. DO NOT EDIT!!!(begin)") !=0){
	  stream << str << "\n";
	  for(var = variables.first();var != 0;var = variables.next()){
	    stream << "\t" << var << endl;
	  }
	  ok = false; // finished
	}
	else{
	  stream << str << "\n";
	}
	str = list.next();
      }
      ok = true;
      while(str != 0 && ok){
	if(str.contains("//Generated area. DO NOT EDIT!!!(end)") != 0){
	  stream << str << "\n";
	  ok = false;
	}
	str = list.next();
      }
      while(str != 0){
	stream << str << "\n";
	str = list.next();
      }
    }
    file.close();
    if(was_in_buffer){
      ((CKDevelop*)parent())->switchToFile(prj->getProjectDir() + info.header_file);
    }
  }

  ((CKDevelop*)parent())->kdlg_get_edit_widget()->setModified(false);
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->setWidgetAdded(false);
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->setWidgetRemoved(false);
  ((CKDevelop*)parent())->kdlg_get_edit_widget()->setVarnameChanged(false);

  slotFileSave();
}

void KDlgEdit::slotViewGrid()
{
  KDlgGridDialog dlg((QWidget*)parent());

  if (dlg.exec())
    {
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->setGridSize(dlg.getGridX(), dlg.getGridY());
      ((CKDevelop*)parent())->kdlg_get_edit_widget()->mainWidget()->repaintItem();
    }
}

QString KDlgEdit::getRelativeName(QString abs_filename){
  CProject* prj = ((CKDevelop*)parent())->getProject();
  // normalize it a little bit
  abs_filename.replace(QRegExp("///"),"/"); // remove ///
  abs_filename.replace(QRegExp("//"),"/"); // remove //
  abs_filename.replace(QRegExp(prj->getProjectDir()),"");
  return abs_filename;
}
  
void KDlgEdit::generateInitialHeaderFile(TDialogFileInfo info,QString baseclass_header){
  CGenerateNewFile generator;
  CProject* prj = ((CKDevelop*)parent())->getProject(); 
  QString header_file = prj->getProjectDir() + info.header_file;
  generator.genHeaderFile(header_file,prj);


  // modify the header
  QStrList list;
  QFile file(header_file);
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
    QFileInfo fileinfo(header_file);
    stream << "\n#ifndef " + fileinfo.baseName().upper() + "_H\n";
    stream << "#define "+ fileinfo.baseName().upper() + "_H\n\n";
    if(info.baseclass != "Custom"){
      stream << "//Generated area. DO NOT EDIT!!!(begin)\n";
      stream << "//Generated area. DO NOT EDIT!!!(end)\n";
    }
    stream << "\n#include <"+baseclass_header+">\n";
    
    stream << "\n/**\n";
    stream << "  *@author "+ prj->getAuthor() + "\n";
    stream << "  */\n\n";
    stream << "class " + info.classname;
    stream << " : ";
    stream << "public ";
    if(info.baseclass != "Custom"){
      stream << info.baseclass + " ";
    }
    stream << " {\n";
    stream << "   Q_OBJECT\n";
    
    stream << "public: \n";
    stream << "\t" + info.classname+"(";
    stream << "QWidget *parent=0, const char *name=0";
    stream << ");\n";
    stream << "\t~" + info.classname +"();\n\n";

    stream << "protected: \n";
    stream << "\tvoid initDialog();\n";
    stream << "\t//Generated area. DO NOT EDIT!!!(begin)\n";
    stream << "\t//Generated area. DO NOT EDIT!!!(end)\n\n";
    stream << "private: \n";
    stream << "};\n\n#endif\n";
  }
}
void KDlgEdit::generateInitialSourceFile(TDialogFileInfo info){
  CGenerateNewFile generator;
  CProject* prj = ((CKDevelop*)parent())->getProject(); 
  QString source_file = prj->getProjectDir() + info.source_file;
  generator.genCPPFile(source_file,prj);
  QFileInfo header_file_info(prj->getProjectDir() + info.header_file);
  

  // modify the source
  QStrList list;
  QFile file(source_file);
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
    stream << "#include \"" << header_file_info.fileName() << "\"\n\n";


    // constructor
    if(info.baseclass == "QDialog"){
      stream << info.classname + "::" + info.classname 
	+ "(QWidget *parent, const char *name) : QDialog(parent,name,true){\n";
    } 
    else if(info.baseclass == "QTabDialog"){
       stream << info.classname + "::" + info.classname 
	+ "(QWidget *parent, const char *name) : QTabDialog(parent,name,true){\n";
    }
    else {
      stream << info.classname + "::" + info.classname 
	+ "(QWidget *parent, const char *name) : " + info.baseclass +"(parent,name){\n";
    }
    
    stream << "\tinitDialog();\n}\n\n";
    // destructor
    stream << info.classname + "::~" + info.classname +"(){\n}\n";
  }
}

void KDlgEdit::generateWidget(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  if ((!wid) || (!stream)){
    return;
  }
#define generateIfWidget(type,include,gen_meth) \
  if(wid->itemClass() == type){\
    variables.append(QString(type)+  " *"+wid->getProps()->getPropValue("VarName")+";");\
    if(includes.contains("#include <"+QString(include)+">") == 0){\
      includes.append("#include <"+QString(include)+">");\
    }\
    gen_meth(wid,stream,_parent);\
  }

  generateIfWidget("QPushButton","qpushbutton.h",generateQPushButton);
  generateIfWidget("QLineEdit","qlineedit.h",generateQLineEdit);
  generateIfWidget("QLCDNumber","qlcdnumber.h",generateQLCDNumber);
  generateIfWidget("QLabel","qlabel.h",generateQLabel);
  generateIfWidget("QRadioButton","qradiobutton.h",generateQRadioButton);
  generateIfWidget("QCheckBox","qcheckbox.h",generateQCheckBox);
  generateIfWidget("QComboBox","qcombobox.h",generateQComboBox);
  generateIfWidget("QListBox","qlistbox.h",generateQListBox);
  generateIfWidget("QProgressBar","qprogressbar.h",generateQProgressBar);
  generateIfWidget("QMultiLineEdit","qmultilinedit.h",generateQMultiLineEdit);
  generateIfWidget("QGroupBox","qgroupbox.h",generateQGroupBox);
  generateIfWidget("QSlider","qslider.h",generateQSlider);
  generateIfWidget("QScrollBar","qscrollbar.h",generateQScrollBar);
  generateIfWidget("QSpinBox","qspinbox.h",generateQSpinBox);
  generateIfWidget("KColorButton","kcolorbtn.h",generateKColorButton);
  generateIfWidget("QListView","qlistview.h",generateQListView);
  generateIfWidget("KCombo","kcombo.h",generateKCombo);
  generateIfWidget("KDatePicker","kdatepik.h",generateKDatePicker);
  generateIfWidget("KDateTable","kdatetbl.h",generateKDateTable);
  generateIfWidget("KKeyButton","kkeydialog.h",generateKKeyButton);
  generateIfWidget("KLed","kled.h",generateKLed);
  generateIfWidget("KLedLamp","kledlamp.h",generateKLedLamp);
  generateIfWidget("KProgress","kprogress.h",generateKProgress);
  generateIfWidget("KRestrictedLine","krestrictedline.h",generateKRestrictedLine);
  generateIfWidget("KSeparator","kseparator.h",generateKSeparator);
  generateIfWidget("KTreeList","ktreelist.h",generateKTreeList);




  





#undef generateIfWidget


  if (wid->itemClass().upper() == "QWIDGET"){
    if(wid->getProps()->getPropValue("VarName") != "this"){
      variables.append("QWidget *"+wid->getProps()->getPropValue("VarName")+";");
    }
    if(includes.contains("#include <qwidget.h>") == 0){
      includes.append("#include <qwidget.h>");
    }
    generateQWidget(wid,stream,_parent);
    KDlgItemDatabase *cdb = wid->getChildDb();
    if (cdb)
      {
	KDlgItem_Base *cdit = cdb->getFirst();
	while (cdit)
	  {
	    generateWidget( (KDlgItem_Widget*)cdit, stream, wid->getProps()->getPropValue("VarName"));
	    cdit = cdb->getNext();
	  }
      }
  }
}

void KDlgEdit::generateQLCDNumber(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QLCDNumber(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  //display
  *stream << varname_p + "display(" + props->getPropValue("Value") +");\n";
  
  //setNumDigits
  if(props->getPropValue("NumDigits") != ""){
    *stream << varname_p + "setNumDigits(" + props->getPropValue("NumDigits") + ");\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateQLineEdit(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QLineEdit(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  //setText
  if(props->getPropValue("Text") != ""){
	  if(((CKDevelop*)parent())->getProject()->isKDEProject()){
  	  *stream << varname_p + "setText(i18n(\""+ props->getPropValue("Text") +"\"));\n";
 	 }
 	 else{
 	   *stream << varname_p + "setText(\""+ props->getPropValue("Text") +"\");\n";
 	 }
  }
  //setMaxLenght
  if(props->getPropValue("MaxLength") != ""){
    *stream << varname_p + "setMaxLength("+ props->getPropValue("MaxLength") +");\n";
  }
  //setFrame
  if(props->getPropValue("hasFrame") != "TRUE"){
    *stream << varname_p + "setFrame(false);\n";
  }
  //isTextSelected
  if(props->getPropValue("isTextSelected") != "FALSE"){
    *stream << varname_p + "selectAll();\n";
  }
  
  // CursorPosition
  if(props->getPropValue("CursorPosition") != ""){
    *stream << varname_p + "setCursorPosition(" + props->getPropValue("CursorPosition") +");\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateQMultiLineEdit(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QMultiLineEdit(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  // setText
  if(props->getPropValue("Text") != ""){
	  if(((CKDevelop*)parent())->getProject()->isKDEProject()){
  	  *stream << varname_p + "setText(i18n(\""+ props->getPropValue("Text") +"\"));\n";
 	 }
 	 else{
 	   *stream << varname_p + "setText(\""+ props->getPropValue("Text") +"\");\n";
 	 }
 	}

  //isTextSelected
  if(props->getPropValue("isTextSelected") != "FALSE"){
    *stream << varname_p + "selectAll();\n";
  }
  //isAutoUpdate
  if(props->getPropValue("isAutoUpdate") == "FALSE"){
    *stream << varname_p + "setAutoUpdate(false);\n";
  }
  //isReadOnly
   if(props->getPropValue("isReadOnly") == "TRUE"){
     *stream << varname_p + "setReadOnly(true);\n";
   }
   //isOverWriteMode
   if(props->getPropValue("isOverWriteMode") == "TRUE"){
     *stream << varname_p + "setOverwriteMode(true);\n";
   }
   if(props->getPropValue("setFixedVisibleLines") != ""){
     *stream << varname_p + "setFixedVisibleLines("+props->getPropValue("setFixedVisibleLines")+");\n";
   }
  *stream << "\n";
}
void KDlgEdit::generateQProgressBar(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QProgressBar(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  

  //setTotalSteps
  if(props->getPropValue("TotalSteps") != ""){
    *stream << varname_p + "setTotalSteps("+ props->getPropValue("TotalSteps") +");\n";
  }
  
  *stream << "\n";
}
void KDlgEdit::generateQSlider(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  cerr << "SLIDER";
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QSlider(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  
  //setValue
  if(props->getPropValue("Value") != ""){
    *stream << varname_p + "setValue("+ props->getPropValue("Value") +");\n";
  }
  //setRange
  *stream << varname_p + "setRange("+ props->getPropValue("MinValue") +","+props->getPropValue("MaxValue")+");\n";
  //setTracking
  if(props->getPropValue("isTracking") == "FALSE"){
    *stream << varname_p + "setTracking(false);\n";
  }
  // setOrientation
  if(props->getPropValue("Orientation") == "Horizontal"){
    *stream << varname_p + "setOrientation(QSlider::Horizontal);\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateQSpinBox(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QSpinBox(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  
  //setValue
  if(props->getPropValue("Value") != ""){
    *stream << varname_p + "setValue("+ props->getPropValue("Value") +");\n";
  }
  //setRange
  *stream << varname_p + "setRange("+ props->getPropValue("MinValue") +","+props->getPropValue("MaxValue")+");\n";
  //setTracking
  if(props->getPropValue("isWrapping") == "TRUE"){
    *stream << varname_p + "seWrapping(true);\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateQScrollBar(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  cerr << "SCROLLBAR";
  KDlgPropertyBase* props = wid->getProps();

  *stream << "\t" + props->getPropValue("VarName") +" = new QScrollBar(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  
  //setValue
  if(props->getPropValue("Value") != ""){
    *stream << varname_p + "setValue("+ props->getPropValue("Value") +");\n";
  }
  //setRange
  *stream << varname_p + "setRange("+ props->getPropValue("MinValue") +","+props->getPropValue("MaxValue")+");\n";
  //setTracking
  if(props->getPropValue("isTracking") == "FALSE"){
    *stream << varname_p + "setTracking(false);\n";
  }
  // setOrientation
  if(props->getPropValue("Orientation") == "Horizontal"){
    *stream << varname_p + "setOrientation(QScrollBar::Horizontal);\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateQRadioButton(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QRadioButton(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  //setText
  if(props->getPropValue("Text") != ""){
	  if(((CKDevelop*)parent())->getProject()->isKDEProject()){
  	  *stream << varname_p + "setText(i18n(\""+props->getPropValue("Text") +"\"));\n";
 	 }
 	 else{
 	   *stream << varname_p + "setText(\""+props->getPropValue("Text") +"\");\n";
 	 }
 	}
  //  setChecked
  if(props->getPropValue("isChecked") == "TRUE"){
    *stream << varname_p + "setChecked(true);\n";
  }
  //isAutoResize
  if(props->getPropValue("isAutoResize") == "TRUE"){
    *stream << varname_p + "setAutoResize(true);\n";
  }
  //isAutoRepeat
  if(props->getPropValue("isAutoRepeat") == "TRUE"){
    *stream << varname_p + "setAutoRepeat(true);\n";
  }
  //Pixmap
  if(props->getPropValue("Pixmap") != ""){
    if(local_includes.contains("#include <qpixmap.h>") == 0){
      local_includes.append("#include <qpixmap.h>");
    }
    *stream << varname_p + "setPixmap(QPixmap(\""+props->getPropValue("Pixmap")+"\"));\n";
    
  }
  *stream << "\n";
  
}
void KDlgEdit::generateQCheckBox(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QCheckBox(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  
  //setText
  if(props->getPropValue("Text") != ""){
	  if(((CKDevelop*)parent())->getProject()->isKDEProject()){
  	  *stream << varname_p + "setText(i18n(\""+props->getPropValue("Text") +"\"));\n";
	  }
  	else{
    	*stream << varname_p + "setText(\""+props->getPropValue("Text") +"\");\n";
	  }
	}
 
  //  setChecked
  if(props->getPropValue("isChecked") == "TRUE"){
    *stream << varname_p + "setChecked(true);\n";
  }
  //isAutoResize
  if(props->getPropValue("isAutoResize") == "TRUE"){
    *stream << varname_p + "setAutoResize(true);\n";
  }
  //isAutoRepeat
  if(props->getPropValue("isAutoRepeat") == "TRUE"){
    *stream << varname_p + "setAutoRepeat(true);\n";
  }
  //Pixmap
  if(props->getPropValue("Pixmap") != ""){
    if(local_includes.contains("#include <qpixmap.h>") == 0){
      local_includes.append("#include <qpixmap.h>");
    }
    *stream << varname_p + "setPixmap(QPixmap(\""+props->getPropValue("Pixmap")+"\"));\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateQComboBox(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QComboBox(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  
  //isAutoResize
  if(props->getPropValue("isAutoResize") == "TRUE"){
    *stream << varname_p + "setAutoResize(true);\n";
  }

  *stream << "\n";
}

void KDlgEdit::generateQLabel(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QLabel(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  //setText
  if(props->getPropValue("Text") != ""){
	  if(((CKDevelop*)parent())->getProject()->isKDEProject()){
  	  *stream << varname_p + "setText(i18n(\""+props->getPropValue("Text") +"\"));\n";
 	 }
 	 else{
  	  *stream << varname_p + "setText(\""+props->getPropValue("Text") +"\");\n";
 	 }
  }
  //isAutoResize
  if(props->getPropValue("isAutoResize") == "TRUE"){
    *stream << varname_p + "setAutoResize(true);\n";
  }
  //Margin
  if(props->getPropValue("Margin") != ""){
    *stream << varname_p + "setMargin("+props->getPropValue("Margin")+");\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateQListBox(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
   *stream << "\t" + props->getPropValue("VarName") +" = new QListBox(" + _parent +",\"" 
     +props->getPropValue("Name") + "\");\n";
   generateQWidget(wid,stream,_parent);

   QString varname_p = "\t"+props->getPropValue("VarName") + "->";

   // //isAutoUpdate
   if(props->getPropValue("isAutoUpdate") == "FALSE"){
     *stream << varname_p + "setAutoUpdate(false);\n";
   }
   //isAutoScroll
   if(props->getPropValue("isAutoScroll") == "FALSE"){
     *stream << varname_p + "setAutoScroll(false);\n";
   }
    //isAutoScrollBar
   if(props->getPropValue("isAutoScrollBar") == "FALSE"){
      *stream << varname_p + "setAutoScrollBar(false);\n";
    }
   //  //isAutoBottomScrollBar
    if(props->getPropValue("isAutoBottomScrollBar") == "FALSE"){
      *stream << varname_p + "setAutoBottomScrollBar(false);\n";
    }
   //isBottomScrollBar
   if(props->getPropValue("isBottomScrollBar") == "TRUE"){
     *stream << varname_p + "setBottomScrollBar(true);\n";
   }
   //isDragSelect
   if(props->getPropValue("isDragSelect") == "FALSE"){
     *stream << varname_p + "setDragSelect(false);\n";
   }
   //isSmoothScrolling
   if(props->getPropValue("isSmoothScrolling") == "FALSE"){
     *stream << varname_p + "setSmoothScrolling(false);\n";
   }
   if(props->getPropValue("setFixedVisibleLines") != ""){
     *stream << varname_p + "setFixedVisibleLines("+props->getPropValue("setFixedVisibleLines")+");\n";
   }

   *stream << "\n";
}
void KDlgEdit::generateQPushButton(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QPushButton(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  //setText
  if(props->getPropValue("Text") != ""){
	  if(((CKDevelop*)parent())->getProject()->isKDEProject()){
  	  *stream << varname_p + "setText(i18n(\""+props->getPropValue("Text") +"\"));\n";
	  }
 	 else{
 	   *stream << varname_p + "setText(\""+props->getPropValue("Text") +"\");\n";
 	 }
  }
  //isDefault
  if(props->getPropValue("isDefault") == "TRUE"){
    *stream << varname_p + "setDefault(true);\n";
  }
  //IsAutoDefaul
  if(props->getPropValue("isAutoDefault") == "TRUE"){
    *stream << varname_p + "setAutoDefault(true);\n";
  }
  //IsToggleButton
  if(props->getPropValue("isToggleButton") == "TRUE"){
    *stream << varname_p + "setToggleButton(true);\n";
  }
  //isToogledOn
  if(props->getPropValue("isToggledOn") == "TRUE"){
    *stream << varname_p + "setToggleOn(true);\n";
  }
  //IsMenuButton
  if(props->getPropValue("isMenuButton") == "TRUE"){
    *stream << varname_p + "setIsMenuButton(true);\n";
  }
  //isAutoResize
  if(props->getPropValue("isAutoResize") == "TRUE"){
    *stream << varname_p + "setAutoResize(true);\n";
  }
  //isAutoRepeat
  if(props->getPropValue("isAutoRepeat") == "TRUE"){
    *stream << varname_p + "setAutoRepeat(true);\n";
  }
  //Pixmap
  if(props->getPropValue("Pixmap") != ""){
    if(local_includes.contains("#include <qpixmap.h>") == 0){
      local_includes.append("#include <qpixmap.h>");
    }
    *stream << varname_p + "setPixmap(QPixmap(\""+props->getPropValue("Pixmap")+"\"));\n";
  }
  *stream << "\n";
  
}
void KDlgEdit::generateQGroupBox(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QGroupBox(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  //setText
  if(props->getPropValue("Title") != ""){
    if(((CKDevelop*)parent())->getProject()->isKDEProject()){
      //      *stream << varname_p + "setTitle(i18n(\""+ props->getPropValue("Title") +"\"));\n";
    }else{
      *stream << varname_p + "setTitle(\""+ props->getPropValue("Title") +"\");\n";
    }
  }

  *stream << "\n";
}
void KDlgEdit::generateQListView(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new QListView(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";

}
void KDlgEdit::generateKColorButton(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KColorButton(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  //setText
  if(props->getPropValue("Text") != ""){
    if(((CKDevelop*)parent())->getProject()->isKDEProject()){
      *stream << varname_p + "setText(i18n(\""+props->getPropValue("Text") +"\"));\n";
    }
    else{
      *stream << varname_p + "setText(\""+props->getPropValue("Text") +"\");\n";
    }
  }
  //isDefault
  if(props->getPropValue("isDefault") == "TRUE"){
    *stream << varname_p + "setDefault(true);\n";
  }
  //IsAutoDefaul
  if(props->getPropValue("isAutoDefault") == "TRUE"){
    *stream << varname_p + "setAutoDefault(true);\n";
  }
  //IsToggleButton
  if(props->getPropValue("isToggleButton") == "TRUE"){
    *stream << varname_p + "setToggleButton(true);\n";
  }
  //isToogledOn
  if(props->getPropValue("isToggledOn") == "TRUE"){
    *stream << varname_p + "setToggleOn(true);\n";
  }
  //IsMenuButton
  if(props->getPropValue("isMenuButton") == "TRUE"){
    *stream << varname_p + "setIsMenuButton(true);\n";
  }
  //isAutoResize
  if(props->getPropValue("isAutoResize") == "TRUE"){
    *stream << varname_p + "setAutoResize(true);\n";
  }
  //isAutoRepeat
  if(props->getPropValue("isAutoRepeat") == "TRUE"){
    *stream << varname_p + "setAutoRepeat(true);\n";
  }
  //Pixmap
  if(props->getPropValue("Pixmap") != ""){
    if(local_includes.contains("#include <qpixmap.h>") == 0){
      local_includes.append("#include <qpixmap.h>");
    }
    *stream << varname_p + "setPixmap(QPixmap(\""+props->getPropValue("Pixmap")+"\"));\n";
  }
  if(props->getPropValue("DisplayedColor") != ""){
    QColor col = Str2Color(props->getPropValue("DisplayedColor"));
    *stream << varname_p + "setColor(QColor("+QString().setNum(col.red()) + "," 
      + QString().setNum(col.green()) + "," 
      + QString().setNum(col.blue()) + "));";
  }
  *stream << "\n";
  
}
void KDlgEdit::generateKCombo(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KCombo(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";

  //isAutoResize
  if(props->getPropValue("isAutoResize") == "TRUE"){
    *stream << varname_p + "setAutoResize(true);\n";
  }
  //setText
  if(props->getPropValue("Text") != ""){
	  if(((CKDevelop*)parent())->getProject()->isKDEProject()){
  	  *stream << varname_p + "setText(i18n(\""+ props->getPropValue("Text") +"\"));\n";
 	 }
 	 else{
 	   *stream << varname_p + "setText(\""+ props->getPropValue("Text") +"\");\n";
 	 }
  }
  *stream << "\n";
}
void KDlgEdit::generateKDatePicker(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KDatePicker(" + _parent +");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";

  //setFontsize
  if(props->getPropValue("FontSize") != ""){
    *stream << varname_p + "setFontSize("+props->getPropValue("FontSize")+");\n";
  }
  
}
void KDlgEdit::generateKDateTable(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KDateTable(" + _parent +");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
}
void KDlgEdit::generateKKeyButton(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KKeyButton(\"" + props->getPropValue("Name") +"\"," +  _parent +");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
}
void KDlgEdit::generateKLed(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KLed(KLed::yellow ," + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);
  
  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
}
void KDlgEdit::generateKLedLamp(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KLedLamp(" + _parent + ");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
}
void KDlgEdit::generateKProgress(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KProgress(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
}
void KDlgEdit::generateKRestrictedLine(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KRestrictedLine(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
}
void KDlgEdit::generateKSeparator(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KSeparator(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
  
  if(props->getPropValue("Orientation") == "Horizontal"){
    *stream << varname_p + "setOrientation(KSeparator::HLine);\n";
  }
  *stream << "\n";
}
void KDlgEdit::generateKTreeList(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  *stream << "\t" + props->getPropValue("VarName") +" = new KTreeList(" + _parent +",\"" 
    +props->getPropValue("Name") + "\");\n";
  generateQWidget(wid,stream,_parent);

  QString varname_p = "\t"+props->getPropValue("VarName") + "->";
}

void KDlgEdit::generateQWidget(KDlgItem_Widget *wid, QTextStream *stream,QString _parent){
  KDlgPropertyBase* props = wid->getProps();
  QString varname_p;
  // new
  if(props->getPropValue("VarName") != "this" && wid->itemClass() == "QWidget"){
    *stream << "\t" + props->getPropValue("VarName") +" = new QWidget(" + _parent +",\"" 
      +props->getPropValue("Name") + "\");\n";
  }
   
  varname_p = "\t"+props->getPropValue("VarName") + "->";
  ///////////////////////////////////////geometry////////////////////////////////////
  // setGeometry
  if(props->getPropValue("VarName") != "this"){
    *stream << varname_p + "setGeometry("+props->getPropValue("X")+","
      +props->getPropValue("Y")+","+props->getPropValue("Width")+","+props->getPropValue("Height")+");\n";
  }
  else {
    *stream << varname_p + "resize("+props->getPropValue("Width")+","+props->getPropValue("Height")+");\n";
  }

  //setMinimumSize
  if(props->getPropValue("MinWidth") != "0" || props->getPropValue("MinHeight") != "0"){
    *stream << varname_p + "setMinimumSize("+props->getPropValue("MinWidth")+","
    +props->getPropValue("MinHeight")+");\n";
  }

  //setMaximumSize
  if(props->getPropValue("MaxWidth") != "" && props->getPropValue("MaxHeight") != ""){
    *stream << varname_p + "setMaximumSize("+props->getPropValue("MaxWidth")+","
    +props->getPropValue("MaxHeight")+");\n";
  }
  //setFixedSize
  if(props->getPropValue("IsFixedSize") == "TRUE"){
    *stream << varname_p + "setFixedSize("+props->getPropValue("Width")
      +","+props->getPropValue("Height")+");\n";
  }
  //setSizeIncrement ( int w, int h )
  if(props->getPropValue("SizeIncX") != "" && props->getPropValue("SizeIncY") != ""){
    *stream << varname_p + "setSizeIncrement("+props->getPropValue("SizeIncX")
      +","+props->getPropValue("SizeIncY")+");\n";
  }
  /////////////////////////////////General///////////////////////////
  //IsHidden
  if(props->getPropValue("IsHidden") != "FALSE"){
    *stream << varname_p + "hide();\n";
  }
  //isEnabled
  if(props->getPropValue("IsEnabled") != "TRUE"){
    *stream << varname_p + "setEnabled(false);\n";
  }
  ////////////////////////////////C++ Code//////////////////////////
  
  
  ////////////////////////////////Appearance/////////////////////////
  //BgPixmap
  if(props->getPropValue("BgPixmap") != ""){
    if(local_includes.contains("#include <qpixmap.h>") == 0){
      local_includes.append("#include <qpixmap.h>");
    }
    *stream << varname_p + "setBackgroundPixmap(QPixmap(\""+props->getPropValue("BgPixmap")+"\"));\n";
  }
  //MaskBitmap
  if(props->getPropValue("MaskBitmap") != ""){
    if(local_includes.contains("#include <qbitmap.h>") == 0){
      local_includes.append("#include <qbitmap.h>");
    }
    *stream << varname_p + "setMask(QBitmap(\""+props->getPropValue("MaskBitmap")+"\"));\n";
  }
}



