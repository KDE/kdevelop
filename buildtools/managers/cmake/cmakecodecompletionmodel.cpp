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
#include <duchain.h>
#include <duchainlock.h>
#include <ducontext.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include "astfactory.h"

using namespace KTextEditor;
using namespace KDevelop;

CMakeCodeCompletionModel::CMakeCodeCompletionModel(QObject *parent)
    : CodeCompletionModel(parent), m_commands(AstFactory::self()->commands())
{
}

void CMakeCodeCompletionModel::completionInvoked(View* view, const Range& range, InvocationType invocationType)
{
    int numRows = m_commands.count();
    m_declarations.clear();
    DUChainReadLocker lock(DUChain::lock());
    TopDUContext* ctx = DUChain::self()->chainForDocument( view->document()->url() );
    if(ctx ) {
        typedef QPair<Declaration*, int> DeclPair;
        foreach(const DeclPair& pair, ctx->allDeclarations( SimpleCursor(range.start()), ctx ))
            m_declarations.append(DeclarationPointer(pair.first));
        numRows+=m_declarations.count();
    }
    setRowCount(numRows);
    reset();
}

CMakeCodeCompletionModel::Type CMakeCodeCompletionModel::indexType(int row) const
{
    if(row<m_commands.count())
        return Command;
    else
        return VariableMacro;
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
        else if(type==VariableMacro)
        {
            int pos=index.row()-m_commands.count();
            DUChainReadLocker lock(DUChain::lock());
            return m_declarations[pos]->identifier().toString();
        }
    }
    else if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Prefix)
    {
        if(type==Command)
            return "Command";
        else if(type==VariableMacro)
        {
            int pos=index.row()-m_commands.count();
            DUChainReadLocker lock(DUChain::lock());
            FunctionType *func=dynamic_cast<FunctionType*>(m_declarations[pos]->abstractType().data());
            if(func)
                return "Macro";
            else
                return "Variable";
        }
    }
    else if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Arguments)
    {
        if(type==Command)
            return QString(); //TODO
        else if(type==VariableMacro)
        {
            DUChainReadLocker lock(DUChain::lock());
            int pos=index.row()-m_commands.count();
            FunctionType *func=dynamic_cast<FunctionType*>(m_declarations[pos]->abstractType().data());
            if(func)
            {
                QStringList args;
                foreach(const AbstractType::Ptr& t, func->arguments())
                {
                    const DelayedType *delay=dynamic_cast<const DelayedType*>(t.data());
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
        case VariableMacro: {
            int pos=row-m_commands.count();
            DUChainReadLocker lock(DUChain::lock());
            FunctionType *func=dynamic_cast<FunctionType*>(m_declarations[pos]->abstractType().data());
            if(func)
            {
                document->replaceText(word, data(index(row, Name, QModelIndex())).toString()+'(');
            }
            else
            {
                Range r=word, prefix(Cursor(word.start().line(), word.start().column()-2), word.start());
                QString bef=document->text(prefix);
                if(r.start().column()>=2 && bef=="${")
                    r.start().setColumn( r.start().column()-2 );
                document->replaceText(r, "${"+data(index(row, Name, QModelIndex())).toString()+'}');
            }
        }   break;
    }
}

