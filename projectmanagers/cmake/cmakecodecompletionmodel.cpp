/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakecodecompletionmodel.h"
#include <QVariant>
#include <QModelIndex>
#include <kurl.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/delayedtype.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include "astfactory.h"

using namespace KTextEditor;
using namespace KDevelop;

CMakeCodeCompletionModel::CMakeCodeCompletionModel(QObject *parent)
    : CodeCompletionModel(parent), m_commands(AstFactory::self()->commands())
{
}

bool isFunction(const Declaration* decl)
{
    return decl->abstractType().cast<FunctionType>();
}

void CMakeCodeCompletionModel::completionInvoked(View* view, const Range& range, InvocationType invocationType)
{
    m_declarations.clear();
    DUChainReadLocker lock(DUChain::lock());
    KTextEditor::Document* d=view->document();
    TopDUContext* ctx = DUChain::self()->chainForDocument( d->url() );
    
    KTextEditor::Cursor pos=range.end(), step(0,1);
    m_inside=false;
    for(QChar i=d->character(pos); pos.column()>0 && !m_inside; i=d->character(pos-=step))
    { m_inside = (i=='('); }
    
    int numRows = m_inside? 0 : m_commands.count();
    m_varCount=0;
    if(ctx)
    {
        typedef QPair<Declaration*, int> DeclPair;
        QList<DeclPair> list=ctx->allDeclarations( SimpleCursor(range.start()), ctx );
        
        m_varCount=0;
        foreach(const DeclPair& pair, list)
        {
            Declaration *d=pair.first;
            bool func=isFunction(d);
            if((func && !m_inside) || (!func && m_inside))
                m_declarations.append(d);
        }
        
        numRows+=m_declarations.count();
    }
    setRowCount(numRows);
    reset();
}

CMakeCodeCompletionModel::Type CMakeCodeCompletionModel::indexType(int row) const
{
    if(m_inside)
        return Variable;
    else
    {
        if(row<m_commands.count())
            return Command;
        else
            return Macro;
    }
}

QVariant CMakeCodeCompletionModel::data (const QModelIndex & index, int role) const
{
    if(!index.isValid())
        return QVariant();
    Type type=indexType(index.row());

    if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Name)
    {
        if(type==Command)
            return m_commands[index.row()];
        else if(type==Variable || type==Macro)
        {
            int pos = (type==Variable) ? index.row() : index.row()-m_commands.count();
            DUChainReadLocker lock(DUChain::lock());
            return m_declarations[pos].data()->identifier().toString();
        }
    }
    else if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Prefix)
    {
        switch(type)
        {
            case Command:   return "Command";
            case Variable:  return "Variable";
            case Macro:     return "Macro";
        }
    }
    else if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Arguments)
    {
        switch(type) {
            case Variable:
            case Command:
                break;
            case Macro:
            {
                DUChainReadLocker lock(DUChain::lock());
                int pos=index.row()-m_commands.count();
                AbstractType::Ptr type; Q_ASSERT(type = m_declarations[pos].data()->abstractType());
                FunctionType::Ptr func; Q_ASSERT(func = type.cast<FunctionType>());
                
                QStringList args;
                foreach(const AbstractType::Ptr& t, func->arguments())
                {
                    DelayedType::Ptr delay = t.cast<DelayedType>();
                    args.append(delay->identifier().toString());
                }
                return '('+args.join(", ")+')';
            }
        }
        
    }
    return QVariant();
}

void CMakeCodeCompletionModel::executeCompletionItem(Document* document, const Range& word, int row) const
{
    switch(indexType(row))
    {
        case Command:
            document->replaceText(word, data(index(row, Name, QModelIndex())).toString()+'(');
            break;
        case Macro: {
            int pos=row-m_commands.count();
            DUChainReadLocker lock(DUChain::lock());
            Declaration* decl = m_declarations[pos].data();
            if(!decl)
                return;
            AbstractType::Ptr type; Q_ASSERT(type = m_declarations[pos].data()->abstractType());
            FunctionType::Ptr func; Q_ASSERT(func = type.cast<FunctionType>());
            document->replaceText(word, data(index(row, Name, QModelIndex())).toString()+'(');
        }   break;
        case Variable: {
            DUChainReadLocker lock(DUChain::lock());
            Range r=word, prefix(Cursor(word.start().line(), word.start().column()-2), word.start());
            QString bef=document->text(prefix);
            if(r.start().column()>=2 && bef=="${")
                r.start().setColumn( r.start().column()-2 );
            document->replaceText(r, "${"+data(index(row, Name, QModelIndex())).toString()+'}');
        }   break;
    }
}

