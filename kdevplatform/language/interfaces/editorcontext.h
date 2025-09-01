/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.org>
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
