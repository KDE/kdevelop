/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.org>
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "editorcontext.h"

#include <KTextEditor/Cursor>
#include <KTextEditor/Document>
#include <KTextEditor/View>

namespace KDevelop {
class EditorContextPrivate
{
public:
    EditorContextPrivate(KTextEditor::View* view, const KTextEditor::Cursor& position)
        : m_url(view->document()->url())
        , m_position(position)
        , m_currentLine(view->document()->line(m_position.line()))
        , m_view(view)
    {
        int wordStart = m_position.column();
        int wordEnd = m_position.column();
        while (wordStart > 0 && wordStart < m_currentLine.length() &&
               (m_currentLine[wordStart - 1].isLetterOrNumber() || m_currentLine[wordStart - 1] == QLatin1Char('_')))
            --wordStart;
        while (wordEnd >= 0 && wordEnd < m_currentLine.length() &&
               (m_currentLine[wordEnd].isLetterOrNumber() || m_currentLine[wordEnd] == QLatin1Char('_')))
            ++wordEnd;
    }

    QUrl m_url;
    KTextEditor::Cursor m_position;
    QString m_currentLine, m_currentWord;
    KTextEditor::View* m_view;
};

EditorContext::EditorContext(KTextEditor::View* view, const KTextEditor::Cursor& position)
    : DeclarationContext(view, position)
    , d_ptr(new EditorContextPrivate(view, position))
{}

EditorContext::~EditorContext() = default;

int EditorContext::type() const
{
    return Context::EditorContext;
}

QUrl EditorContext::url() const
{
    Q_D(const EditorContext);

    return d->m_url;
}

QList<QUrl> EditorContext::urls() const
{
    Q_D(const EditorContext);

    return {
               d->m_url
    };
}

KTextEditor::Cursor EditorContext::position() const
{
    Q_D(const EditorContext);

    return d->m_position;
}

QString EditorContext::currentLine() const
{
    Q_D(const EditorContext);

    return d->m_currentLine;
}

QString EditorContext::currentWord() const
{
    Q_D(const EditorContext);

    return d->m_currentWord;
}

KTextEditor::View* EditorContext::view() const
{
    Q_D(const EditorContext);

    return d->m_view;
}
}
