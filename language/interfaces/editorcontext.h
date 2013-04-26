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

#ifndef KDEVPLATFORM_EDITORCONTEXT_H
#define KDEVPLATFORM_EDITORCONTEXT_H

#include "codecontext.h"

namespace KTextEditor {
    class View;
    class Cursor;
}

class KUrl;

namespace KDevelop {

/**A context for the KTextEditor.*/
class KDEVPLATFORMLANGUAGE_EXPORT EditorContext: public DeclarationContext
{
public:
    /**Builds a context for a KTextEditor part.
        @param url The url of a file in the editor.
        @param position The position where the cursor is.
        @param linestr The content of the line where the cursor is.
        @param wordstr The current word under the cursor.*/
    EditorContext( KTextEditor::View*, KTextEditor::Cursor position );

    /**Destructor.*/
    virtual ~EditorContext();

    virtual int type() const;

    /**@return The url for the file which this context was invoked for.*/
    KUrl url() const;

    /**@return The cursor position.*/
    KTextEditor::Cursor position() const;

    /**@return A QString with the content of the line which this context was
        invoked for.*/
    QString currentLine() const;

    /**@return A QString containing the word near to the cursor when this
        context object was created.*/
    QString currentWord() const;

    KTextEditor::View* view() const;

private:
    class EditorContextPrivate* const d;

    EditorContext( const EditorContext & );
    EditorContext &operator=( const EditorContext & );
};

}

#endif

