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
#include <KLocalizedString>
#include <KIcon>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include "astfactory.h"
#include "cmakeutils.h"
#include "icmakedocumentation.h"

#include <KMimeType>

using namespace KTextEditor;
using namespace KDevelop;

QStringList CMakeCodeCompletionModel::s_commands;

CMakeCodeCompletionModel::CMakeCodeCompletionModel(QObject* parent)
    : CodeCompletionModel(parent)
{}

bool isFunction(const Declaration* decl)
{
    return decl->abstractType().cast<FunctionType>();
}

bool isPathChar(const QChar& c)
{
    return c.isLetterOrNumber() || c=='/' || c=='.';
}

void CMakeCodeCompletionModel::completionInvoked(View* view, const Range& range, InvocationType invocationType)
{
    if(s_commands.isEmpty()) {
        ICMakeDocumentation* cmakedoc=CMake::cmakeDocumentation();
        
        if(cmakedoc)
            s_commands=cmakedoc->names(ICMakeDocumentation::Command);
    }
    
    Q_UNUSED(invocationType);
    m_declarations.clear();
    DUChainReadLocker lock(DUChain::lock());
    KTextEditor::Document* d=view->document();
    TopDUContext* ctx = DUChain::self()->chainForDocument( d->url() );
    
    QString line=d->line(range.end().line());
//     m_inside=line.lastIndexOf('(', range.end().column())>=0;
    m_inside=line.lastIndexOf('(', range.end().column()-line.size()-1)>=0;
    
    for(int l=range.end().line(); l>=0 && !m_inside; --l)
    {
        QString cline=d->line(l);
        QString line=cline.mid(0, cline.indexOf('#'));
        
        bool out=line.lastIndexOf(')')>=0;
        m_inside= !out && line.indexOf('(')>=0;
    }
    
    int numRows = 0;
    if(m_inside) {
        Cursor start=range.start();
        for(; isPathChar(d->character(start)); start-=Cursor(0,1))
        {}
        start+=Cursor(0,1);
        
        QString tocomplete=d->text(Range(start, range.end()-Cursor(0,1)));
        
        int lastdir=tocomplete.lastIndexOf('/');
        QString path=d->url().upUrl().path(KUrl::AddTrailingSlash);
        QString basePath;
        if(lastdir>=0) {
            basePath=tocomplete.mid(0, lastdir);
            path+=basePath;
        }
        QDir dir(path);
        
        m_paths=dir.entryList(QStringList() << tocomplete.mid(lastdir+1)+'*',
                                          QDir::AllEntries | QDir::NoDotAndDotDot);
        
        numRows += m_paths.count();
    } else
        numRows += s_commands.count();
    
    if(ctx)
    {
        typedef QPair<Declaration*, int> DeclPair;
        QList<DeclPair> list=ctx->allDeclarations( ctx->transformToLocalRevision(SimpleCursor(range.start())), ctx );
        
        foreach(const DeclPair& pair, list)
        {
            bool func=isFunction(pair.first);
            if((func && !m_inside) || (!func && m_inside))
                m_declarations.append(pair.first);
        }
        
        numRows+=m_declarations.count();
    }
    setRowCount(numRows);
    reset();
}

CMakeCodeCompletionModel::Type CMakeCodeCompletionModel::indexType(int row) const
{
    if(m_inside)
    {
        if(row<m_declarations.count())
            return Variable;
        else
            return Path;
    }
    else
    {
        if(row<m_declarations.count())
            return Macro;
        else
            return Command;
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
            return s_commands[index.row()-m_declarations.size()];
        else if(type==Path)
            return m_paths[index.row()-m_declarations.size()];
        else if(type==Variable || type==Macro)
        {
            int pos = index.row();
            DUChainReadLocker lock(DUChain::lock());
            Declaration* dec=m_declarations[pos].data();
            if(dec)
                return dec->identifier().toString();
            else
                return i18n("INVALID");
        }
    }
    else if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Prefix)
    {
        switch(type)
        {
            case Command:   return i18n("Command");
            case Variable:  return i18n("Variable");
            case Macro:     return i18n("Macro");
            case Path:      return i18n("Path");
        }
    }
    else if(role==Qt::DecorationRole && index.column()==CodeCompletionModel::Icon)
    {
        switch(type)
        {
            case Command:   return KIcon("code-block");
            case Variable:  return KIcon("code-variable");
            case Macro:     return KIcon("code-function");
            case Path: {
                QString url = m_paths[index.row()-m_declarations.size()];
                return KIcon(KMimeType::findByUrl(url, 0, false, true)->iconName(url));
            }
        }
    }
    else if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Arguments)
    {
        switch(type) {
            case Variable:
            case Command:
            case Path:
                break;
            case Macro:
            {
                DUChainReadLocker lock(DUChain::lock());
                int pos=index.row();

                FunctionType::Ptr func;
                
                if(m_declarations[pos].data())
                    func = m_declarations[pos].data()->abstractType().cast<FunctionType>();
                
                if(!func)
                    return QVariant();

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
        case Path:
            document->replaceText(word, data(index(row, Name, QModelIndex())).toString());
            break;
        case Macro:
        case Command: {
            QString code=data(index(row, Name, QModelIndex())).toString();
            if(!document->line(word.start().line()).contains('('))
                code.append('(');
            
            document->replaceText(word, code);
        }   break;
        case Variable: {
            Range r=word, prefix(word.start()-Cursor(0,2), word.start());
            QString bef=document->text(prefix);
            if(r.start().column()>=2 && bef=="${")
                r.start().setColumn( r.start().column()-2 );
            QString code="${"+data(index(row, Name, QModelIndex())).toString();
            if(document->character(word.end())!='}')
                code+='}';
            
            document->replaceText(r, code);
        }   break;
    }
}

