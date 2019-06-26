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

class QUrl;

namespace KDevelop {
class EditorContextPrivate;

/**A context for the KTextEditor.*/
class KDEVPLATFORMLANGUAGE_EXPORT EditorContext
    : public DeclarationContext
{
public:
    /**
     * Builds a context for a KTextEditor part.
     * @param view The view for the editor context.
     * @param position The cursor position.
     */
    EditorContext(KTextEditor::View* view, const KTextEditor::Cursor& position);

    /**Destructor.*/
    ~EditorContext() override;

    EditorContext(const EditorContext&) = delete;
    EditorContext& operator=(const EditorContext&) = delete;

    int type() const override;

    /**@return The url for the file which this context was invoked for.*/
    QUrl url() const;
    QList<QUrl> urls() const override;

    /**@return The cursor position.*/
    KTextEditor::Cursor position() const;

    /**@return A QString with the content of the line which this context was
        invoked for.*/
    QString currentLine() const;

    /**@return A QString containing the word near to the cursor when this
        context object was created.*/
    QString currentWord() const;

    /**
     * Returns the associated view.
     */
    KTextEditor::View* view() const;

private:
    const QScopedPointer<class EditorContextPrivate> d_ptr;
    Q_DECLARE_PRIVATE(EditorContext)
};
}

#endif
