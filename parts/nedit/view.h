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

#ifndef _KNEDIT_VIEW_INCLUDE_
#define _KNEDIT_VIEW_INCLUDE_

#include <qpoint.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/dynwordwrapinterface.h>

namespace KNEdit
{

class Document;

/**
  The KNedit::View text editor interface.
*/
class View : public KTextEditor::View, KTextEditor::ViewCursorInterface
{
  Q_OBJECT

  public:
    View ( Document *, QWidget *, const char * = 0 );
    virtual ~View ();

    void embedNEdit( WId id );

    // KTextEditor stuff
    KTextEditor::Document *document() const;

  public slots:
    /** Get the current cursor coordinates in pixels. */
    QPoint cursorCoordinates ();

    /** Get the cursor position */
    void cursorPosition (uint *line, uint *col);

    /** Get the cursor position, calculated with 1 character per tab */
    void cursorPositionReal (uint *line, uint *col);

    /** Set the cursor position */
    bool setCursorPosition (uint line, uint col);

    /** Set the cursor position, use 1 character per tab */
    bool setCursorPositionReal (uint line, uint col);

    uint cursorLine ();
    uint cursorColumn ();
    uint cursorColumnReal ();

  signals:
    void cursorPositionChanged ();

  private:
    Document* m_doc;
};

};

#endif
