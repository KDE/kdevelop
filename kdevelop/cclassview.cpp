/***************************************************************************
                 classview.cpp - the classview for kdevelop
                             -------------------                                         

    version              :                                   
    begin                : 13 Aug 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de
    some bugfixes        : Jost Schenk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "cclassview.h"
#include "ckdevelop.h"
#include <qstrlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <iostream.h>
#include <qregexp.h>
#include "debug.h"

CClassView::CClassView(QWidget*parent,const char* name) : KTreeList(parent,name){
  streamed_files = new QList<TStreamedFile>;
  streamed_files->setAutoDelete(true);
  class_infos = new QList<TClassInfo>;
  class_infos->setAutoDelete(true);

  icon_loader = KApplication::getKApplication()->getIconLoader();
  left_button = true;
  right_button = false;
  class_pop = new KPopupMenu();
  class_pop->setTitle(i18n("Class:"));
  class_pop->insertItem(i18n("Declaration"),this,SLOT(slotViewDeclaration()));
  class_pop->insertSeparator();
  class_pop->insertItem(i18n("New Class..."),this,SLOT(slotClassNew()));
  //  class_pop->insertItem(i18n("Remove Class "),this,SLOT(slotClassRemove())); I think it is not
  // usefull, or? -Sandy
  // class_pop->insertItem(i18n("Delete Class..."),this,SLOT(slotClassDelete()));
  //  class_pop->insertSeparator();
  // class_pop->insertItem(i18n("New Method"),this,SLOT(slotMethodNew()));
  // class_pop->insertItem(i18n("New Variable"),this,SLOT(slotVariableNew()));


  member_pop = new KPopupMenu();
  member_pop->setTitle(i18n("Method:"));
  //member_pop->insertItem(i18n("Declaration"),this,SLOT(slotViewDeclaration()));
  member_pop->insertItem(i18n("Definition"),this,SLOT(slotViewDefinition()));
  //member_pop->insertSeparator();
  //member_pop->insertItem(i18n("New Method"),this,SLOT(slotMethodNew()));
  //member_pop->insertItem(i18n("New Variable"),this,SLOT(slotVariableNew()));

  project_pop = new KPopupMenu();
  project_pop->setTitle(i18n("Project:"));
  project_pop->insertItem(i18n("New File..."),this,SLOT(slotFileNew()));
  project_pop->insertItem(i18n("New Class..."),this,SLOT(slotClassNew()));
  project_pop->insertSeparator();
  project_pop->insertItem(i18n("Options..."),this,SLOT(slotProjectOptions()));

  connect(this,SIGNAL(singleSelected(int)),SLOT(slotSingleSelected(int)));
  
}
CClassView::~CClassView(){
}

/** read all headers and sources and put it into a variables 'streamed files'
  */
void CClassView::CVReadAllFiles(){
  QFile file;
  QTextStream in_stream(&file);
  QString filename;
  TStreamedFile* stream_info;
  streamed_files->clear();
  //read the headers
  if (!prj_info->getHeaders().isEmpty()){
    QStrList headers = prj_info->getHeaders();
    for(filename = headers.first();filename != 0;filename = headers.next()){
      stream_info = new TStreamedFile;

      file.setName(filename);
      stream_info->filename = filename;
      if (file.exists()){
	file.open(IO_ReadOnly);
	stream_info->stream.resize(file.size()+1);
	file.readBlock(stream_info->stream.data(),file.size());
	file.close();
	CVRemoveAllComments(&stream_info->stream);
	streamed_files->append(stream_info); // add it to the classfiles
      }
    }
  }
  // // //read the sources
//   if (!prj_info->getSources().isEmpty()){
//     for(filename = prj_info->getSources().first();filename != 0;filename = prj_info->getSources().next()){
//       stream_info = new TStreamedFile;
//       file.setName(prj_info->getProjectDir() + prj_info->getSubDir() + filename);
//       stream_info->filename = filename;
//       if (file.exists()){
// 	file.open(IO_ReadOnly);
// 	//cerr << "LESE CPPDATEI" << endl;
// 	 // while(!in_stream.eof()){
// // 	  stream_info->stream = stream_info->stream + in_stream.readLine() + "\n";
// // 	}
// 	stream_info->stream.resize(file.size()+1);
// 	file.readBlock(stream_info->stream,file.size());
// 	file.close();
// 	CVRemoveAllComments(&stream_info->stream);
// 	streamed_files->append(stream_info); // add it to the classfiles
//       }
//     }
//   }

}

void CClassView::refresh(CProject* prj){
  prj_info = prj;
  KPath path;
  TClassInfo* class_info;
  TVarInfo* var_info;
  TMethodInfo* method_info;
  QString pix_dir;
  CVReadAllFiles();
  CVFindTheClasses();
  CVFindTheMethodsAndVars();
  KPath* saved_item_path;

  

  // let the show begin
  // save the current item;
  saved_item_path = itemPath(currentItem());
  //cerr << "ITEM_PATH:" << saved_item_path << endl;
  
  pix_dir = KApplication::kde_datadir() + "/kdevelop/pics/mini/";
  QPixmap class_pix(pix_dir + "CVclass.xpm");
  QPixmap struct_pix(pix_dir + "CVstruct.xpm");
  QPixmap public_var_pix(pix_dir + "CVpublic_var.xpm");
  QPixmap protected_var_pix(pix_dir + "CVprotected_var.xpm");
  QPixmap private_var_pix(pix_dir + "CVprivate_var.xpm");
  QPixmap public_meth_pix(pix_dir + "CVpublic_meth.xpm");
  QPixmap protected_meth_pix(pix_dir + "CVprotected_meth.xpm");
  QPixmap private_meth_pix(pix_dir + "CVprivate_meth.xpm");
  QPixmap kwm_pix = icon_loader->loadMiniIcon("kwm.xpm");
  
  setUpdatesEnabled( false );

  clear();
  QString* p_str = new QString;
  *p_str = prj_info->getProjectName();
  //  path.push(&prj_info->getProjectName());  
  path.push(p_str);
  //  insertItem(prj_info->getProjectName(),&icon_loader->loadMiniIcon("kwm.xpm"));
  insertItem(*p_str,&kwm_pix);
  if (class_infos->isEmpty()) return; // no classes
  
  for(class_info = class_infos->first();class_info != 0;class_info = class_infos->next()){
    if (class_info->type == "class"){
    addChildItem(class_info->classname,&class_pix,&path);
    }
    else{
      addChildItem(class_info->classname,&struct_pix,&path);
    }
    path.push(&class_info->classname);
    
    // add the methods
    if(!class_info->method_infos.isEmpty()){
      
      for(method_info = class_info->method_infos.first();method_info != 0;
 	  method_info = class_info->method_infos.next()){
	if (method_info->visibility == "public"){
	addChildItem(method_info->name,&public_meth_pix,&path);
	}
	if (method_info->visibility == "protected"){
	  addChildItem(method_info->name,&protected_meth_pix,&path);
	}
	if (method_info->visibility == "private"){
	  addChildItem(method_info->name,&private_meth_pix,&path);
	}
      }
    }
    
    // add the variables
    if(!class_info->variable_infos.isEmpty()){
      
      for(var_info = class_info->variable_infos.first();var_info != 0;
 	  var_info = class_info->variable_infos.next()){
	if (var_info->visibility == "public"){
	  addChildItem(var_info->name,&public_var_pix,&path);
	}
	if (var_info->visibility == "protected"){
	  addChildItem(var_info->name,&protected_var_pix,&path);
	}
	if (var_info->visibility == "private"){
	  addChildItem(var_info->name,&private_var_pix,&path);
	}
      } // end for
    }
    
     path.pop();
   }
  setExpandLevel(1);
  setUpdatesEnabled( TRUE );
  repaint();

 //  if(saved_item_path != 0){
//     QString* item_name = saved_item_path->pop();
//     int parent_index = itemIndex(itemAt(saved_item_path));
//     cerr << "ParentIndex:" << parent_index;
//     setExpandLevel(0);
//     expandItem(parent_index);
//     //    setCurrentItem(item_index);
//   }

   // refresh the comboboxes in the toolbar
  

 
}

bool CClassView::leftButton(){
  return left_button;
}
bool CClassView::rightButton(){
  return right_button;
}
void CClassView::mousePressEvent(QMouseEvent* event){
  if(event->button() == RightButton){    
    left_button = false;
    right_button = true;
  }
  if(event->button() == LeftButton){
    left_button = true;
    right_button = false;
  }
  mouse_pos.setX(event->pos().x());
  mouse_pos.setY(event->pos().y());
  KTreeList::mousePressEvent(event); 
}

void CClassView::slotSingleSelected(int index){
  if(rightButton()){
    if(isMethod(index)){
      member_pop->popup(this->mapToGlobal(mouse_pos));
    } 
    else if(isClass(index)){
      class_pop->popup(this->mapToGlobal(mouse_pos));

    }
    else{
      project_pop->popup(this->mapToGlobal(mouse_pos));
    }
  }
}

bool CClassView::isClass(int index){
  KTreeListItem* current = itemAt(index);
  if(current == 0) return false;
  KTreeListItem* parent = current->getParent();
  if(parent == 0) return false;
  KTreeListItem* pparent = parent->getParent();
  if(pparent == 0) return false;  
  KTreeListItem* ppparent = pparent->getParent();
  if(ppparent == 0) return true;
  return false;
}

bool CClassView::isMethod(int index){
  KTreeListItem* current = itemAt(index);
  if(current == 0) return false;
  KTreeListItem* parent = current->getParent();
  if(parent == 0) return false;
  KTreeListItem* pparent = parent->getParent();
  if(pparent == 0) return false;  
  KTreeListItem* ppparent = pparent->getParent();
  if(ppparent == 0) return false;
  return true;
}

void CClassView::slotProjectOptions(){
  emit selectedProjectOptions();
}
void CClassView::slotFileNew(){
  emit selectedFileNew();
}
void CClassView::slotClassNew(){
  emit selectedClassNew();
}
void CClassView::slotClassRemove(){
}
void CClassView::slotClassDelete(){
}
void CClassView::slotMethodNew(){
}
void CClassView::slotVariableNew(){
}
void CClassView::slotViewDeclaration(){
  int index=currentItem();
  emit selectedViewDeclaration(index);
}
void CClassView::slotViewDefinition(){
  int index=currentItem();
  emit selectedViewDefinition(index);
}

void CClassView::CVFindTheClasses(){


  class_infos->clear();
  //cerr << endl <<"BEGINNE MIT DER ANALYSE";
  TClassInfo* class_info;
  int act_pos = 0;
  int pos1 =0;
  int num_classes=0; //number of classes in the actual file
  QRegExp regexp;
  
  QString stream; // the actual stream string
  TStreamedFile* stream_info; 
  if (streamed_files->isEmpty()) return; // no file, no class
  
  // scan all the streamed files
  for(stream_info = streamed_files->first();stream_info != 0;stream_info = streamed_files->next()){
    stream = stream_info->stream; // set the actual stream
    act_pos=0;
    // first find the classes
    num_classes = stream.contains("class ");
    // cerr << endl << "Gefunden KLASSEN in " << stream_info->filename << ": " << num_classes ;
    while(num_classes != 0){
      class_info = new TClassInfo;
      class_info->filename = stream_info->filename;
      class_info->type = "class";
      regexp = "class ";
      act_pos = stream.find(regexp,act_pos); // find the class token
      regexp = " [a-zA-Z]";
      act_pos = stream.find(regexp,act_pos) +1; // find the begin of the classname
      regexp = "[{;]";
      pos1 = stream.find(regexp, act_pos);    // test for forward declarations
      if ( stream.mid(pos1, 1) == "{" )
      {
        regexp = "[;{ \t\n]";
        pos1 = stream.find(regexp,act_pos); // find the end of the classname
        class_info->classname = stream.mid(act_pos,(pos1-act_pos)); // get the classname
        act_pos = stream.find('{',act_pos);
        class_info->begin = act_pos;
        class_info->end = CVFindClassDecEnd(stream,act_pos);
      

        //cerr << endl <<"KLASSENAME:" << class_info->classname << ":" << endl;
        //cerr << "Begin:" << class_info->begin << ":" << endl;
        //cerr << "Ende:" << class_info->end << ":" << endl;
        class_infos->append(class_info);
      }
      else
        act_pos = pos1;
      num_classes--; 
    }

    // and now the structures
    act_pos=0;
    num_classes = stream.contains("struct ");
    //cerr << endl << "Gefunden STRUCTUREN in " << stream_info->filename << ": " << num_classes ;
    while(num_classes != 0){
      class_info = new TClassInfo;
      class_info->filename = stream_info->filename;
      class_info->type = "struct";
      regexp = "struct ";
      act_pos = stream.find(regexp,act_pos); // find the struct token
      regexp = " [a-zA-Z]";
      act_pos = stream.find(regexp,act_pos) +1; // find the begin of the structname
      regexp = "[{;]";
      pos1 = stream.find(regexp, act_pos);     // test for forward declarations
      if ( stream.mid(pos1, 1) == "{" )
      {
        regexp = "[{ ]";
        pos1 = stream.find(regexp,act_pos); // find the end of the structname
        class_info->classname = stream.mid(act_pos,pos1-act_pos); // get the structname
        act_pos = stream.find('{',act_pos);
        class_info->begin = act_pos;
        class_info->end = CVFindClassDecEnd(stream,act_pos);
      
        //cerr << endl <<"STRUCTNAMENAME:" << class_info->classname << ":" << endl;
        //cerr << "Begin:" << class_info->begin << ":" << endl;
        //cerr << "Ende:" << class_info->end << ":" << endl;
        class_infos->append(class_info);
      }
      else
        act_pos = pos1;
      num_classes--; 
    }
    
  }
 
}

int CClassView::CVFindClassDecEnd(QString stream,int startpos){
  int num =1;
  int act_pos = startpos;
  while (num !=0){
    act_pos++;
    if(act_pos == (int)stream.size()){
      KDEBUG(KDEBUG_ERROR,CCLASSVIEW,"FIXME: Error by parsing,coundn't find a class-end definition");
      return act_pos;
    }
    if (stream[act_pos] == '{') num++;
    if (stream[act_pos] == '}') num--;
  }
  return act_pos;
}

void CClassView::CVFindTheMethodsAndVars(){
  int begin,end,act_pos,prev_pos;
  QString stream,str,name,temp;
  TClassInfo* class_info;
  TStreamedFile* streamed_file;
  TMethodInfo* method_info;
  TVarInfo* var_info;
  QString visibility;
  if(class_infos->isEmpty()) return; // no classes no Methods and no Vars
  
  // iterate all classes
  for(class_info = class_infos->first();class_info != 0;class_info = class_infos->next()){
    begin = class_info->begin;
    end = class_info->end;
    act_pos=begin;prev_pos=begin;

    for(streamed_file = streamed_files->first();streamed_file != 0;
	streamed_file = streamed_files->next()){
      if (streamed_file->filename == class_info->filename){
	stream = streamed_file->stream; // the correct stream for the class found
      }
    }
    
    if (class_info->type == "struct") {visibility = "public";}
    else {visibility = "private";}
    act_pos = stream.find(';',prev_pos+1);
    while((act_pos <= end) && act_pos != -1 ){
      
      str = stream.mid(prev_pos,act_pos-prev_pos);
      temp = CVGetVisibility(str);
      if (temp  != "nothing" ) {
	visibility = temp;
      }
      // get the methods
      if (CVIsItAMethod(str)){
	name = CVGetMethod(str);
	method_info = new TMethodInfo;
	method_info->name = name;
	method_info->visibility = visibility;
	class_info->method_infos.append(method_info); // a new method
	//	cerr << "METH:" << name;
	//cerr << "Visibility:" << visibility  << endl;
      }
      // get the variables
      if (CVIsItAVariable(str)){
	name = CVGetVariable(str);
	var_info = new TVarInfo;
	var_info->name = name;
	var_info->visibility = visibility;
	class_info->variable_infos.append(var_info); // a new method
	//	cerr << "VAR:" << name << endl;
	//	cerr << "Visibility:" << visibility  << endl;
      }
      
      prev_pos = act_pos; // and now the next please
      act_pos = stream.find(';',prev_pos+1);
    }
    
  }// end iterate all classes
}


bool CClassView::CVIsItAMethod(QString str){
  if ((str.contains(')') == 1) && str.contains('(') == 1) return true;
  return false;
}
bool CClassView::CVIsItAVariable(QString str){
  return (!CVIsItAMethod(str));
}
void CClassView::CVRemoveAllComments(QString* str){
  //  return;
  int begin;
  int end;
  int i=0;
  while ((begin = str->find("/*")) != -1){
    end = str->find("*/",begin+2);
    if(end == -1){
      end = str->length();
    }
    for(i=begin;i<=end;i++){
      if ((*str)[i] != '\n'){
	(*str)[i] = '_'; // remove the complete comment
      }
    }
  }
  while ((begin = str->find("//")) != -1){
    if (str->find("\n",begin) != -1){
      end = str->find("\n",begin);
    }
    else{
      end = str->find("\0", begin);
    }
    i=0;
    if(begin != 0){
      i = begin-1;
    }
    for(;i<=end;i++){
      if ((*str)[i] != '\n'){
	(*str)[i] = '_'; // remove the complete comment
      }
    }
  }
  // cerr << endl << *str;
}


QString CClassView::CVGetVisibility(QString str){
  QString visi_str;
  QRegExp regexp;
  int end = str.length()-1;
  int begin =-1;
  int len;
  end = str.findRev(':',end);
  if (end == -1) {
    //cerr << endl << "VIS: no change" << endl;
    return "nothing"; // no visibility change
    
  }
  regexp = "[a-zA-Z]";
  end = str.findRev(regexp,end);
  regexp = "[ \n]";
  begin = str.findRev(regexp,end);
  if (begin == -1 || end == -1){
    return "nothing";
  }
  len = end-begin;
  visi_str = str.mid(begin+1,len);
  visi_str = visi_str.stripWhiteSpace();
  
  
  if (visi_str == "signals") return "public";
  if (visi_str != "slots" && visi_str != "public" 
      && visi_str != "protected" && visi_str != "private"){
    return "nothing";
    
  }
  if (visi_str == "slots"){
    regexp = "[a-zA-Z]";
    end = str.findRev(regexp,begin);
    regexp = "[ \n]";
    begin = str.findRev(regexp,end);
    visi_str = str.mid(begin+1,end-begin);
    visi_str = visi_str.stripWhiteSpace(); 
  }
  return visi_str;
}
QString CClassView::CVGetMethod(QString str){
  int begin,end;
  QRegExp regexp;
  
  end = str.findRev(')',str.size()-1); // find the end
  begin = str.findRev('(',end);
  regexp = "[a-zA-Z]";
  begin = str.findRev(regexp,begin);
  regexp = "[ \t:;*]";
  begin = str.findRev(regexp,begin);
  return str.mid(begin+1,end-begin);
}
QString CClassView::CVGetVariable(QString str){
  int begin,end;
  QRegExp regexp;
  regexp = "[a-zA-Z]";
  end = str.findRev(regexp,str.size()-1); // find the last letter of the name
  regexp = "[ \t*]";
  begin = str.findRev(regexp,end); // find the first letter
  if((end-begin+1) <0){
    return "error";
  }
  return str.mid(begin+1,end-begin+1);
}


