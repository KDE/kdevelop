/***************************************************************************
                             editorview.cpp
                             --------------
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


#include "editorview.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <ktexteditor.h>

#include <qlayout.h>
#include <qpainter.h>
#include <qpaintdevice.h>
#include <qpixmap.h>

const int borderWidth=19;

// ***************************************************************************

IconBorder::IconBorder( KTextEditor::Document *document, QWidget *parent, const char *name ) :
  QWidget(parent, name),
  m_buffer(borderWidth, document->numLines()*16/*document->getFontHeight()*/),
  m_addBP(KGlobal::iconLoader()->loadIcon("undo", KIcon::Small)),
  m_removeBP(borderWidth, 16/*document->getFontHeight()*/),
  m_scrollPos(0),
  m_docFontHeight(/*document->getFontHeight()*/16)
{
  setFixedWidth(borderWidth);

  // A different colour so we can debug this :-)
  m_buffer.fill ( QColor( 250, 250, 200) );
  m_removeBP.fill ( QColor( 250, 250, 200) );
}

// ***************************************************************************

IconBorder::~IconBorder()
{
}

// ***************************************************************************

QSizePolicy IconBorder::sizePolicy() const
{
  return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
}

// ***************************************************************************

void IconBorder::paintEvent( QPaintEvent * )
{
  QPainter p( this );
  p.drawPixmap(0, -m_scrollPos, m_buffer);
}

// ***************************************************************************

void IconBorder::slotScrollValueChanged(int value)
{
  kdDebug(9004) << "Scroll value = " << value << endl;
  m_scrollPos = value;
  paintEvent(0);
}

// ***************************************************************************

void IconBorder::addBreakpoint(int lineNum)
{
  bitBlt (&m_buffer, 0, lineNum*m_docFontHeight, &m_addBP);
  paintEvent(0);
}

// ***************************************************************************

// Assume one breakpoint per line.
void IconBorder::removeBreakpoint(int lineNum)
{
  bitBlt (&m_buffer, 0, lineNum*m_docFontHeight, &m_removeBP);
  paintEvent(0);
}

// ***************************************************************************

void IconBorder::mousePressEvent(QMouseEvent *e)
{
  m_cursorOnLine = ( e->y() + m_scrollPos ) / m_docFontHeight;

  if (e->button() == RightButton)
  {
    KPopupMenu menu( this );
    menu.insertItem(i18n("Toggle bookmark"),            this, SLOT(slotToggleBookmark()));
    menu.insertItem(i18n("Clear all bookmarks"),        this, SLOT(clearBookmarks()));
    menu.insertSeparator();
    menu.insertItem(i18n("Toggle breakpoint"),          this, SLOT(slotToggleBreakpoint()));
    menu.insertItem(i18n("Edit breakpoint"),            this, SLOT(slotEditBreakpoint()));
    menu.insertItem(i18n("Enable/disable breakpoint"),  this, SLOT(slotToggleEnableBreakpoint()));
    menu.insertItem(i18n("Clear all breakpoints"),      this, SIGNAL(clearAllBreakpoints()));
    menu.insertSeparator();
    menu.insertItem(i18n("LMB sets breakpoints"),       this, SLOT(slotLMBMenuToggle()));
    menu.insertItem(i18n("LMB sets bookmarks"),         this, SLOT(slotLMBMenuToggle()));
    menu.exec(QCursor::pos());
    return;
  }

  if (e->button() == LeftButton)
    slotToggleBreakpoint();
}

void IconBorder::slotToggleBookmark()
{
}

// ***************************************************************************

void IconBorder::clearBookmarks()
{
}

// ***************************************************************************

void IconBorder::slotToggleBreakpoint()
{
  emit toggleBreakpoint(m_cursorOnLine);
  // test code
//  addBreakpoint(m_cursorOnLine);
}

// ***************************************************************************

void IconBorder::slotEditBreakpoint()
{
}

// ***************************************************************************

void IconBorder::clearAllBreakpoints()
{
}

// ***************************************************************************

void IconBorder::slotLMBMenuToggle()
{
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

EditorView::EditorView( KTextEditor::Document *document, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
  QBoxLayout * l = new QHBoxLayout( this );
  l->setAutoAdd( true );

  IconBorder* lhborder = new IconBorder(document, this, name);

  KTextEditor::View * view = document->createView( this, name);
  connect ( view,     SIGNAL(scrollValueChanged(int)),
            lhborder, SLOT(slotScrollValueChanged(int)));
  view->setName(name);
  view->setInternalContextMenuEnabled(false);
}


EditorView::~EditorView()
{
}
