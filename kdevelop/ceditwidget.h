/***************************************************************************
              ceditwidget.h  -  an abstraction layer for an editwidget   
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

#ifndef CEDITWIDGET_H
#define CEDITWIDGET_H

#include <kapp.h>
#include <kspell.h>
#include "./kwrite/kwview.h"

/** An abstraction layer for the editwidget.
  *@author Sandy Meier
  */
class CEditWidget : public KWrite {
  Q_OBJECT

public: // Constructor and destructor

  CEditWidget(KApplication* a=0,QWidget* parent=0,char* name=0);
  ~CEditWidget();

public: // Public methods to set attribute values

  /** Set the filename.
   * @param filename The new filename.
   */
  void setName(QString filename)   { KWrite::setFileName(filename); }

  /** Make the edit view get the input focus. */
  void setFocus();

  /** Discard old text without warning and set new text
   * @param text The new text.
   */
  void setText(QString &text)      { KWrite::setText(text); }

  /** Sets the modification status of the document. */
  void toggleModified(bool mod)    { KWrite::setModified(mod); }

public: // File related method

  /** Loads the file given in name into the editor.
   * @param filename Name of the file to load. 
   * @param mode Current not used.
   * @return The line at which the file got loaded.
   */
  int loadFile(QString filename, int mode);

  /** Save the editbuffer to the current file. */
  void doSave()                    { KWrite::save(); }

  /** Save the editbuffer to a file with the supplied filename.
   * @param filename Name of the file to save the buffer to.
   */
  void doSave(QString filename)   { KWrite::writeFile(filename); }

public: // Cursor placement methods

  /** Goto the start of the line that contains the specified position.
   * @param pos Position to go to
   * @param text The editwidget text.
   */
  void gotoPos(int pos, QString text);

  /** Presents a "Goto Line" dialog to the user. */
  void gotoLine()                 { KWrite::gotoLine(); }

public: // Selection related methods
  
  /** Copies the marked text into the clipboard. */
  void copyText()                 { KWrite::copy(); }

  /** Inverts the current selection. */
  void invertSelection()          { KWrite::invertSelection(); }

  /** Deselects all text. */
  void deselectAll()              { KWrite::deselectAll(); }

public: // Search and replace methods

  /** Presents a search dialog to the user. */
  void search()                   { KWrite::find(); }

  /** Repeats the last search or replace operation. 
   * On replace, the user is prompted even if the "Prompt On Replace" 
   * option was off.
   */
  void searchAgain()              { KWrite::findAgain(); }

  /** Presents a replace dialog to the user. */
  void replace()                  { KWrite::replace(); }

public: // Indent methods

  /** Moves the current line or the selection one position to the 
   * right
   */
  void indent()                   { KWrite::indent(); }

  /** Moves the current line or the selection one position to the 
   * left
   */
  void unIndent()                 { KWrite::unIndent(); }

public: // Queries

  /** Return the popupmenu. 
   * @return Pointer to the current popupmenu.
   */
  QPopupMenu* popup()             { return pop; };

  /** Gets the marked text as string. 
   * @return The marked text as a string.
   */
  QString markedText()            { return KWrite::markedText(); }
  
  /** Get the current filename. 
   * @return The current filename.
   */
  QString getName();

  /** Gets the complete document content as string. 
   * @return A string containing the whole document.
   */
  QString text()                  { return KWrite::text(); }

  /** Returns the number of lines in the text. 
   * @return Number of lines in the text.
   */
  uint lines()                    { return (uint) numLines();}//(text().contains('\n', false)); }
	
public slots:
 void spellcheck();
 void spellcheck2(KSpell*);
 void slotGrepText();
		
public: // Method to manipulate the buffer

  /** Delete an interval of lines.
   * @param startAt Line to start deleting at.
   * @param endAt Line to stop deleting at.
   */
  void deleteInterval( uint startAt, uint endAt );

  /** Insert the string at the supplied line. 
   * @param toInsert Text to insert.
   * @param atLine Line to start inserting the text.
   */
  void insertAtLine( const char *toInsert, uint atLine );
  
  /** Append a text at the end of the file. 
   * @param toAdd Text to append.
   */
  void append( const char *toAdd );

protected: // Protected attributes

  /** Current popupmenu. */
  QPopupMenu* pop;

  /** Current searchtext. */
  QString searchtext;

protected: // Protected methods
  void enterEvent ( QEvent * e); 
  void mousePressEvent(QMouseEvent* event);

  /** Get the startposition in the buffer of a line
   * @param buf Buffer to search in.
   * @param aLine Linenumber to find position to.
   * @return The 0-based bufferposition or -1 if not found.
   */
  int getLinePos( const char *buf, uint aLine );

protected slots:
 void slotLookUp();

signals:
 void lookUp(QString text);
 void grepText(QString text);
 void bufferMenu(const QPoint&);

private:
 KSpell *kspell;
 KSpellConfig *ksc;
 int spell_offset;
};

#endif


