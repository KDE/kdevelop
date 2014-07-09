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
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include "astfactory.h"
#include <cmakeduchaintypes.h>
#include "cmakeutils.h"
#include "icmakedocumentation.h"

#include <KMimeType>

using namespace KTextEditor;
using namespace KDevelop;

QStringList CMakeCodeCompletionModel::s_commands;

CMakeCodeCompletionModel::CMakeCodeCompletionModel(QObject* parent)
    : CodeCompletionModel(parent)
    , m_inside(false)
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
    beginResetModel();
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
        QString line=cline.left(cline.indexOf('#'));
        
        int close=line.lastIndexOf(')'), open=line.indexOf('(');
        
        if(close>=0 && open>=0) {
            m_inside=open>close;
            break;
        } else if(open>=0) {
            m_inside=true;
            break;
        } else if(close>=0) {
            m_inside=false;
            break;
        }
    }
    
    int numRows = 0;
    if(m_inside) {
        Cursor start=range.start();
        for(; isPathChar(d->characterAt(start)); start-=Cursor(0,1))
        {}
        start+=Cursor(0,1);
        
        QString tocomplete=d->text(Range(start, range.end()-Cursor(0,1)));
        
        int lastdir=tocomplete.lastIndexOf('/');
        QString path = KUrl(d->url()).upUrl().path(KUrl::AddTrailingSlash);
        QString basePath;
        if(lastdir>=0) {
            basePath=tocomplete.mid(0, lastdir);
            path+=basePath;
        }
        QDir dir(path);
        
        QFileInfoList paths=dir.entryInfoList(QStringList() << tocomplete.mid(lastdir+1)+'*',
                                              QDir::AllEntries | QDir::NoDotAndDotDot);
        m_paths.clear();
        foreach(const QFileInfo& f, paths) {
            QString currentPath = f.fileName();
            if(f.isDir())
                currentPath+='/';
            m_paths += currentPath;
        }
        
        numRows += m_paths.count();
    } else
        numRows += s_commands.count();
    
    if(ctx)
    {
        typedef QPair<Declaration*, int> DeclPair;
        QList<DeclPair> list=ctx->allDeclarations( ctx->transformToLocalRevision(KTextEditor::Cursor(range.start())), ctx );
        
        foreach(const DeclPair& pair, list)
        {
            bool func=isFunction(pair.first);
            if((func && !m_inside) || (!func && m_inside))
                m_declarations.append(pair.first);
        }
        
        numRows+=m_declarations.count();
    }
    setRowCount(numRows);
    endResetModel();
}

CMakeCodeCompletionModel::Type CMakeCodeCompletionModel::indexType(int row) const
{
    if(m_inside)
    {
        if(row < m_declarations.count()) {
            KDevelop::DUChainReadLocker lock;
            Declaration* dec = m_declarations.at(row).declaration();
            if (dec && dec->type<TargetType>())
                return Target;
            else
                return Variable;
        } else
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
        int pos = index.row();
        switch(type) {
            case Command:
                return s_commands[pos-m_declarations.size()];
            case Path:
                return m_paths[pos-m_declarations.size()];
            case Target:
            case Variable:
            case Macro: {
                DUChainReadLocker lock(DUChain::lock());
                Declaration* dec=m_declarations[pos].data();
                
                return dec ? dec->identifier().toString() : i18n("INVALID");
            }
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
            case Target:      return i18n("Target");
        }
    }
    else if(role==Qt::DecorationRole && index.column()==CodeCompletionModel::Icon)
    {
        switch(type)
        {
            case Command:   return QIcon::fromTheme("code-block");
            case Variable:  return QIcon::fromTheme("code-variable");
            case Macro:     return QIcon::fromTheme("code-function");
            case Target:    return QIcon::fromTheme("system-run");
            case Path: {
                QString url = m_paths[index.row()-m_declarations.size()];
                return QIcon::fromTheme(KMimeType::findByUrl(url, 0, false, true)->iconName());
            }
        }
    }
    else if(role==Qt::DisplayRole && index.column()==CodeCompletionModel::Arguments)
    {
        switch(type) {
            case Variable:
            case Command:
            case Path:
            case Target:
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
                    args.append(delay ? delay->identifier().toString() : i18n("wrong"));
                }
                return QString('('+args.join(", ")+')');
            }
        }
        
    }
    return QVariant();
}

void CMakeCodeCompletionModel::executeCompletionItem(Document* document, const Range& word, int row) const
{
    switch(indexType(row))
    {
        case Path: {
            Range r=word;
            for(QChar c=document->characterAt(r.end()); c.isLetterOrNumber() || c=='.'; c=document->characterAt(r.end())) {
                r.setEnd(KTextEditor::Cursor(r.end().line(), r.end().column()+1));
            }
            document->replaceText(r, data(index(row, Name, QModelIndex())).toString());
        }   break;
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
                r.setStart(KTextEditor::Cursor(r.start().line(), r.start().column()-2));
            QString code="${"+data(index(row, Name, QModelIndex())).toString();
            if(document->characterAt(word.end())!='}')
                code+='}';
            
            document->replaceText(r, code);
        }   break;
        case Target:
            document->replaceText(word, data(index(row, Name, QModelIndex())).toString());
            break;
    }
}

