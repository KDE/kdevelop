/***************************************************************************
               ceditwidget.cpp  -  a abstraction layer for an editwidget   
                             -------------------                                         

    begin                : 23 Aug 1998                                        
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

#include <sys/stat.h>
#include <unistd.h>
#include <iostream.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qclipboard.h>
#include <qregexp.h>
#include <assert.h>
#include <klocale.h>
#include <kiconloader.h>
#include "./kwrite/kwdoc.h"
#include "./kwrite/highlight.h"
#include "resource.h"
#include "ceditwidget.h"


// We don't want to clutter the global namespace with this stuff.
class CEditWidgetPrivate
{
public:
    dev_t dev;
    ino_t ino;
    time_t mtime;
};


HlManager hlManager; //highlight manager

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CEditWidget::CEditWidget()
 * CEditWidget()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CEditWidget::CEditWidget(QWidget* parent,const char* name)
  : KWrite(new KWriteDoc(&hlManager),parent,name)
{  
   
    setFocusProxy (kWriteView); 
  pop = new QPopupMenu();
  //  pop->insertItem(i18n("Open: "),this,SLOT(open()),0,6);
  pop->insertItem(BarIcon("undo"),i18n("Undo"),this,SLOT(undo()),0,ID_EDIT_UNDO);
  pop->insertItem(BarIcon("redo"),i18n("Redo"),this,SLOT(redo()),0,ID_EDIT_REDO);
  pop->insertSeparator();
  pop->insertItem(BarIcon("cut"),i18n("Cut"),this,SLOT(cut()),0,ID_EDIT_CUT);
  pop->insertItem(BarIcon("copy"),i18n("Copy"),this,SLOT(copy()),0,ID_EDIT_COPY);
  pop->insertItem(BarIcon("paste"),i18n("Paste"),this,SLOT(paste()),0,ID_EDIT_PASTE);
  pop->setItemEnabled(ID_EDIT_CUT,false);
  pop->setItemEnabled(ID_EDIT_COPY,false);
  pop->setItemEnabled(ID_EDIT_PASTE,false);
  pop->insertSeparator();
  pop->insertItem(BarIcon("grep"),"",this,SLOT(slotGrepText()),0,ID_EDIT_SEARCH_IN_FILES);
  pop->insertItem(BarIcon("lookup"),"",this,SLOT(slotLookUp()),0,ID_HELP_SEARCH_TEXT);
  
  
  bookmarks.setAutoDelete(true);
  d = new CEditWidgetPrivate();
  
}

CEditWidget::CEditWidget(QWidget* parent,const char* name,KWriteDoc* doc,CEditWidgetPrivate* data,QPopupMenu* shared_pop)
  : KWrite(doc,parent,name)
{  
    setFocusProxy (kWriteView); 
    bookmarks.setAutoDelete(true);
    d = data;
    pop = shared_pop;
}

/*-------------------------------------- CEditWidget::~CEditWidget()
 * ~CEditWidget()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CEditWidget::~CEditWidget() {
    //  delete doc();
    //  delete d;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------------- CEditWidget::getName()
 * getName()
 *   Get the current filename.
 *
 * Parameters:
 *   -
 * Returns:
 *   QString      The current filename.
 *-----------------------------------------------------------------*/
QString CEditWidget::getName(){
  //return filename;
  QString s(KWrite::fileName());
  if (s.isNull()) s = "";
  return s;//QString(KWrite::fileName());
}

/*--------------------------------------------- CEditWidget::loadFile()
 * loadFile()
 *   Loads the file given in name into the editor.
 *
 * Parameters:
 *   filename     Name of the file to load.
 *   mode         Current not used.
 *
 * Returns:
 *   int          The line at which the file got loaded.
 *-----------------------------------------------------------------*/
int CEditWidget::loadFile(QString filename, int) {
  struct stat buf;
  KWrite::loadFile(filename);
  ::stat(filename, &buf);
  d->ino = buf.st_ino;
  d->dev = buf.st_dev;
  d->mtime = buf.st_mtime;
  return 0;
}

void CEditWidget::doSave() {
  struct stat buf;
  KWrite::save();
  ::stat(KWrite::fileName(), &buf);
  d->mtime = buf.st_mtime;
}

bool CEditWidget::modifiedOnDisk() {
  struct stat buf;
  ::stat(KWrite::fileName(), &buf);
  return buf.st_mtime != d->mtime;
}

bool CEditWidget::isEditing(const QString &filename)
{
  struct stat buf;
  ::stat(filename, &buf);
  return (buf.st_dev == d->dev) && (buf.st_ino == d->ino);
}


/*------------------------------------------- CEditWidget::setFocus()
 * setFocus()
 *   Make the edit view get the input focus.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CEditWidget::setFocus(){
    cerr << "CEditWidget::setFocus()\n";
//    KWrite::setFocus();
//    kWriteView->setFocus();
}

/*--------------------------------------------- CEditWidget::gotoPos()
 * gotoPos()
 *   Goto the start of the line that contains the specified position.
 *
 * Parameters:
 *   pos          Position to go to
 *   text         The editwidget text.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CEditWidget::gotoPos(int pos,QString text_str){
  
  //  cerr << endl << "POS: " << pos;
  // calculate the line
  QString last_textpart = text_str.right(text_str.length()+1-pos); // the second part of the next,after the pos
  int line = text_str.contains("\n") - last_textpart.contains("\n");
  //  cerr << endl << "LINE:" << line;
  setCursorPosition(line,0);
  setFocus();
}

void CEditWidget::spellcheck(){
   kspell= new KSpell (this, "KDevelop: Spellcheck", this,
		   SLOT (spellcheck2 (KSpell *)));	
}

void CEditWidget::spellcheck2(KSpell *){

//	setReadOnly (TRUE);

/*	connect (kspell, SIGNAL (misspelling (char *, QStrList *, unsigned)),
		 this, SLOT (misspelling (char *, QStrList *, unsigned)));
	connect (kspell, SIGNAL (corrected (char *,
						 char *, unsigned)),
		 this, SLOT (corrected (char *,
					char *, unsigned)));

       connect (kspell, SIGNAL (progress (unsigned int)),
                this, SIGNAL (spellcheck_progress (unsigned int)) );

	connect (kspell, SIGNAL (done(char *)),
		 this, SLOT (spellResult (char *)));
	
	kspell->setProgressResolution (2);
*/
	kspell->check (text().data());
	
}

/*------------------------------------- CEditWidget::deleteInterval()
 * deleteInterval()
 *   Delete an interval of lines.
 *
 * Parameters:
 *   startAt       Line to start deleting at.
 *   endAt         Line to end deleting at.
 *  
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CEditWidget::deleteInterval( uint startAt, uint endAt )
{
  assert( startAt >= endAt );

  int startPos=0;
  int endPos=0;
  QString txt;
  
  // Fetch the text.
  txt = text();

  // Find start position.
  startPos = getLinePos( txt, startAt );

  // We find the end of the line by increasing the linecounter and 
  // subtracting by one to make pos point at the last \n character.
  endPos = getLinePos( txt, endAt + 1 );
  endPos--;

  // Remove the interval.
  txt.remove( startPos, endPos - startPos );

  // Update the editwidget with the new text.
  setText( txt );

  // Set the buffer as modified.
  toggleModified( true );
}

/*------------------------------------- CEditWidget::deleteInterval()
 * deleteInterval()
 *   Insert the string at the given line.
 *
 * Parameters:
 *   toInsert      Text to insert.
 *   atLine        Line to start inserting the text.
 *  
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CEditWidget::insertAtLine( const char *toInsert, uint atLine )
{
  assert( toInsert != NULL );
  
  int pos=0;
  QString txt;
  
  // Fetch the text.
  txt = text();

  // Find the position to insert at.
  pos = getLinePos( txt, atLine );
  
  // Insert the new text.
  txt.insert( pos, toInsert );

  // Update the editwidget with the new text.
  setText( txt );

  // Set the buffer as modified.
  toggleModified( true );
}

/*---------------------------------------------- CEditWidget::append()
 * append()
 *   Append a text at the end of the file.
 *
 * Parameters:
 *   toAdd         Text to append.
 *  
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CEditWidget::append( const char *toAdd )
{
  QString txt;
  
  txt = text();
  txt.append( toAdd );
  setText( txt );
}

/*********************************************************************
 *                                                                   *
 *                          PROTECTED METHODS                        *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ CEditWidget::getLinePos()
 * getLinePos()
 *   Get the startposition in the buffer of a line
 *
 * Parameters:
 *   buf        Buffer to search in.
 *   aLine      Linenumber to find position to.
 *
 * Returns:
 *   int        The 0-based bufferposition or -1 if not found.
 *-----------------------------------------------------------------*/
int CEditWidget::getLinePos( const char *buf, uint aLine )
{
  uint line=0;
  int pos=0;

  while( line < aLine )
  {
    pos++;
    if( buf[ pos ] == '\n' )
      line++;
  }
  
  // Pos currently points at the last \n, that's why we add 1.
  return pos + 1;
}

void CEditWidget::enterEvent ( QEvent * e){
    KWrite::enterEvent(e);
    //  setFocus();
}

void CEditWidget::mousePressEvent(QMouseEvent* event){

  if(event->button() == RightButton){
    
    if(event->state() & ControlButton) {
      emit bufferMenu(this->mapToGlobal(event->pos()));
      return;
    }
    
    int state;
    int pos;
    state = undoState();
    //undo
    if(state & 1){
      pop->setItemEnabled(ID_EDIT_UNDO,true);
    }
    else{
      pop->setItemEnabled(ID_EDIT_UNDO,false);
    }
    //redo
    if(state & 2){
      pop->setItemEnabled(ID_EDIT_REDO,true);
    }
    else{
      pop->setItemEnabled(ID_EDIT_REDO,false);
    }
    
    QString str = markedText();
    if(!str.isEmpty()){
      pop->setItemEnabled(ID_EDIT_CUT,true);
      pop->setItemEnabled(ID_EDIT_COPY,true);
    }
    else{
      pop->setItemEnabled(ID_EDIT_CUT,false);
      pop->setItemEnabled(ID_EDIT_COPY,false);
    }		


    QClipboard *cb = kapp->clipboard();
    QString text=cb->text();
    if(text.isEmpty())
      pop->setItemEnabled(ID_EDIT_PASTE,false);
    else
      pop->setItemEnabled(ID_EDIT_PASTE,true);
    
    if(str == ""){
      str = word(event->x(),event->y());
    }

    searchtext = str;
    str.replace(QRegExp("^\n"), "");
    pos=str.find("\n");
    if (pos>-1)
     str=str.left(pos);

    if(str.length() > 20 ){
      str = str.left(20) + "...";
    }

    pop->setItemEnabled(ID_HELP_SEARCH_TEXT, !str.isEmpty());
    pop->setItemEnabled(ID_EDIT_SEARCH_IN_FILES, !str.isEmpty());

    pop->changeItem(BarIcon("grep"),i18n("grep: ") + str,ID_EDIT_SEARCH_IN_FILES); // the grep entry
    pop->changeItem(BarIcon("lookup"),i18n("look up: ") + str,ID_HELP_SEARCH_TEXT); // the lookup entry

    pop->popup(this->mapToGlobal(event->pos()));
  }
  setFocus();
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CEditWidget::slotLookUp(){
    emit lookUp(searchtext);
}
void CEditWidget::slotGrepText(){
    emit grepText(searchtext);
}

