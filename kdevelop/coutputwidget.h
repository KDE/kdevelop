/***************************************************************************
                     coutputwidget.h - the output window in kdevelop   
                             -------------------                                         

    version              :                                   
    begin                : 5 Aug 1998                                        
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
#ifndef COUTPUTWIDGET_H
#define COUTPUTWIDGET_H

#include <kregexp.h>

#include <qmultilineedit.h>
#include <qpalette.h>
#include <qlistbox.h>
#include <qstack.h>


/** the view for the compiler and tools-output
  *@author Sandy Meier
  */
class COutputWidget : public QMultiLineEdit
{
  Q_OBJECT

public:
  /**contructor*/
  COutputWidget(QWidget* parent, const char* name=0);
  /**destructor*/
  virtual ~COutputWidget(){};

  void insertAtEnd(const QString& s);

protected:
  void mouseReleaseEvent(QMouseEvent* event);
  void keyPressEvent ( QKeyEvent* event);
  
signals:
  /** emited, if the mouse was clicked over the widget*/
  void clicked();
  void keyPressed(int key);
};

class MakeOutputItem : public QListBoxText
{
public:
  enum Type { Diagnostic, Normal, Error };
  MakeOutputItem(const QString &text, Type type, const QString& filename=QString::null, int lineNo=-1);
  QString filename() const    { return m_filename; }
  int lineNo() const          { return m_lineNo; }

private:
  virtual void paint(QPainter *p);

  Type m_type;
  QString m_filename;
  int m_lineNo;
};

class CMakeOutputWidget : public QListBox
{
  Q_OBJECT

public:
  CMakeOutputWidget(QWidget* parent, const char* name=0);
  ~CMakeOutputWidget() {};

  void insertAtEnd(const QString& s);

//  void setCursorPosition (int, int);
//  QString textLine (int);
//  void cursorPosition (int *, int *);
//  QString text() { return QString::null; }
  void start();
  void viewNextError();
  void viewPreviousError();

protected:
//  void mouseReleaseEvent(QMouseEvent* event);
//  void keyPressEvent ( QKeyEvent* event);

signals:
  /** emited, if the mouse was clicked over the widget*/
//  void clicked();
//  void keyPressed(int key);
  void switchToFile(const QString& filename, int lineNo);

public slots:
  void slotClicked (QListBoxItem* item);

private:
  void processLine(const QString& line);

  QString buf;
  QStack<QString> dirStack;
  KRegExp enterDir;
  KRegExp leaveDir;
  KRegExp errorGcc;
  KRegExp errorJade;
};

#endif
