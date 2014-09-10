/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.org>
Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "editorcontext.h"
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

namespace KDevelop {

class EditorContextPrivate
{
public:
    EditorContextPrivate( KTextEditor::View* view, KTextEditor::Cursor position )
            : m_view( view )
    {
        m_url = view->document()->url();
        m_position = position;
        m_currentLine = view->document()->line(m_position.line());
        int wordStart = m_position.column();
        int wordEnd = m_position.column();
        while(wordStart > 0 && wordStart < m_currentLine.length() && (m_currentLine[wordStart-1].isLetterOrNumber() || m_currentLine[wordStart-1] == '_'))
            --wordStart;
        while(wordEnd >= 0 && wordEnd < m_currentLine.length() && (m_currentLine[wordEnd].isLetterOrNumber() || m_currentLine[wordEnd] == '_'))
            ++wordEnd;
    }

    QUrl m_url;
    KTextEditor::Cursor m_position;
    QString m_currentLine, m_currentWord;
    KTextEditor::View* m_view;
};

EditorContext::EditorContext( KTextEditor::View* view, KTextEditor::Cursor position )
        : DeclarationContext( view, position ), d( new EditorContextPrivate( view, position ) )
{}

EditorContext::~EditorContext()
{
    delete d;
}

int EditorContext::type() const
{
    return Context::EditorContext;
}

QUrl EditorContext::url() const
{
    return d->m_url;
}

KTextEditor::Cursor EditorContext::position() const
{
    return d->m_position;
}

QString EditorContext::currentLine() const
{
    return d->m_currentLine;
}

QString EditorContext::currentWord() const
{
    return d->m_currentWord;
}

KTextEditor::View* EditorContext::view() const
{
    return d->m_view;
}
}

