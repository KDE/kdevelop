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

 //#include <keditcl.h>
 #include <kapp.h>
 #include <kspell.h>

#include "./kwrite/kwview.h"
/** an abstraction layer for an editwidget
  *@author Sandy Meier
  */
class CEditWidget : public KWrite {
  Q_OBJECT
public:
  CEditWidget(KApplication* a=0,QWidget* parent=0,char* name=0);
  ~CEditWidget();
  void setName(QString filename);
  void setText(QString &text);
  void setFocus();
  int loadFile(QString filename, int mode);
  void doSave();
  void doSave(QString filename);
  void copyText();
  void gotoPos(int pos,QString text);
  void toggleModified(bool);
  void search();
  void searchAgain();
  void replace();
  void gotoLine();
  void indent();
  void unIndent();
  void invertSelection();
  void deselectAll();

public slots:
	void spellcheck();
	void spellcheck2(KSpell*);
	
public: // Public queries
  QString markedText();
  QString getName();
  QString text();

  /** Returns the number of lines in the text. */
  uint lines();

public: // Method to manipulate the buffer
  /** Insert the string at the supplied line. */
  void insertAtLine( const char *toInsert, uint atLine );
  
  /** Append a text at the end of the file. */
  void append( const char *toAdd );

protected:
//  QString filename;
  QPopupMenu* pop;
  QString searchtext;
  void enterEvent ( QEvent * e); 
  void mousePressEvent(QMouseEvent* event);
 protected slots:
 void slotLookUp();
  signals:
 void  lookUp(QString text);
 void  bufferMenu(const QPoint&);

private:
	KSpell *kspell;
	KSpellConfig *ksc;
	int spell_offset;
};

#endif







