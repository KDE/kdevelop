/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakecodecompletionmodel.h"
#include <QVariant>
#include <QModelIndex>
#include <QMimeDatabase>
#include <QUrl>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/delayedtype.h>
#include <util/stringviewhelpers.h>

#include <cmakeduchaintypes.h>
#include "cmakeutils.h"
#include "icmakedocumentation.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KIO/Global>
#include <KLocalizedString>

using namespace KTextEditor;
using namespace KDevelop;

QVector<QString> CMakeCodeCompletionModel::s_commands;

CMakeCodeCompletionModel::CMakeCodeCompletionModel(QObject* parent)
    : CodeCompletionModel(parent)
    , m_inside(false)
{}

bool isFunction(const Declaration* decl)
{
    return decl->abstractType().dynamicCast<FunctionType>();
}

bool isPathChar(QChar c)
{
    return c.isLetterOrNumber() || c == QLatin1Char('/') || c == QLatin1Char('.');
}

QString escapePath(QString path)
{
    // see https://cmake.org/Wiki/CMake/Language_Syntax#Escapes
    const QString toBeEscaped = QStringLiteral("\"()#$^");

    for(const QChar &ch : toBeEscaped) {
        path.replace(ch, QLatin1Char('\\') + ch);
    }
    return path;
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
    m_inside=line.lastIndexOf(QLatin1Char('('), range.end().column()-line.size()-1)>=0;
    
    for(int l=range.end().line(); l>=0 && !m_inside; --l)
    {
        const auto lineString = d->line(l);
        const auto line = leftOfNeedleOrEntireView(lineString, QLatin1Char{'#'});

        int close=line.lastIndexOf(QLatin1Char(')')), open=line.indexOf(QLatin1Char('('));
        
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

        const auto toCompleteString = d->text(Range(start, range.end() - Cursor(0, 1)));
        const QStringView toComplete = toCompleteString;

        const auto lastdir = toComplete.lastIndexOf(QLatin1Char{'/'});
        QString path = KIO::upUrl(QUrl(d->url())).adjusted(QUrl::StripTrailingSlash).toLocalFile()+QLatin1Char('/');
        if(lastdir>=0) {
            const auto basePath = toComplete.first(lastdir);
            path += basePath;
        }
        QDir dir(path);

        const auto paths = dir.entryInfoList(QStringList(toComplete.sliced(lastdir + 1) + QLatin1Char{'*'}),
                                             QDir::AllEntries | QDir::NoDotAndDotDot);
        m_paths.clear();
        m_paths.reserve(paths.size());
        for (const QFileInfo& f : paths) {
            QString currentPath = f.fileName();
            if(f.isDir())
                currentPath += QLatin1Char('/');
            m_paths += currentPath;
        }
        
        numRows += m_paths.count();
    } else
        numRows += s_commands.count();
    
    if(ctx)
    {
        const auto list = ctx->allDeclarations( ctx->transformToLocalRevision(KTextEditor::Cursor(range.start())), ctx );

        for (const auto& pair : list) {
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
            case Command:   return i18nc("@item", "Command");
            case Variable:  return i18nc("@item", "Variable");
            case Macro:     return i18nc("@item", "Macro");
            case Path:      return i18nc("@item", "Path");
            case Target:    return i18nc("@item", "Target");
        }
    }
    else if(role==Qt::DecorationRole && index.column()==CodeCompletionModel::Icon)
    {
        switch(type)
        {
            case Command:   return QIcon::fromTheme(QStringLiteral("code-block"));
            case Variable:  return QIcon::fromTheme(QStringLiteral("code-variable"));
            case Macro:     return QIcon::fromTheme(QStringLiteral("code-function"));
            case Target:    return QIcon::fromTheme(QStringLiteral("system-run"));
            case Path: {
                QUrl url = QUrl::fromUserInput(m_paths[index.row()-m_declarations.size()]);
                QString iconName;
                if (url.isLocalFile()) {
                    // don't read contents even if it is a local file
                    iconName = QMimeDatabase().mimeTypeForFile(url.toLocalFile(), QMimeDatabase::MatchExtension).iconName();
                }
                else {
                    // remote always only looks at the extension
                    iconName = QMimeDatabase().mimeTypeForUrl(url).iconName();
                }
                return QIcon::fromTheme(iconName);
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
                    func = m_declarations[pos].data()->abstractType().dynamicCast<FunctionType>();

                if(!func)
                    return QVariant();

                QStringList args;
                const auto arguments = func->arguments();
                args.reserve(arguments.size());
                for (const AbstractType::Ptr& t : arguments) {
                    auto delay = t.dynamicCast<DelayedType>();
                    args.append(delay ? delay->identifier().toString() : i18n("wrong"));
                }
                return QString(QLatin1Char('(') + args.join(QLatin1String(", ")) + QLatin1Char(')'));
            }
        }
        
    }
    return QVariant();
}

void CMakeCodeCompletionModel::executeCompletionItem(View* view, const Range& word, const QModelIndex& idx) const
{
    Document* document = view->document();
    const int row = idx.row();
    switch(indexType(row))
    {
        case Path: {
            Range r=word;
            for (QChar c=document->characterAt(r.end()); c.isLetterOrNumber() || c==QLatin1Char('.'); c=document->characterAt(r.end())) {
                r.setEnd(KTextEditor::Cursor(r.end().line(), r.end().column()+1));
            }
            QString path = data(index(row, Name, QModelIndex())).toString();

            document->replaceText(r, escapePath(path));
        }   break;
        case Macro:
        case Command: {
            QString code=data(index(row, Name, QModelIndex())).toString();
            if (!document->line(word.start().line()).contains(QLatin1Char('(')))
                code.append(QLatin1Char('('));
            
            document->replaceText(word, code);
        }   break;
        case Variable: {
            Range r=word, prefix(word.start()-Cursor(0,2), word.start());
            QString bef=document->text(prefix);
            if(r.start().column()>=2 && bef==QLatin1String("${"))
                r.setStart(KTextEditor::Cursor(r.start().line(), r.start().column()-2));
            QString code = QLatin1String("${")+data(index(row, Name, QModelIndex())).toString();
            if(document->characterAt(word.end())!=QLatin1Char('}'))
                code += QLatin1Char('}');
            
            document->replaceText(r, code);
        }   break;
        case Target:
            document->replaceText(word, data(index(row, Name, QModelIndex())).toString());
            break;
    }
}

#include "moc_cmakecodecompletionmodel.cpp"
