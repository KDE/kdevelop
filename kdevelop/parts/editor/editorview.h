/***************************************************************************
                             editorview.h
                             ------------
    copyright            : (C) 2000 by KDevelop team
    email                : kdevelop_team@kdevelop.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _EDITORVIEW_H_
#define _EDITORVIEW_H_

namespace KTextEditor {
  class Document;
};

#include <qwidget.h>
#include <qpixmap.h>

class QPixmap;

class IconBorder : public QWidget
{
  Q_OBJECT

public:
  IconBorder( KTextEditor::Document *document, QWidget *parent=0, const char *name=0 );
  ~IconBorder();

  QSizePolicy sizePolicy() const;
  void addBreakpoint(int lineNum);
  void removeBreakpoint(int lineNum);

private slots:
  void slotScrollValueChanged(int value);
	void slotToggleBookmark();
	void clearBookmarks();
  void slotToggleBreakpoint();
	void slotEditBreakpoint();
  void clearAllBreakpoints();
  void slotLMBMenuToggle();

protected:
  void paintEvent( QPaintEvent * );
  void mousePressEvent(QMouseEvent *e);

signals:
  void toggleBreakpoint(int onLine);

private:
  QPixmap m_buffer;
  QPixmap m_addBP;
  QPixmap m_removeBP;
  int m_scrollPos;
  int m_docFontHeight;
  int m_cursorOnLine;
};



class EditorView : public QWidget
{
    Q_OBJECT

public:
    EditorView( KTextEditor::Document *document, QWidget *parent=0, const char *name=0 );
    ~EditorView();
};


#endif
