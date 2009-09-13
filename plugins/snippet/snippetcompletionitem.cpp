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

#include "snippet.h"

SnippetCompletionItem::SnippetCompletionItem( const QString& name, const QString& snippet )
    : CompletionTreeItem(), m_name(name), m_snippet(snippet)
{
}

QVariant SnippetCompletionItem::data( const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model ) const
{
    // as long as the snippet completion model is not a kdevelop code completion model,
    // model will usually be 0. hence don't use it.
    Q_UNUSED(model);
    if ( role == Qt::DisplayRole ) {
        if ( index.column() == KTextEditor::CodeCompletionModel::Name ) {
            return m_name;
        }
    }
    return QVariant();
}

void SnippetCompletionItem::execute( KTextEditor::Document* document, const KTextEditor::Range& word )
{
    document->replaceText( word, m_snippet );
}
