/**
 * This file is part of KDevelop
 *
 * Copyright 2009 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "snippetcompletionitem.h"

#include <KTextEditor/Document>
#include <KLocalizedString>
#include <QtGui/QTextEdit>

#include <language/codecompletion/codecompletionmodel.h>

#include "snippet.h"
#include "selectionjumper.h"

SnippetCompletionItem::SnippetCompletionItem( const QString& name, const QString& snippet )
    : CompletionTreeItem(), m_name(name), m_snippet(snippet)
{
}

SnippetCompletionItem::~SnippetCompletionItem()
{
}

QVariant SnippetCompletionItem::data( const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model ) const
{
    // as long as the snippet completion model is not a kdevelop code completion model,
    // model will usually be 0. hence don't use it.
    Q_UNUSED(model);

    switch ( role ) {
    case Qt::DisplayRole:
        if ( index.column() == KTextEditor::CodeCompletionModel::Name ) {
            return m_name;
        } else if ( index.column() == KTextEditor::CodeCompletionModel::Prefix ) {
            return i18n("Snippet");
        }
        break;
    case KDevelop::CodeCompletionModel::IsExpandable:
        return QVariant(true);
    case KDevelop::CodeCompletionModel::ExpandingWidget:
        {
        QTextEdit *textEdit = new QTextEdit();
        ///TODO: somehow make it possible to scroll like in other expanding widgets
        // don't make it too large, only show a few lines
        textEdit->resize(textEdit->width(), 100);
        textEdit->setPlainText(m_snippet);
        textEdit->setReadOnly(true);
        textEdit->setLineWrapMode(QTextEdit::NoWrap);

        QVariant v;
        v.setValue<QWidget*>(textEdit);
        return v;
        }
    }

    return QVariant();
}

void SnippetCompletionItem::execute( KTextEditor::Document* document, const KTextEditor::Range& word )
{
    // get indendation of the current line
    QString indendation;
    QString currentLine = document->line(word.start().line());
    for ( int i = 0; i < currentLine.size(); ++i ) {
        if ( currentLine[i].isSpace() ) {
            indendation += currentLine[i];
        } else {
            break;
        }
    }

    // parse snippet for markers
    QList<KTextEditor::Range> markers;
    bool isEscaped = false;
    int line = word.start().line();
    int column = word.start().column();
    KTextEditor::Cursor start = KTextEditor::Cursor::invalid();
    QList<int> charsToRemove;
    for ( int i = 0; i < m_snippet.size(); ++i ) {
        if ( m_snippet[i] == '\\' ) {
            ++column;
            isEscaped = !isEscaped;
        } else if ( m_snippet[i] == '\n' ) {
            ++line;
            column = indendation.size();
        } else if ( m_snippet[i] == '$' ) {
            if ( isEscaped ) {
                charsToRemove << i - 1;
                isEscaped = false;
            } else {
                charsToRemove << i;
                if ( start.isValid() ) {
                    KTextEditor::Cursor end(line, column);
                    markers << KTextEditor::Range(start, end);
                    start = KTextEditor::Cursor::invalid();
                } else {
                    start = KTextEditor::Cursor(line, column);
                }
            }
        } else {
            isEscaped = false;
            ++column;
        }
    }
    // remove $'s
    int deletedChars = 0;
    foreach ( const int &pos, charsToRemove ) {
        m_snippet.remove(pos - deletedChars, 1 );
        ++deletedChars;
    }
    // indent
    m_snippet.replace('\n', '\n' + indendation);

    document->replaceText( word, m_snippet );

    if ( !markers.isEmpty() ) {
        new SelectionJumper(markers, document);
    }
}
