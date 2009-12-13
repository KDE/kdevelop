/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "snippetcompletionmodel.h"
#include <ktexteditor/document.h>
#include "snippetstore.h"
#include "snippetrepository.h"
#include "snippet.h"
#include "snippetcompletionitem.h"

SnippetCompletionModel::SnippetCompletionModel()
    : KTextEditor::CodeCompletionModel(0)
{
    initData();
    setHasGroups(false);
}

SnippetCompletionModel::~SnippetCompletionModel()
{
}

QVariant SnippetCompletionModel::data( const QModelIndex& idx, int role ) const
{
    if( !idx.isValid() || idx.row() < 0 || idx.row() >= rowCount() ) {
        return QVariant();
    } else {
        return m_snippets.at( idx.row() )->data(idx, role, 0);
    }
}

void SnippetCompletionModel::executeCompletionItem( KTextEditor::Document* doc, const KTextEditor::Range& w, int row ) const
{
    m_snippets.at( row )->execute(doc, w);
}

void SnippetCompletionModel::completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType invocationType)
{
    Q_UNUSED( view );
    Q_UNUSED( range );
    Q_UNUSED( invocationType );
    initData();
}

void SnippetCompletionModel::initData()
{
    m_snippets.clear();
    SnippetStore* store = SnippetStore::instance();
    for(int i = 0; i < store->rowCount(); i++ )
    {
        SnippetRepository* repo = dynamic_cast<SnippetRepository*>( store->item( i, 0 ) );
        if( repo )
        {
            foreach (Snippet* snippet, repo->getSnippets()) {
                m_snippets << new SnippetCompletionItem(snippet->text(), snippet->getSnippetPlainText());
            }
        }
    }
    reset();
    setRowCount( m_snippets.count() );
}

#include "snippetcompletionmodel.moc"
