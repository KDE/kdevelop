/* This file is part of the KDE libraries
   Copyright (C) 2001 Harald Fernengel <harry@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "document.h"
#include "view.h"
#include "kneditfactory.h"

#include <qlayout.h>
#include <qlabel.h>

#include <qxembed.h>
#include <kdebug.h>


namespace KNEdit
{

View::View ( Document *doc, QWidget *parent, const char *name ) : KTextEditor::View (doc, parent, name)
{
  setInstance( KNEditFactory::instance() );
  
  doc->addView( this );
  m_doc = doc;
  setFocusPolicy( StrongFocus );

  QVBoxLayout *vbl = new QVBoxLayout( this );

  QLabel* lbl = new QLabel( "embedding of NEdit not implemented yet. Use the NEdit window that is floating around somewhere ;)", this );
  vbl->addWidget( lbl );
}

View::~View ()
{
  if ( m_doc )
    m_doc->removeView( this ); 
}

KTextEditor::Document *View::document() const
{
  return m_doc;
}

void View::embedNEdit( WId id )
{
  kdDebug() << "embedding: " << id << endl;

  QXEmbed* embed = new QXEmbed( this, "embedded NEdit" );
  embed->embed( id );
  if ( layout() ) {
    layout()->add( embed );
  }
}


QPoint View::cursorCoordinates ()
{
  // TODO
  return QPoint();
}

void View::cursorPosition (uint *line, uint *col)
{
  // TODO
}

void View::cursorPositionReal (uint *line, uint *col)
{
  // TODO
}

bool View::setCursorPosition (uint line, uint col)
{
  Q_ASSERT( m_doc );
  m_doc->invokeNC("-line " + QString::number(line + 1), true);
  return true;
}

bool View::setCursorPositionReal (uint line, uint col)
{
  return setCursorPosition(line, col);
}

uint View::cursorLine ()
{
  // TODO
  return 0;
}

uint View::cursorColumn ()
{
  // TODO
  return 0;
}

uint View::cursorColumnReal ()
{
  // TODO
  return 0;
}

};

#include "view.moc"

