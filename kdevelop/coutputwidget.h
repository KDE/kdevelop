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
#include <qstack.h>
#include <qmap.h>


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
  void insertAtEnd(const char * s) { insertAtEnd(QString::fromLocal8Bit(s));};
  void insertAtEnd(const QCString & s) { insertAtEnd(QString::fromLocal8Bit(s.data()));};
};



class CMakeOutputWidget : public QMultiLineEdit
{
  Q_OBJECT

public:

  enum MakeOutputErrorType { Error, Diagnostic, Normal };
  class ErrorDetails
  {
    public:
    ErrorDetails(const QString& filename, int lineNumber, MakeOutputErrorType type) :
      m_fileName(filename), m_lineNumber(lineNumber), m_type(type) {};

    ErrorDetails() : m_fileName(QString::null), m_lineNumber(-1), m_type(Normal) {};

    QString m_fileName;
    int m_lineNumber;
    MakeOutputErrorType m_type;
  };

  CMakeOutputWidget(QWidget* parent, const char* name=0);
  ~CMakeOutputWidget() {};

  void insertAtEnd(const QString& s, MakeOutputErrorType defaultType=Normal);
  void insertAtEnd(const char * s, MakeOutputErrorType defaultType=Normal) { insertAtEnd(QString::fromLocal8Bit(s),defaultType);};
  void insertAtEnd(const QCString & s, MakeOutputErrorType defaultType=Normal) { insertAtEnd(QString::fromLocal8Bit(s.data()),defaultType);};

  void start();
  void viewNextError();
  void viewPreviousError();

signals:
  void switchToFile(const QString& filename, int lineNo);

private:
  void processLine(const QString& line, MakeOutputErrorType defaultType);
  void checkForError();
  MakeOutputErrorType lineType(int row);
  void selectLine(int line);

  // override from QMultiLineEdit
  void keyPressEvent( QKeyEvent* keyEvent );
  void mouseReleaseEvent(QMouseEvent* event);
#if (QT_VERSION < 300)
  void paintCell(QPainter* p, int row, int col);
  int mapToView( int xIndex, int line );
#else
  static bool isAboveQt303();
#endif

  QString m_buf;
  QStack<QString> m_dirStack;
  KRegExp m_dirChange;
  KRegExp m_errorGcc;

  typedef QMap<int, ErrorDetails> ErrorMap;
  ErrorMap m_errorMap;
  ErrorMap::Iterator it;
};

#endif
