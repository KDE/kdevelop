/* KDevelop CMake Support
 *
 * Copyright 2007-2008 Aleix Pol <aleixpol@gmail.com>
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

#include "cmakeprojectvisitor.h"
#include "cmakeast.h"
#include "cmakecondition.h"
#include "astfactory.h"

#include <simplerange.h>
#include <topducontext.h>
#include <duchain.h>
#include <dumpchain.h>
#include <duchainlock.h>
#include <parsingenvironment.h>
#include <typesystem.h>

#include <KProcess>
#include <KDebug>
#include <QHash>
#include <QFile>
#include <QDir>
#include <QtCore/qglobal.h>
#include <QByteArray>
#include <QRegExp>
#include <QFileInfo>
#include <QScriptEngine>
#include <QScriptValue>

using namespace KDevelop;

CMakeProjectVisitor::CMakeProjectVisitor(const QString& root, TopDUContext *parent)
    : m_root(root), m_defaultPaths(QStringList("/usr/lib/") << "/usr/include"), m_topctx(0), m_parentCtx(parent)
{
}

QStringList CMakeProjectVisitor::envVarDirectories(const QString &varName)
{
    QString env;
    QString start=QString("%1=").arg(varName);
    foreach(const QString& iEnv, QProcess::systemEnvironment())
    {
        if(iEnv.startsWith(start))
        {
            env=iEnv;
            break;
        }
    }
//     kDebug(9042) << ".......resolving env:" << varName << "=" << QProcess::systemEnvironment() << env;
    if(!env.isEmpty())
    {
        QChar separator;
#ifdef Q_OS_WIN
        separator = ';';
#else
        separator = ':';
#endif
        kDebug(9042) << "resolving env:" << varName << "=" << env;
        return env.split('=')[1].split(separator);
    }
    else
    {
        kDebug(9032) << "error:" << varName << " not found";
        return QStringList();
    }
}

QString CMakeProjectVisitor::variableName(const QString &exp, VariableType &type, int &before, int &after)
{
    QString name;
    type=NoVar;
    const int count=exp.count();
    bool done=false;
    int prev=-1;
    before=0; //TODO: remove me
    
    for(int i=before; i<count && !done; i++)
    {
        const QChar& expi=exp[i];
        if(expi=='{')
            prev=i;
        else if(i>0 && prev>0 && expi=='}')
        {
            done=true;
            name = exp.mid(prev+1, i-prev-1);
            after=i;
            if(exp[prev-1]=='$')
            {
                before=prev-1;
                type=CMake;
            }
            else if(exp.mid(prev-4,4)=="$ENV")
            {
                before=prev-4;
                type=ENV;
            }
        }
    }
    return name;
}

QStringList CMakeProjectVisitor::resolveVariable(const QString &exp, const VariableMap *values)
{
    VariableType type;
    int before=0, after;
    QString var = variableName(exp, type, before, after);
    
    if(type)
    {
        QStringList vars;
        if(type==CMake)
        {
            vars = values->value(var);
        }
        else
        {
            vars=envVarDirectories(var);
        }
        QString pre=exp.left(before), post=exp.right(exp.length()-after-1);
        if(vars.isEmpty())
            return QStringList(pre+post);
        vars.first().prepend(pre);
        vars.last().append(post);
        QStringList::iterator it=vars.begin(), itEnd=vars.end();
        QStringList ret;
        
        for(; it!=itEnd; ++it)
        {
            ret += resolveVariable(*it, values);
        }
        return ret;
    }
    return QStringList(exp);
}

int CMakeProjectVisitor::notImplemented(const QString &name) const
{
    kDebug(9042) << "not implemented!" << name;
    return 1;
}

int CMakeProjectVisitor::visit(const CMakeAst *ast)
{
    kDebug(9042) << "error! function not implemented" << ast->content()[ast->line()].name;
    return 1;
}

int CMakeProjectVisitor::visit(const ProjectAst *project)
{
    m_projectName = project->projectName();
    if(!m_vars->contains("CMAKE_PROJECT_NAME"))
        m_vars->insert("CMAKE_PROJECT_NAME", QStringList(project->projectName()));

    m_vars->insert("PROJECT_NAME", QStringList(project->projectName()));
    m_vars->insert("PROJECT_SOURCE_DIR", QStringList(m_root));
    m_vars->insert(QString("%1_SOURCE_DIR").arg(m_projectName), QStringList(m_root));
    return 1;
}

int CMakeProjectVisitor::visit(const AddSubdirectoryAst *subd)
{
    kDebug(9042) << "adding subdirectory" << subd->sourceDir();
    m_subdirectories += subd->sourceDir();
    return 1;
}

int CMakeProjectVisitor::visit(const SubdirsAst *sdirs)
{
    kDebug(9042) << "adding subdirectory" << sdirs->directories() << sdirs->exluceFromAll();
    m_subdirectories += sdirs->directories() << sdirs->exluceFromAll();
    return 1;
}

int CMakeProjectVisitor::visit(const AddExecutableAst *exec)
{
//     QString name = resolveVariable(exec->executable(), m_vars).join(";");
    VisitorState p=m_backtrace.front();
    DUChainWriteLocker lock(DUChain::lock());
    Declaration *d = new Declaration(p.context->url(), p.code->at(p.line).arguments.first().range(), Declaration::GlobalScope, p.context);
    d->setIdentifier( Identifier(exec->executable()) );
    m_declarationsPerTarget.insert(exec->executable(), d);
    //kDebug(9042) << "looooooool" << d
    //    << p.code->at(p.line).writeBack() << p.code->at(p.line).filePath << ':' << p.line;

    m_filesPerTarget.insert(exec->executable(), exec->sourceLists());
    kDebug(9042) << "exec:" << exec->executable() << "->" << m_filesPerTarget[exec->executable()]
        << "was" << exec->content()[exec->line()].writeBack();
    return 1;
}

int CMakeProjectVisitor::visit(const AddLibraryAst *lib)
{
    VisitorState p=m_backtrace.front();
    DUChainWriteLocker lock(DUChain::lock());
    Declaration *d = new Declaration(p.context->url(), p.code->at(p.line).arguments.first().range(), Declaration::GlobalScope, p.context);
    d->setIdentifier( Identifier(lib->libraryName()) );
    m_declarationsPerTarget.insert(lib->libraryName(), d);
    
    m_filesPerTarget.insert(lib->libraryName(), lib->sourceLists());
    kDebug(9042) << "lib:" << lib->libraryName();
    return 1;
}

int CMakeProjectVisitor::visit(const SetAst *set)
{
    //FIXME: Must deal with ENV{something} case
    m_vars->insert(set->variableName(), set->values());
    kDebug(9042) << "set:" << set->variableName() << "=" << m_vars->value(set->variableName()) << "...";
    return 1;
}

int CMakeProjectVisitor::visit(const IncludeDirectoriesAst * dirs)
{
    kDebug(9042) << "adding include directories" << dirs->includedDirectories();
    IncludeDirectoriesAst::IncludeType t = dirs->includeType();

    QStringList toInclude = dirs->includedDirectories();

    if(t==IncludeDirectoriesAst::DEFAULT)
    {
        if(m_vars->contains("CMAKE_INCLUDE_DIRECTORIES_BEFORE") && m_vars->value("CMAKE_INCLUDE_DIRECTORIES_BEFORE")[0]=="ON")
            t = IncludeDirectoriesAst::BEFORE;
        else
            t = IncludeDirectoriesAst::AFTER;
    }

    if(t==IncludeDirectoriesAst::AFTER)
        m_includeDirectories += toInclude;
    else
        m_includeDirectories = toInclude + m_includeDirectories;
    kDebug(9042) << "done." << m_includeDirectories;
    return 1;
}

QString CMakeProjectVisitor::findFile(const QString &file, const QStringList &folders, const QStringList& suffixes, FileType t)
{
    if( file.isEmpty() || QFileInfo(file).isAbsolute() )
         return file;
    
    QString filename;
    switch(t) {
        case Library:
#ifdef Q_OS_WIN
            filename=QString("%1.dll").arg(file);
#else
            filename=QString("lib%1.so").arg(file);
#endif
            break;
        case Executable:
#ifdef Q_OS_WIN
            kDebug(9042) << "Setting file extension to .exe";   
            filename=file+".exe";
            break;
#else
            kDebug(9042) << "NOT Setting file extension";
            filename=file;
            break;
#endif
        case Location:
            filename=file;
            break;
        case File:
            filename=file;
            break;
    }
    
    QStringList suffixFolders, useSuffixes(suffixes);
    useSuffixes.prepend(QString());
    foreach(const QString& apath, folders)
    {
        foreach(const QString& suffix, useSuffixes)
        {
            suffixFolders.append(apath+'/'+suffix);
        }
    }
    
    KUrl path;
    foreach(const QString& mpath, suffixFolders)
    {
        if(mpath.isEmpty())
            continue;

        KUrl file(mpath);
        file.addPath(filename);
        kDebug(9042) << "Trying:" << mpath << '.' << filename << file;
#if 0
// #ifndef Q_OS_WIN
        if(t==Library)
        {
            QDir direc(mpath);
            QStringList entries=direc.entryList(QStringList(filename+"*"));
            kDebug(9042) << "lib entries" << entries.count() << mpath << filename;
            if(!entries.isEmpty())
            {
                path=KUrl(mpath);
                path.addPath(entries.first());
                break;
            }
        }
        else
#endif
        if(QFile::exists(file.toLocalFile()))
        {
            switch(t) {
                case Location:
                    path=mpath;
                    break;
                case Executable:    //TODO: Must check if it is an executable.
                case File:
                case Library:
                    path=file;
                    break;
            }
            break;
        }
    }
    kDebug(9042) << "find file" << filename << "into:" << folders << "found at:" << path;
    return path.toLocalFile(KUrl::RemoveTrailingSlash);
}

int CMakeProjectVisitor::visit(const IncludeAst *inc)
{
    const QStringList modulePath = m_vars->value("CMAKE_MODULE_PATH") + m_modulePath + m_vars->value("CMAKE_CURRENT_SOURCE_DIR");
    kDebug(9042) << "Include:" << inc->includeFile() << "@" << modulePath << " into ";

    QString possib=inc->includeFile();
    QString path;
    if(possib[0]=='/' && QFile::exists(possib))
        path=possib;
    else
    {
        if(!possib.contains('.'))
            possib += ".cmake";
        path=findFile(possib, modulePath, QStringList());
    }

    if(!path.isEmpty())
    {
        m_vars->insertMulti("CMAKE_CURRENT_LIST_FILE", QStringList(path));
        CMakeFileContent include = CMakeListsParser::readCMakeFile(path);
        if ( !include.isEmpty() )
        {
            TopDUContext *aux=m_topctx;
            if(m_topctx)
            {
                DUChainWriteLocker lock(DUChain::lock());
                m_topctx=DUChain::self()->chainForDocument(KUrl(include.first().filePath));
                if(m_topctx==0)
                {
                    m_topctx=new TopDUContext(HashedString(KUrl(include.first().filePath).prettyUrl()),
                            SimpleRange(0,0, include.last().endColumn, include.last().endLine));
                    DUChain::self()->addDocumentChain(
                        IdentifiedFile(HashedString(KUrl(include.first().filePath).prettyUrl())), m_topctx);
                    
                    Q_ASSERT(DUChain::self()->chainForDocument(KUrl(include.first().filePath)));
                }
                else
                {
                    m_topctx->clearLocalDeclarations();
                    m_topctx->deleteUses();
                }
                aux->addImportedParentContext(m_topctx);
            }
            kDebug(9042) << "including:" << path;
            walk(include, 0);
            m_topctx=aux;
        }
        else
        {
            //FIXME: Put here the error.
            kDebug(9042) << "Include. Parsing error.";
        }
        m_vars->take("CMAKE_CURRENT_LIST_FILE");
    }
    else
    {
        if(!inc->optional())
        {
            kDebug(9032) << "error!! Could not find" << inc->includeFile() << "=" << possib << "into" << modulePath;
        }
    }
    
    if(!inc->resultVariable().isEmpty())
    {
        QString result="NOTFOUND";
        if(!path.isEmpty())
            result=path;
        m_vars->insert(inc->resultVariable(), QStringList(result));
    }
    kDebug(9042) << "include of" << inc->includeFile() << "done.";
    return 1;
}

int CMakeProjectVisitor::visit(const FindPackageAst *pack)
{
    if(!haveToFind(pack->name()))
        return 1;
    const QStringList modulePath = m_vars->value("CMAKE_MODULE_PATH") + m_modulePath;
    kDebug(9042) << "Find:" << pack->name() << "package." << m_modulePath;

    QString possib=pack->name();
    if(!possib.endsWith(".cmake"))
        possib += ".cmake";
    QString path=findFile("Find"+possib, modulePath, QStringList());
    if(!path.isEmpty())
    {
        m_vars->insertMulti("CMAKE_CURRENT_LIST_FILE", QStringList(path));
        CMakeFileContent package=CMakeListsParser::readCMakeFile( path );
        if ( !package.isEmpty() )
        {
            path=KUrl(path).prettyUrl();
            kDebug(9042) << "================== Found" << path << "===============";
            TopDUContext *aux=m_topctx;
            {
                DUChainWriteLocker lock(DUChain::lock());
                m_topctx=DUChain::self()->chainForDocument(KUrl(path));
                if(m_topctx==0)
                {
                    m_topctx=new TopDUContext(HashedString(path),
                            SimpleRange(0,0, package.last().endColumn, package.last().endLine));
                    DUChain::self()->addDocumentChain(
                        IdentifiedFile(HashedString(path)), m_topctx);
                    
                    Q_ASSERT(DUChain::self()->chainForDocument(KUrl(path)));
                    aux->addImportedParentContext(m_topctx);
                }
                else
                {
                    m_topctx->clearLocalDeclarations();
                    m_topctx->deleteUses();
                }
            }
            walk(package, 0);
            m_topctx=aux;
        }
        else
        {
            kDebug(9032) << "error: find_package. Parsing error." << path;
        }
        m_vars->take("CMAKE_CURRENT_LIST_FILE");
    }
    else if(pack->isRequired())
    {
        //FIXME: Put here the error.
        kDebug(9032) << "error: Could not find" << pack->name() << "into" << modulePath;
    }
    kDebug(9042) << "Exit. Found:" << pack->name() << m_vars->value(pack->name()+"_FOUND");
    return 1;
}

bool CMakeProjectVisitor::haveToFind(const QString &varName)
{
    if(m_vars->contains(varName+"_FOUND"))
        return false;
    else if(m_vars->contains(varName+"-NOTFOUND"))
        m_vars->remove(varName+"-NOTFOUND");
    return true;
}

int CMakeProjectVisitor::visit(const FindProgramAst *fprog)
{
    if(!haveToFind(fprog->variableName()))
        return 1;

    QStringList modulePath = fprog->path();
#ifdef Q_OS_WIN
    if(!fprog->noSystemEnvironmentPath() && !fprog->noDefaultPath())
        modulePath += envVarDirectories("Path");
#else
    if(!fprog->noSystemEnvironmentPath() && !fprog->noDefaultPath())
        modulePath += envVarDirectories("PATH");
#endif
    kDebug(9042) << "Find:" << fprog->variableName() /*<< "program into" << modulePath<<":"<< fprog->path()*/;
    QStringList paths;
    foreach(const QString& file, fprog->filenames())
    {
        QString path=findFile(file, modulePath, fprog->pathSuffixes(), Executable);
        if(!path.isEmpty()) {
            paths+=path;
            break;
        }
    }
    if(!paths.isEmpty())
        m_vars->insert(fprog->variableName(), paths);
    else
        m_vars->insert(fprog->variableName()+"-NOTFOUND", QStringList());

    kDebug(9042) << "FindProgram:" << fprog->variableName() << "=" << m_vars->value(fprog->variableName()) << modulePath;
    return 1;
}

int CMakeProjectVisitor::visit(const FindPathAst *fpath)
{
    if(!haveToFind(fpath->variableName()))
        return 1;

    bool error=false;
    QStringList locationOptions = fpath->path();
    QStringList path, files=fpath->filenames();
    
    if(!fpath->noDefaultPath()) {
        locationOptions += m_defaultPaths;
    }

    kDebug(9042) << "Find:" << /*locationOptions << "@" <<*/ fpath->variableName() << /*"=" << files <<*/ " path.";
    foreach(const QString& p, files)
    {
        QString p1=findFile(p, locationOptions, fpath->pathSuffixes(), Location);
        if(p1.isEmpty())
        {
            kDebug(9042) << p << "not found";
            error=true;
        }
        else
        {
            path += p1;
        }
    }

    if(!path.isEmpty())
    {
        m_vars->insert(fpath->variableName(), QStringList(path));
    }
    else
    {
        kDebug(9042) << "Path not found";
    }
    kDebug(9042) << "Find path: " << fpath->variableName() << m_vars->value(fpath->variableName());
//     m_vars->insert(fpath->variableName()+"-NOTFOUND", QStringList());
    return 1;
}

int CMakeProjectVisitor::visit(const FindLibraryAst *flib)
{
    if(!haveToFind(flib->variableName()))
        return 1;

    bool error=false;
    QStringList locationOptions = flib->path();
    QStringList path, files=flib->filenames();

    if(!flib->noDefaultPath())
    {
        locationOptions += m_defaultPaths;
    }

    foreach(const QString& p, files)
    {
        QString p1=findFile(p, locationOptions, flib->pathSuffixes(), Library);
        if(p1.isEmpty())
        {
            kDebug(9042) << p << "not found";
            error=true;
        }
        else
        {
            path += p1;
            break;
        }
    }

    if(!path.isEmpty())
    {
        m_vars->insert(flib->variableName(), QStringList(path));
    }
    else
        kDebug(9032) << "error. Library" << flib->filenames() << "not found";
//     m_vars->insert(fpath->variableName()+"-NOTFOUND", QStringList());
    kDebug(9042) << "Find Library:" << flib->filenames() << m_vars->value(flib->variableName());
    return 1;
}

int CMakeProjectVisitor::visit(const FindFileAst *ffile)
{
    if(!haveToFind(ffile->variableName()))
        return 1;

    bool error=false;
    QStringList locationOptions = ffile->path();
    QStringList path, files=ffile->filenames();

    kDebug(9042) << "Find File:" << ffile->filenames();
    foreach(const QString& p, files)
    {
        QString p1=findFile(p, locationOptions, ffile->pathSuffixes(), File);
        if(p1.isEmpty())
        {
            kDebug(9042) << p << "not found";
            error=true;
        }
        else
        {
            path += p1;
        }
    }

    if(!path.isEmpty())
    {
        m_vars->insert(ffile->variableName(), QStringList(path));
    }
    else
        kDebug(9032) << "error. File" << ffile->filenames() << "not found";
//     m_vars->insert(fpath->variableName()+"-NOTFOUND", QStringList());
    return 1;
}


int CMakeProjectVisitor::visit(const TryCompileAst *tca)
{
    kDebug(9042) << "try_compile" << tca->resultName() << tca->binDir() << tca->source()
            << "cmakeflags" << tca->cmakeFlags() << "outputvar" << tca->outputName();
    if(m_projectName.isEmpty())
    {
        kDebug(9042) << "file compile" << tca->compileDefinitions() << tca->copyFile();
    }
    else
    {
        kDebug(9042) << "project compile" << tca->projectName() << tca->targetName();
    }
    m_vars->insert(tca->resultName(), QStringList("TRUE"));
    return 1;
}


int CMakeProjectVisitor::visit(const TargetLinkLibrariesAst *)
{
    kDebug(9042) << "target_link_libraries";
    return 1;
}

int CMakeProjectVisitor::visit(const MacroAst *macro)
{
    kDebug(9042) << "Adding macro:" << macro->macroName();
    Macro m;
    m.name = macro->macroName();
    m.knownArgs=macro->knownArgs();
    CMakeFileContent::const_iterator it=macro->content().constBegin()+macro->line();
    CMakeFileContent::const_iterator itEnd=macro->content().constEnd();
    int lines=0;
    for(; it!=itEnd; ++it)
    {
        if(it->name.toLower()=="endmacro")
            break;
        m.code += *it;
        ++lines;
    }
    ++lines; //We do not want to return to endmacro
    m_macros->insert(macro->macroName(), m);

    DUChainWriteLocker lock(DUChain::lock());
    QList<Declaration*> decls=m_topctx->findDeclarations(Identifier(macro->macroName()));
    SimpleRange sr=macro->content().first().arguments.first().range();
    if(!decls.isEmpty())
    {
        int idx=m_topctx->indexForUsedDeclaration(decls.first(), false);
        m_topctx->createUse(idx, sr, 0);
    }
    else
    {
        Declaration *d = new Declaration(m_topctx->url(), sr, Declaration::GlobalScope, m_topctx);
        d->setIdentifier( Identifier(macro->macroName()) );
        
        FunctionType* func=new FunctionType();
        foreach(const QString& arg, macro->knownArgs())
        {
            DelayedType *delayed=new DelayedType;
            delayed->setIdentifier( arg );
            func->addArgument(AbstractType::Ptr(delayed));
        }
        d->setAbstractType( AbstractType::Ptr(func) );
    }

    return lines;
}

int CMakeProjectVisitor::visit(const MacroCallAst *call)
{
    if(m_macros->contains(call->name()))
    {
        const Macro code=m_macros->value(call->name());
        kDebug(9042) << "Running macro:" << call->name() << "params:" << call->arguments() << "=" << code.knownArgs << "for" << code.code.count() << "lines";
        
        if(code.knownArgs.count() > call->arguments().count())
        {
            kDebug(9032) << "error: more parameters needed when calling" << call->name();
        }
        else
        {
            //Giving value to parameters
            QStringList::const_iterator mit = code.knownArgs.constBegin();
            QStringList::const_iterator cit = call->arguments().constBegin();
            QStringList argn;
            bool haveArgn=false;
            int i=1;
            while(cit != call->arguments().constEnd())
            {
                if(mit!=code.knownArgs.constEnd())
                {
                    kDebug(9042) << "param:" << *mit << "=" << *cit;
                    m_vars->insertMulti(*mit, QStringList(*cit));
                    m_vars->insertMulti(QString("ARGV%1").arg(i), QStringList(*cit));
                    mit++;
                }
                else
                {
                    haveArgn=true;
                    argn += *cit;
                }
                cit++;
                i++;
            }
            m_vars->insertMulti("ARGN", argn);
            m_vars->insertMulti("ARGV", call->arguments());
            m_vars->insertMulti("ARGC", QStringList(QString::number(call->arguments().count())));
            kDebug(9042) << "argn=" << m_vars->value("ARGN");
            
            //Executing
            TopDUContext *auxctx=m_topctx;
            m_topctx=0;
            int len = walk(code.code, 1);
            kDebug(9042) << "visited!" << call->name()  <<
                m_vars->value("ARGV") << "_" << m_vars->value("ARGN") << "..." << len;
            m_topctx=auxctx;
            
            //Restoring
            i=1;
            foreach(const QString& name, code.knownArgs)
            {
                m_vars->take(QString("ARGV%1").arg(i));
                m_vars->take(name);
                i++;
            }

            m_vars->take("ARGV");
            m_vars->take("ARGC");
            m_vars->take("ARGN");

        }
    }
    else
    {
        kDebug(9032) << "error: Did not find the macro:" << call->name() << call->content()[call->line()].writeBack();
    }
    return 1;
}


int CMakeProjectVisitor::visit(const IfAst *ifast)  //Highly crappy code
{
    int lines=ifast->line();
    CMakeCondition cond(m_vars);
    bool result=cond.condition(ifast->condition());
    
    kDebug(9042) << "Visiting If" << ifast->condition() << "?" << result;
    if(result)
    {
//         kDebug(9042) << "if executed, @" << lines; //<< "now:" << ifast->content()[lines+1].writeBack();
        lines+=walk(ifast->content(), lines+1)-lines;
    }
    else
    {
        int inside=0;
//         kDebug(9042) << "if() was false, looking for an else/elseif @" << lines;
        CMakeFileContent::const_iterator it=ifast->content().constBegin()+lines;
        CMakeFileContent::const_iterator itEnd=ifast->content().constEnd();
        
        for(; it!=itEnd; ++it, lines++)
        {
            QString funcName=it->name.toLower();
//             kDebug(9032) << "looking @" << lines << it->writeBack() << ">>" << inside;
            if(funcName=="if")
            {
                inside++;
            }
            else if(funcName=="endif")
            {
                inside--;
                if(inside<=0)
                    break;
//                 kDebug(9042) << "found an endif at:" << lines << "but" << inside;
            }
            else if(inside==1 && funcName.startsWith("else"))
            {
                if(funcName.endsWith("if")) //it is an else if
                {
//                     kDebug(9042) << "found an elseif" << it->writeBack();
                    IfAst myIf;
                    if(!myIf.parseFunctionInfo(*it))
                        kDebug(9042) << "elseif not correct";
                    if(cond.condition(myIf.condition()))
                    {
//                         kDebug(9042) << "which was true, calculating";
                        lines = walk(ifast->content(), lines+1);
                        break;
                    }
//                     else kDebug(9042) << "which was false";
                }
                else //it is an else
                {
//                     kDebug(9042) << "Found an else finally";
                    lines = walk(ifast->content(), lines+1);
                    break;
                }
            }
        }
    }
    
//     kDebug(9042) << "looking for the endif now @" << lines;
    int inside=0;
    CMakeFileContent::const_iterator it=ifast->content().constBegin()+lines;
    CMakeFileContent::const_iterator itEnd=ifast->content().constEnd();
    for(; inside>=0 && it!=itEnd; ++it, lines++)
    {
        QString funcName=it->name.toLower();
        if(funcName=="if")
            inside++;
        else if(funcName=="endif")
            inside--;
//         kDebug(9042) << "endif???" << it->writeBack() << lines;
    }
    
//     kDebug(9042) << "endif==" << ifast->content()[lines-1].writeBack() << "<>" << ifast->condition() << '=' << lines-ifast->line() << "@" << lines;
    return lines-ifast->line();
}

int CMakeProjectVisitor::visit(const ExecProgramAst *exec)
{
    QString execName = exec->executableName();
    QStringList argsTemp = exec->arguments();
    QStringList args;

    foreach(const QString& arg, argsTemp)
    {
        if(arg.contains(' '))
        {
            QStringList val=arg.split(' ');
            foreach(const QString& s, val)
            {
                args.append(s);
            }
        }
        else
        {
            args.append(arg);
        }
    }
    kDebug(9042) << "Executing:" << execName << "::" << args << "in" << exec->workingDirectory();

    KProcess p;
    if(!exec->workingDirectory().isEmpty()) 
        p.setWorkingDirectory(exec->workingDirectory());
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, args);
    p.start();

    if(!p.waitForFinished())
    {
        kDebug(9032) << "error: failed to execute:" << execName << "error:" << p.error() << p.exitCode();
    }

    if(!exec->returnValue().isEmpty())
    {
        kDebug(9042) << "execution returned: " << exec->returnValue() << " = " << p.exitCode();
        m_vars->insert(exec->returnValue(), QStringList(QString::number(p.exitCode())));
    }

    if(!exec->outputVariable().isEmpty())
    {
        QByteArray b = p.readAllStandardOutput();
        QString t;
        t.prepend(b.trimmed());
        m_vars->insert(exec->outputVariable(), QStringList(t.trimmed()));
        kDebug(9042) << "executed" << execName << "<" << t;
    }
    return 1;
}

int CMakeProjectVisitor::visit(const ExecuteProcessAst *exec)
{
    kDebug(9042) << "executing... " << exec->commands();
    QList<KProcess*> procs;
    foreach(QStringList args, exec->commands())
    {
        KProcess *p=new KProcess(), *prev=0;
        if(!procs.isEmpty())
        {
            prev=procs.last();
        }
        p->setWorkingDirectory(exec->workingDirectory());
        p->setOutputChannelMode(KProcess::MergedChannels);
        QString execName=args.takeFirst();
        p->setProgram(execName, args);
        p->start();
        procs.append(p);
        kDebug(9042) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

        if(prev)
        {
            prev->setStandardOutputProcess(p);
        }
    }

    foreach(KProcess* p, procs)
    {
        if(!p->waitForFinished())
        {
            kDebug(9042) << "error: failed to execute:" << p;
        }
    }

    if(!exec->outputVariable().isEmpty())
    {
        QByteArray b = procs.last()->readAllStandardOutput();
        QString t;
        t.prepend(b.trimmed());
        m_vars->insert(exec->outputVariable(), QStringList(t.trimmed()));
        kDebug(9042) << "executed " << exec->outputVariable() << "=" << t;
    }
    qDeleteAll(procs);
    return 1;
}


int CMakeProjectVisitor::visit(const FileAst *file)
{
    switch(file->type()) //TODO
    {
        case FileAst::WRITE:
            kDebug(9042) << "(ni) File write: " << file->path() << file->message();
            break;
        case FileAst::APPEND:
            kDebug(9042) << "(ni) File append: " << file->path() << file->message();
            break;
        case FileAst::READ:
        {
            KUrl filename=file->path();
            QFileInfo ifile(filename.toLocalFile());
            kDebug(9042) << "FileAst: reading " << file->path() << ifile.isFile();
            if(!ifile.isFile())
                return 1;
            QFile f(filename.toLocalFile());
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
                return 1;
            QString output;
            while (!f.atEnd()) {
                QByteArray line = f.readLine();
                output += line;
            }
            m_vars->insert(file->variable(), QStringList(output));
            kDebug(9042) << "FileAst: read ";
        }
            break;
//         case FileAst::GLOB:
//         case FileAst::GLOB_RECURSE:
//         case FileAst::REMOVE:
//         case FileAst::REMOVE_RECURSE:
//         case FileAst::MAKE_DIRECTORY:
//         case FileAst::RELATIVE_PATH:
        case FileAst::TO_CMAKE_PATH:
#ifdef Q_OS_WIN
            m_vars->insert(file->variable(), file->path().split(';'));
#else
            m_vars->insert(file->variable(), file->path().split(':'));
#endif
            kDebug(9042) << "file TO_CMAKE_PATH variable:" << file->variable() << "="
                    << m_vars->value(file->variable()) << "file:" << file->path();
            break;
//         case FileAst::TO_NATIVE_PATH:
//             break;
        default:
            kDebug(9032) << "error: not implemented. file:" << file->type() << "variable:" << file->variable() << "file:" << file->path();
            break;
    }
    return 1;
}

int CMakeProjectVisitor::visit(const MessageAst *msg)
{
    kDebug(9032) << "message:" << msg->message();
    return 1;
}

int CMakeProjectVisitor::visit(const MathAst *math)
{
    QScriptEngine eng;
    QScriptValue result = eng.evaluate(math->expression());

    if (result.isError())
    {
        kDebug(9032) << "error: found an error while calculating" << math->expression();
    }
    kDebug(9042) << "math. " << math->expression() << "=" << result.toString();
    m_vars->insert(math->outputVariable(), QStringList(result.toString()));
    return 1;
}

int CMakeProjectVisitor::visit(const GetFilenameComponentAst *filecomp)
{
    QString val, path=filecomp->fileName();
    
    switch(filecomp->type())
    {
        case GetFilenameComponentAst::PATH:
            val=path.mid(0, path.lastIndexOf('/'));
            break;
        case GetFilenameComponentAst::ABSOLUTE:
            val=m_root+'/'+path;//FIXME: Should solve it.
            break;
        case GetFilenameComponentAst::NAME: {
            int p=path.lastIndexOf('/')+1;
            val=path.mid(p, path.count()-p);
        } break;
        case GetFilenameComponentAst::EXT: {
            int p=path.lastIndexOf('.')+1;
            if(p>=0)
                val=path.mid(p, path.count()-p);
        } break;
        case GetFilenameComponentAst::NAME_WE: {
            int p=path.lastIndexOf('/')+1;
            int p2 = path.lastIndexOf('.');
            val=path.mid(p, p2-p);
        } break;
        case GetFilenameComponentAst::PROGRAM:
            kDebug(9042) << "error: filenamecopmonent PROGRAM not implemented"; //TODO: <<
            break;
    }
    m_vars->insert(filecomp->variableName(), QStringList(val));
    kDebug(9042) << "filename component" << filecomp->variableName() << "= "
            << filecomp->fileName() << "=" << val << endl;
    return 1;
}

int CMakeProjectVisitor::visit(const OptionAst *opt)
{
    kDebug(9042) << "option" << opt->variableName() << "-" << opt->description();
    if(!m_vars->contains(opt->variableName()))
    {
        m_vars->insert(opt->variableName(), QStringList(opt->defaultValue()));
    }
    return 1;
}

int CMakeProjectVisitor::visit(const ListAst *list)
{
    QString output = list->output();
    QStringList theList = m_vars->value(list->list());
    switch(list->type())
    {
        case ListAst::LENGTH:
            m_vars->insert(output, QStringList(QString::number(theList.count())));
            kDebug(9042) << "List length" << m_vars->value(output);
            break;
        case ListAst::GET:
            if(list->index().first()>=theList.count())
                kDebug(9032) << "error! trying to GET an element that doesn't exist!" << list->index().first();
            else
                m_vars->insert(output, QStringList(theList[list->index().first()]));
            kDebug(9042) << "List: Get" << theList << list->output() << m_vars->value(list->output());
            break;
        case ListAst::APPEND:
            theList += list->elements();
            m_vars->insert(list->list(), theList);
            break;
        case ListAst::FIND: {
            int idx=-1;
            foreach(const QString& val, theList)
            {
                if(val==list->elements().first())
                    break;
                idx++;
            }
            m_vars->insert(list->output(), QStringList(QString::number(idx)));
            kDebug(9042) << "List: Find" << theList << list->output() << list->elements() << idx;
        }   break;
        case ListAst::INSERT: {
            int p=list->index().first();
            foreach(const QString& elem, list->elements())
            {
                theList.insert(p, elem);
                p++;
            }
            m_vars->insert(list->list(), theList);
        }   break;
        case ListAst::REMOVE_ITEM:
            kDebug(9042) << "list remove item: " << theList << list->elements();
            foreach(const QString& elem, list->elements())
            {
                theList.removeAll(elem);
            }

            m_vars->insert(list->list(), theList);
            break;
        case ListAst::REMOVE_AT: {
            QList<int> indices=list->index();
            qSort(indices);
            QList<int>::const_iterator it=indices.constEnd();
            kDebug(9042) << "list remove: " << theList << indices;
            while(it!=indices.constBegin())
            {
                --it;
                theList.removeAt(*it);
            }
            m_vars->insert(list->list(), theList);
        }   break;
        case ListAst::SORT:
            qSort(theList);
            m_vars->insert(list->list(), theList);
            break;
        case ListAst::REVERSE: {
            QStringList reversed;
            foreach(const QString& elem, theList)
                reversed.prepend(elem);
            m_vars->insert(list->list(), reversed);
            }
            break;
    }
    kDebug(9042) << "List!!" << list->output() << '='<< m_vars->value(list->output()) << " -> " << m_vars->value(list->list());
    return 1;
}

int CMakeProjectVisitor::visit(const ForeachAst *fea)
{
    kDebug(9042) << "foreach>" << fea->loopVar() << "=" << fea->arguments() << "range=" << fea->range();
    int end = 1;
    if(fea->range())
    {
        for( int i = fea->ranges().start; i < fea->ranges().stop; i += fea->ranges().step )
        {
            m_vars->insertMulti(fea->loopVar(), QStringList(QString::number(i)));
            end=walk(fea->content(), fea->line()+1);
            m_vars->take(fea->loopVar());
        }
    }
    else
    {
        //Looping in a list of values
        QStringList args=fea->arguments();
        if(args.count()==1 && args.first().isEmpty()) { //if the args are empty
            int lines=fea->line()+1, depth=1;
            CMakeFileContent::const_iterator it=fea->content().constBegin()+lines;
            CMakeFileContent::const_iterator itEnd=fea->content().constEnd();
            for(; depth>0 && it!=itEnd; ++it, lines++)
            {
                if(it->name.toLower()=="foreach")
                {
                    depth++;
                }
                else if(it->name.toLower()=="endforeach")
                {
                    depth--;
                }
            }
            end=lines-1;
        }
        else
        {
            foreach(const QString& s, args)
            {
                m_vars->insertMulti(fea->loopVar(), QStringList(s));
                end=walk(fea->content(), fea->line()+1);
                m_vars->take(fea->loopVar());
            }
        }
    }
    kDebug(9042) << "EndForeach" << fea->loopVar();
    return end-fea->line()+1;
}

int CMakeProjectVisitor::visit(const StringAst *sast)
{
    kDebug(9042) << "String to" /*<< sast->input()*/ << sast->input().isEmpty();
    switch(sast->type())
    {
        case StringAst::REGEX:
        {
            QStringList res;
            QRegExp rx(sast->regex());
            switch(sast->cmdType())
            {
                case StringAst::MATCH:
                    foreach(const QString& in, sast->input())
                    {
                        int match=rx.indexIn(in);
                        if(match>=0) {
                            res = QStringList(in.mid(match, rx.matchedLength()));
                            break;
                        }
                    }
                    break;
                case StringAst::MATCHALL:
                    foreach(const QString& in, sast->input())
                    {
                        int match=rx.indexIn(in);
                        if(match>0)
                        {
                            res += in.mid(match, rx.matchedLength());
                        }
                    }
                    break;
                case StringAst::REGEX_REPLACE:
                {
                    QRegExp rx(sast->regex());
                    kDebug(9042) << "REGEX REPLACE" << sast->input() << sast->regex() << sast->replace();
                    if(sast->replace().startsWith('\\'))
                    {
                        rx.indexIn(sast->input()[0]);
                        QStringList info = rx.capturedTexts();
                        int idx = sast->replace().right(sast->replace().size()-1).toInt();
//                         kDebug(9042) << "\\number replace" << idx << info << sast->input();
                        if(idx>=info.count())
                            kDebug(9032) << "error: not matched regex";
                        else
                            res.append(info[idx]);
                    }
                    else
                    {
                        foreach(QString in, sast->input())
                        {
                            int idx = rx.indexIn(in);
                            QStringList info = rx.capturedTexts();
                            if(idx<0)
                            {
                                res.append(in);
                            }
                            else
                            {
                                foreach(QString s, info)
                                {
                                    res.append(in.replace(s, sast->replace()));
                                }
                            }
                        }
                    }
                    kDebug(9042) << "ret: " << res << " << string(regex replace "
                            << sast->regex() << sast->replace() << sast->outputVariable() << sast->input();
                }
                    break;
                default:
                    kDebug(9032) << "ERROR String: Not a regex. " << sast->cmdType();
                    break;
            }
            m_vars->insert(sast->outputVariable(), QStringList(res));
        }
            break;
        case StringAst::REPLACE: {
            QStringList out;
            foreach(QString in, sast->input())
            {
                QString aux=in.replace(sast->regex(), sast->replace());
                out += aux.split(";"); //FIXME: HUGE ugly hack
            }
            kDebug(9042) << "string REPLACE" << sast->input() << "=>" << out;
            m_vars->insert(sast->outputVariable(), out);
        }   break;
        case StringAst::COMPARE:
        {
            QString res;
            switch(sast->cmdType()){
                case StringAst::EQUAL:
                case StringAst::NOTEQUAL:
                    if(sast->input()[0]==sast->input()[1] && sast->cmdType()==StringAst::EQUAL)
                        res = "TRUE";
                    else
                        res = "FALSE";
                    break;
                case StringAst::LESS:
                case StringAst::GREATER:
                    if(sast->input()[0]<sast->input()[1] && sast->cmdType()==StringAst::LESS)
                        res = "TRUE";
                    else
                        res = "FALSE";
                    break;
                default:
                    kDebug(9042) << "String: Not a compare. " << sast->cmdType();
            }
            m_vars->insert(sast->outputVariable(), QStringList(res));
        }
            break;
        case StringAst::ASCII:
        case StringAst::CONFIGURE:
            kDebug(9032) << "Error! String feature not supported!";
            break;
        case StringAst::TOUPPER:
            m_vars->insert(sast->outputVariable(), QStringList(sast->input()[0].toUpper()));
            break;
        case StringAst::TOLOWER:
            m_vars->insert(sast->outputVariable(), QStringList(sast->input()[0].toLower()));
            break;
        case StringAst::LENGTH:
            m_vars->insert(sast->outputVariable(), QStringList(QString::number(sast->input()[0].count())));
            break;
        case StringAst::SUBSTRING:
        {
            QString res=sast->input()[0];
            res=res.mid(sast->begin(), sast->length());
            m_vars->insert(sast->outputVariable(), QStringList(res));
        }
            break;
    }
    kDebug(9042) << "String " << m_vars->value(sast->outputVariable());
    return 1;
}


int CMakeProjectVisitor::visit(const GetCMakePropertyAst *past)
{
    QStringList output;
    switch(past->type())
    {
        case GetCMakePropertyAst::VARIABLES:
            kDebug(9042) << "get cmake prop: variables:" << m_vars->size();
            output = m_vars->keys();
            break;
        case GetCMakePropertyAst::CACHE_VARIABLES: //FIXME: We do not have cache yet
            output = m_vars->keys();
            break;
        case GetCMakePropertyAst::COMMANDS:      //FIXME: We do not have commands yet
            output = QStringList();
            break;
        case GetCMakePropertyAst::MACROS:
            output = m_macros->keys();
            break;
    }
    m_vars->insert(past->variableName(), output);
    return 1;
}

int CMakeProjectVisitor::visit(const CustomCommandAst *ccast)
{
    kDebug(9042) << "CustomCommand" << ccast->outputs();
    if(ccast->isForTarget())
    {
        //TODO: implement me
    }
    else
    {
        foreach(const QString& out, ccast->outputs())
        {
            m_generatedFiles[out] = QStringList(ccast->mainDependency())/*+ccast->otherDependencies()*/;
            kDebug(9042) << "Have to generate:" << out << "with" << m_generatedFiles[out];
        }
    }
    return 1;
}


int CMakeProjectVisitor::visit(const CustomTargetAst *ctar)
{
    kDebug(9042) << "custom_target " << ctar->target() << ctar->dependencies() << ", " << ctar->commandArgs();
    kDebug(9042) << ctar->content()[ctar->line()].writeBack();
    
    m_filesPerTarget.insert(ctar->target(), ctar->dependencies());
    return 1;
}

QPair<QString, QString> definition(const QString& param)
{
    QPair<QString, QString> ret;
    if(!param.startsWith("-D"))
        return ret;
    int eq=param.indexOf('=', 2);
    ret.first=param.mid(2, eq-2);
    if(eq>0)
        ret.second=param.mid(eq+1);
    return ret;
}

int CMakeProjectVisitor::visit(const AddDefinitionsAst *addDef)
{
//     kDebug(9042) << "Adding defs: " << addDef->definitions();
    foreach(const QString& def, addDef->definitions())
    {
        if(def.isEmpty())
            continue;
        QPair<QString, QString> definePair=definition(def);
        if(definePair.first.isEmpty())
            kDebug(9042) << "error: definition not matched" << def;
        
        m_defs[definePair.first]=definePair.second;
        kDebug(9042) << "added definition" << definePair.first << "=" << definePair.second << " from " << def;
    }
    return 1;
}

int CMakeProjectVisitor::visit(const RemoveDefinitionsAst *remDef)
{
    foreach(const QString& def, remDef->definitions())
    {
        if(def.isEmpty())
            continue;
        QPair<QString, QString> definePair=definition(def);
        if(definePair.first.isEmpty())
            kDebug(9042) << "error: definition not matched" << def;

        m_defs.remove(definePair.first);
        kDebug(9042) << "removed definition" << definePair.first << " from " << def;
    }
    return 1;
}

int CMakeProjectVisitor::visit(const MarkAsAdvancedAst *maa)
{
    kDebug(9042) << "Mark As Advanced" << maa->advancedVars();
    return 1;
}

int CMakeProjectVisitor::visit( const SeparateArgumentsAst * separgs )
{
    QString varName=separgs->variableName();
    QStringList res;
    foreach(const QString& value, m_vars->value(varName))
    {
        res += value.split(' ');
    }
    return 1;
}

int CMakeProjectVisitor::visit( const WhileAst * whileast)
{
    CMakeCondition cond(m_vars);
    bool result=cond.condition(whileast->condition());
    
    kDebug(9042) << "Visiting While" << whileast->condition() << "?" << result;
    if(result)
    {
        walk(whileast->content(), whileast->line()+1);
        walk(whileast->content(), whileast->line());
    }
    int inside=0;
    CMakeFileContent::const_iterator it=whileast->content().constBegin()+whileast->line();
    CMakeFileContent::const_iterator itEnd=whileast->content().constEnd();
    int lines=0;
    for(; inside>=0 && it!=itEnd; ++it, lines++)
    {
        QString funcName=it->name.toLower();
        if(funcName=="while")
            inside++;
        else if(funcName=="endwhile")
            inside--;
    }
    return lines;
}

CMakeFunctionDesc CMakeProjectVisitor::resolveVariables(const CMakeFunctionDesc & exp, const VariableMap * vars)
{
    CMakeFunctionDesc ret=exp;
    ret.arguments.clear();
    
    foreach(const CMakeFunctionArgument &arg, exp.arguments)
    {
        int bef=0, aft;
        VariableType t;
        variableName(arg.value, t, bef, aft);
        if(t)
        {
            ret.addArguments(resolveVariable(arg.value, vars));
        }
        else
        {
            ret.arguments << arg;
        }
    }
    
    return ret;
}

enum RecursivityType { No, Yes, End };

RecursivityType recursivity(const QString& functionName)
{
    if(functionName.toUpper()=="IF" || functionName.toUpper()=="WHILE" ||
       functionName.toUpper()=="FOREACH" || functionName.toUpper()=="MACRO")
        return Yes;
    else if(functionName.toUpper()=="ELSE" || functionName.toUpper()=="ELSEIF")
        return End;
    else if(functionName.toUpper().startsWith("END"))
        return End;
    return No;
}

int CMakeProjectVisitor::walk(const CMakeFileContent & fc, int line)
{
    if(m_topctx==0)
    {
        DUChainWriteLocker lock(DUChain::lock());
        m_topctx=DUChain::self()->chainForDocument(KUrl(fc[0].filePath));
        if(m_topctx==0)
        {
            m_topctx=new TopDUContext(HashedString(KUrl(fc[0].filePath).prettyUrl()),
                    SimpleRange(0,0, fc.last().endLine-1, fc.last().endColumn-1));
            
            DUChain::self()->addDocumentChain(IdentifiedFile(HashedString(KUrl(fc[0].filePath).prettyUrl())), m_topctx);
            Q_ASSERT(DUChain::self()->chainForDocument(KUrl(fc[0].filePath)));
        }
        else
        {
            m_topctx->clearLocalDeclarations();
            m_topctx->deleteUses();
        }
        m_topctx->addImportedParentContext(m_parentCtx);
    }
    VisitorState p;
    p.code = &fc;
    p.context = m_topctx;
    p.line = line;
    
    m_backtrace.push(p);
    
    CMakeFileContent::const_iterator it=fc.constBegin()+line, itEnd=fc.constEnd();
    for(; it!=itEnd; )
    {
        Q_ASSERT( line<fc.count() );
        Q_ASSERT( line>=0 );
//         kDebug(9042) << "@" << line;
//         kDebug(9042) <return core()->languageController()->language(name());
        
        Q_ASSERT( *it == fc[line] );
//         kDebug(9042) <lw< "At line" << line << "/" << fc.count();
        CMakeAst* element = AstFactory::self()->createAst(it->name);

        if(!element)
        {
            element = new MacroCallAst;
        }
        
        createUses(*it);
//         kDebug(9042) << "resolving:" << it->writeBack();
        CMakeFunctionDesc func = resolveVariables(*it, m_vars); //FIXME not correct in while case
//         kDebug(9042) << "resolved:" << func.writeBack();
        bool correct = element->parseFunctionInfo(func);
        if(!correct)
        {
            kDebug(9042) << "error! found an error while processing" << func.writeBack() << "was" << it->writeBack() << endl
                << " at" << func.filePath << ":" << func.line << endl;
            //FIXME: Should avoid to run?
        }
        
        RecursivityType r = recursivity(func.name);
        if(r==End)
        {
//             kDebug(9042) << "Found an end." << func.writeBack();
            delete element;
            return line;
        }
        if(element->isDeprecated())
            kDebug(9042) << "Warning: Using the function: " << func.name << " which is deprecated by cmake.";
        element->setContent(fc, line);
        
        createDefinitions(element);

        m_vars->insert("CMAKE_CURRENT_LIST_LINE", QStringList(QString::number(it->line)));
        int lines=element->accept(this);
        line+=lines;
        it+=lines;
        m_backtrace.top().line = line;
        m_backtrace.top().context = m_topctx;
        delete element;
    }
    m_backtrace.pop();
    kDebug(9042) << "Walk stopped @" << line;
    return line;
}

void CMakeProjectVisitor::createDefinitions(const CMakeAst *ast)
{
    if(m_topctx==0)
        return;
    DUChainWriteLocker lock(DUChain::lock());
    foreach(const CMakeFunctionArgument &arg, ast->outputArguments())
    {
        if(!arg.isCorrect())
            continue;
        QList<Declaration*> decls=m_topctx->findDeclarations(Identifier(arg.value));
        if(!decls.isEmpty())
        {
            int idx=m_topctx->indexForUsedDeclaration(decls.first(), false);
            m_topctx->createUse(idx, arg.range(), 0);
        }
        else
        {
            Declaration *d = new Declaration(m_topctx->url(), arg.range(), Declaration::GlobalScope, m_topctx);
            d->setIdentifier( Identifier(arg.value) );
        }
    }
}

void CMakeProjectVisitor::createUses(const CMakeFunctionDesc& desc)
{
    if(m_topctx==0)
        return;
    int before=0;
    DUChainWriteLocker lock(DUChain::lock());
    foreach(const CMakeFunctionArgument &arg, desc.arguments)
    {
        if(!arg.isCorrect())
        {
            continue;
        }
        
        int after;
        VariableType type;
        QString var = variableName(arg.value, type, before, after);
        if(type)
        {
            QList<Declaration*> decls=m_topctx->findDeclarations(Identifier(var));
            
//             qDebug() << "uuuuuuse" << decls.isEmpty();
            
            if(!decls.isEmpty())
            {
                int idx=m_topctx->indexForUsedDeclaration(decls.first(), false);
                m_topctx->createUse(idx, SimpleRange(arg.line-1, arg.column+before, arg.line-1, arg.column+after), 0);
                
//                 DumpChain d;
//                 d.dump(m_topctx);
            }
        }
        before+=2;
    }
}

void CMakeProjectVisitor::setVariableMap(VariableMap * vars)
{
    m_vars=vars;
}

bool isGenerated(const QString& name)
{
    return name.indexOf("#[")>=0;
}

QStringList CMakeProjectVisitor::targetDependencies(const QString & target) const
{
    QStringList ret;
    foreach(const QString& s, m_filesPerTarget[target])
    {
        if(isGenerated(s))
        {
            kDebug(9042) << "Generated:" << s;
            ret += m_generatedFiles[s];
        }
        else
        {
            ret.append(s);
        }
    }
    return ret;
}
