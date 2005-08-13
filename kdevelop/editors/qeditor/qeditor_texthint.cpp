/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "qeditor_texthint.h"
#include "qeditor.h"
#include "qeditor_view.h"

#include <private/qrichtext_p.h>
#include <klocale.h>

QEditorTextHint::QEditorTextHint( QEditorView* editorView )
   : QToolTip( editorView->editor() ), m_editorView( editorView )
{
}

QEditorTextHint::~QEditorTextHint()
{
}

void QEditorTextHint::maybeTip( const QPoint& pt )
{
    QEditor* ed = m_editorView->editor();
    QPoint contentsPt = ed->viewportToContents( pt );

    QTextCursor c;
    c.place( contentsPt, ed->document()->firstParagraph() );

    int line = c.paragraph()->paragId();
    int column = c.index();

    QString textHint = m_editorView->computeTextHint( line, column );

    if( !textHint.isNull() )
        tip( QRect(pt, QSize(100,30)), textHint );
}

