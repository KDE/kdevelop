/***************************************************************************
                 classview.cpp - the classview specific part of CKDevelop
                             -------------------                                         

    version              :                                   
    begin                : 13 Aug 1998                                        
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
#include "ckdevelop.h"
#include <qstrlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <iostream.h>
#include <qregexp.h>
#include "debug.h"
#include "cclassview.h"

void CKDevelop::slotClassTreeSelected(int index){
  if(!class_tree->leftButton()) return; // not pressed the left button

  KPath* path;
  QString* name,*name2;
  int level;
  QString type;
  QString text,last_textpart;
  QRegExp regexp;
  path = class_tree->itemPath(index);
  name = path->pop();


  level = path->count();
  if (level == 1) { // class,struct,or global
    CVGotoClassDecl(*name);
  }
  if (level == 2) { // methods or variables
    if (name->contains("(") != 0){ // it is a method
      name2 = path->pop();// get the classname
      CVGotoMethodeImpl(*name2,*name);
      //cerr << endl << "METHNAME: " << *name;
    }
    else { // it is a variable
      name2 = path->pop();// get the classname
      CVGotoClassVarDecl(*name2,*name);
      //cerr << endl << "VARNAME: " << *name; 
    }
  } // end level 2

}

void CKDevelop::slotCVViewDeclaration(int index){
  KPath* path;
  QString* name,*name2;
  int level;
  QString type;
  QString text,last_textpart;
  QRegExp regexp;
  path = class_tree->itemPath(index);
  name = path->pop();

  level = path->count();
  if (level == 1) { // class,struct,or global
    CVGotoClassDecl(*name);
  }
  if (level == 2) { // methods or variables
    if (name->contains("(") != 0){ // it is a method
      name2 = path->pop();// get the classname
      CVGotoMethodeImpl(*name2,*name);
      //cerr << endl << "METHNAME: " << *name;
    }
    else { // it is a variable
      name2 = path->pop();// get the classname
      CVGotoClassVarDecl(*name2,*name);
      //cerr << endl << "VARNAME: " << *name; 
    }
  } // end level 2


}

void CKDevelop::slotCVViewDefinition(int index){

  KPath* path;
  QString* name,*name2;
  int level;
  QString type;
  QString text,last_textpart;
  QRegExp regexp;
  path = class_tree->itemPath(index);
  name = path->pop();
  
  level = path->count();
  if (level == 1) { // class,struct,or global
    CVGotoClassDecl(*name);
  }
  if (level == 2) { // methods or variables
    if (name->contains("(") != 0){ // it is a method
      name2 = path->pop();// get the classname
      CVGotoMethodeImpl(*name2,*name);
      //cerr << endl << "METHNAME: " << *name;
    }
    else { // it is a variable
      name2 = path->pop();// get the classname
      CVGotoClassVarDecl(*name2,*name);
      //cerr << endl << "VARNAME: " << *name; 
    }
  } // end level 2

}

int CKDevelop::CVGotoClassDecl(QString classname){
  int pos1;
  int num_classes;
  QString text,classname_test;
  QString last_textpart;
  QRegExp regexp;
  int act_pos =0;
  TClassInfo* class_info,*info_found=0;
  int count=0,index=0;

  // search the class_info
  for(class_info =class_tree->class_infos->first(); class_info !=0;
      class_info =class_tree->class_infos->next()){
    if (class_info->classname == classname){
      info_found = class_info;
      index=count;
    }
    else {
      count++;
    }
  }
  if(info_found == 0){ // not found !!!
    return 0; // prevent a segfault in the next line
  }

  switchToFile(prj->getProjectDir() + prj->getSubDir() + info_found->filename);

  text = edit_widget->text();
  class_tree->CVRemoveAllComments(&text);

  num_classes = text.contains(info_found->type);
  while(num_classes != 0){
    
    regexp = info_found->type + " ";
    act_pos = text.find(regexp,act_pos); // find the class token
    num_classes--;
    regexp = " [a-zA-Z]";
    act_pos = text.find(regexp,act_pos) +1; // find the begin of the classname/structname
    regexp = "[ {\n]";
    pos1 = text.find(regexp,act_pos); // find the end of the classname
    classname_test = text.mid(act_pos,(pos1-act_pos)); // get the classname
    if (classname_test == classname) num_classes =0; // found
  }
    
  edit_widget->gotoPos(act_pos,text);
  return act_pos;
  
}


void CKDevelop::CVGotoMethodeImpl(QString classname,QString meth_name){
  QString compl_name = classname + "::" + meth_name;

  //cerr << "COMPLETE_NAME:" << compl_name;
  QString filename;
  QFile file;
  QTextStream in_stream(&file);  
  QString stream;
  int pos;
  QString last_textpart;
  QRegExp regexp = "[ (\t]";

  pos = compl_name.find( regexp );
  if ( pos > 0 )
    compl_name.truncate( pos );
  debug( compl_name );
  
  if (!prj->getSources().isEmpty()){
    for(filename = prj->getSources().first();filename != 0;filename = prj->getSources().next()){
      file.setName(prj->getProjectDir() + prj->getSubDir() + filename);
      file.open(IO_ReadOnly);
      // while(!in_stream.eof()){
// 	stream = stream + in_stream.readLine() + "\n"; // read it
//       }
      stream.resize(file.size()+1);
      
      file.readBlock(stream.data(),file.size());
      file.close(); 
      if (stream.find(compl_name) != -1){
	switchToFile(prj->getProjectDir() + prj->getSubDir() + filename);
	stream = edit_widget->text();
	class_tree->CVRemoveAllComments(&stream);
	pos = stream.find(compl_name);
	edit_widget->gotoPos(pos,stream);
	return; // exit;
      }
    } // end for
  } // end if 
}
void CKDevelop::CVGotoClassVarDecl(QString classname,QString var_name){
  int pos, nextpos;
  QString text,last_textpart;
  QRegExp regexp = "[a-zA-Z0-9_^ ^;^\t^\n^*]";
 
  pos = CVGotoClassDecl(classname);
  nextpos = pos;
  text = edit_widget->text();
  class_tree->CVRemoveAllComments(&text);
  
  do
  {
  pos = text.find(var_name,nextpos);
  if(pos == -1){
    break;// error
  }
  debug("pos - 1: >" + text.mid(pos - 1, 1) + "<");
  debug("pos + length: >" + text.mid(pos + var_name.length(), 1) + "<");
  nextpos = pos + var_name.length();
  } while ( regexp.match( text.mid(pos - 1, 1) )
            || regexp.match( text.mid(pos + var_name.length(), 1) ) );
  
  edit_widget->gotoPos(pos,text);

}

void CKDevelop::slotClassChoiceCombo(int index){
  if(!project){
    return;
  }
  KCombo* class_combo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  KCombo* method_combo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);
  
  QString classname = class_combo->text(index);
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",classname.data());
  if (classname == i18n("Classes")) return; 
  TClassInfo* class_info=0,*info_found=0;
  TMethodInfo* method_info;
  // search the class_info
   for(class_info =class_tree->class_infos->first(); class_info !=0;
       class_info =class_tree->class_infos->next()){
     if (class_info->classname == classname){
       info_found = class_info;
     }
   }
   if (info_found != 0){
     method_combo->clear();
     for(method_info = info_found->method_infos.first();method_info != 0;
	 method_info = info_found->method_infos.next()){
       method_combo->insertItem(method_info->name);
     }
   }
   CVGotoClassDecl(classname);
   
}
void CKDevelop::slotMethodChoiceCombo(int index){
  if(!project){
    return;
  }
  KCombo* class_combo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  KCombo* method_combo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);
  
  QString meth_name = method_combo->text(index);
  QString classname = class_combo->currentText();
  
  CVGotoMethodeImpl(classname,meth_name);
}

void CKDevelop::refreshClassCombos(){
  TClassInfo* class_info;
  TMethodInfo* method_info;
  KCombo* class_combo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  KCombo* method_combo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);
	class_combo->setUpdatesEnabled(false);
	method_combo->setUpdatesEnabled(false);
  class_combo->clear();
  method_combo->clear();
  if (class_tree->class_infos->isEmpty()) return; // no classes
  
  for(class_info = class_tree->class_infos->first();class_info != 0;
      class_info = class_tree->class_infos->next()){
    class_combo->insertItem(class_info->classname);
  }
  class_info = class_tree->class_infos->first();
  for(method_info = class_info->method_infos.first();method_info != 0;
      method_info = class_info->method_infos.next()){
    method_combo->insertItem(method_info->name);
  }
	class_combo->setUpdatesEnabled(true);
	method_combo->setUpdatesEnabled(true);
	class_combo->repaint();
	method_combo->repaint();

}












