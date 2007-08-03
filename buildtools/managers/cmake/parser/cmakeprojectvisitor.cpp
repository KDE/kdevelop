/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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

#include <KProcess>
#include <KDebug>
#include <QHash>
#include <QFile>
#include <QByteArray>
#include <QRegExp>
#include <QFileInfo>

QStringList cmakeModulesDirectories()
{
    QStringList env = CMakeProjectVisitor::envVarDirectories("CMAKEDIR");

    QStringList::iterator it=env.begin();
    for(; it!=env.end(); ++it)
        *it += "/Modules";
    return env;
}

CMakeProjectVisitor::CMakeProjectVisitor(const QString& root) : m_root(root)
{}

QStringList CMakeProjectVisitor::envVarDirectories(const QString &varName)
{
    QStringList env = QProcess::systemEnvironment().filter(QRegExp('^'+varName+"=*"));
//     kDebug(9032) << ".......resolving env:" << varName << "=" << env;
    char separator;
    if(!env.isEmpty())
    {
        separator = env[0].contains(':') ? ':' : ';';   //FIXME: this is not the way to do
        env=env[0].split('=')[1].split(separator);
    }
    else
    {
        kDebug(9032) << "error:" << varName << "not found";
        return QStringList();
    }
//     kDebug(9032) << "resolving env:" << varName << "=" << env;
    return env;
}

CMakeProjectVisitor::VariableType CMakeProjectVisitor::hasVariable(const QString &name)
{
    int CMakeIdx=name.indexOf("${"), envIdx=name.indexOf("$ENV{");
    if(name.indexOf('}')>=0)
    {
        if(CMakeIdx>=0 && envIdx>=0)
        {
            if(CMakeIdx<envIdx)
                return CMake;
            else
                return ENV;
        }
        else if(CMakeIdx<0)
            return ENV;
        else
            return CMake;
    }
    else
        return NoVar;
}

QString CMakeProjectVisitor::variableName(const QString &name, VariableType &type)
{
    type = hasVariable(name);
    QString exp;

    switch(type) {
        case NoVar:
            return QString();
        case CMake:
            exp="\\$\\{[A-z0-9-]+\\}";
            break;
        case ENV:
            exp="\\$ENV\\{[A-z0-9-]+\\}";
            break;
    }
    QRegExp rx(exp);
    int idx = rx.indexIn(name);
    if(idx<0) {
        kDebug(9032) << "error!!! I can't know that it is a variable:" << name << ". Report this bug, please." << endl ;
    }
    int begin=name.indexOf('{', idx)+1;
    int end=name.indexOf('}', begin);

    return name.mid(begin, end-begin);
}

QStringList CMakeProjectVisitor::resolveVariable(const QString &exp, const VariableMap *values)
{
//     kDebug(9032) << "lol!" << exp << "@t" << *values;
    if(hasVariable(exp))
    {
        VariableType type;
        QStringList ret;
        QString var = variableName(exp, type);
        if(type==ENV)
        {
            foreach(QString s, envVarDirectories(var))
            {
                QString res=exp;
                ret += res.replace(QString("$ENV{%1}").arg(var), s);
            }
        }
        else
        {
            if(!values->contains(var))
                kDebug(9032) << "warning: Variable" << var << "not defined";
            else
            {
                foreach(QString s, values->value(var))
                {
                    QString res=exp;
                    ret += resolveVariable(res.replace(QString("${%1}").arg(var), s), values);
//                     kDebug(9032) << "Resolving" << var << "=" << s;
                }
            }
        }
        return ret;
    }
    return QStringList(exp);
}

QStringList CMakeProjectVisitor::resolveVariables(const QStringList & vars, const VariableMap *values)
{
//     kDebug(9032) << "resolving:" << vars << "into" << *values;
    QStringList rvars;
    for(QStringList::const_iterator i=vars.begin(); i!=vars.end(); ++i)
    {
        rvars += resolveVariable(*i, values);
    }

    return rvars;
}

int CMakeProjectVisitor::notImplemented(const QString &name) const
{
    kDebug(9032) << "not implemented!" << name;
    return 1;
}

int CMakeProjectVisitor::visit(const CMakeAst *ast)
{
//     kDebug(9032) << "Pipiripipi" << ast->children().count();
    kDebug(9032) << "error! function not implemented";
#if 0
    if(ast->children().isEmpty())
        kDebug(9032) << "warning: visiting an element without children.";
    QList<CMakeAst*> children = ast->children();
    QList<CMakeAst*>::const_iterator it = children.begin();
    for(; it!=children.end(); it++)
    {
        if(*it)
            (*it)->accept(this);
        else
            kWarning(9032) << "Oops!!! found a null object in the AST!" ;
    }
#endif
    return 1;
}

int CMakeProjectVisitor::visit(const ProjectAst *project)
{
    m_projectName = project->projectName();
    m_vars->insert("PROJECT_SOURCE_DIR", QStringList(m_root));
    return 1;
}

int CMakeProjectVisitor::visit(const AddSubdirectoryAst *subd)
{
    kDebug(9032) << "adding subdirectory" << resolveVariable(subd->sourceDir(), m_vars);
    m_subdirectories += resolveVariable(subd->sourceDir(), m_vars);
    return 1;
}

int CMakeProjectVisitor::visit(const AddExecutableAst *exec)
{
//     QString name = resolveVariable(exec->executable(), m_vars).join(";");
    m_filesPerTarget.insert(exec->executable(), exec->sourceLists());
    kDebug(9032) << "exec:" << exec->executable() << "->" << m_filesPerTarget[exec->executable()] << "was" << exec->content()[exec->line()].writeBack();
    return 1;
}

int CMakeProjectVisitor::visit(const AddLibraryAst *lib)
{
    m_filesPerTarget.insert(lib->libraryName(), lib->sourceLists());
    kDebug(9032) << "lib:" << lib->libraryName();
    return 1;
}

int CMakeProjectVisitor::visit(const SetAst *set)
{
    //FIXME: Must deal with ENV{something} case
    QString name = set->variableName();
    kDebug(9032) << "set:" << name << "=" << set->values();
    if(!set->values().isEmpty())
        m_vars->insert(name, resolveVariables(set->values(), m_vars));
    else
        m_vars->insert(name, QStringList("FALSE")); //FIXME: Must know what do we need here
//     kDebug(9032) << set->variableName() << "-result:-" << m_vars->value(name);
    return 1;
}

int CMakeProjectVisitor::visit(const IncludeDirectoriesAst * dirs)
{
    kDebug(9032) << "including" << dirs->includedDirectories();
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
    kDebug(9032) << "done." << dirs->includedDirectories();
    return 1;
}

QString CMakeProjectVisitor::findFile(const QString &file, const QStringList &folders, FileType t)
{
    QString path, filename;
    switch(t) { //only working on linux/unix
        case Library:
            filename=QString("lib%1.so").arg(file);
            break;
        case Location:
        case Executable:
        case File:
            filename=file;
            break;
    }
    foreach(QString mpath, folders) {
        KUrl p(mpath);
        p.addPath(filename);

        QString possib=p.toLocalFile();

//         kDebug(9032) << "Trying:" << possib /*<< "." << p << "." << mpath*/;
        if(QFile::exists(possib))
        {
            switch(t) {
                case Location:
                    path=mpath;
                    break;
                case Executable:    //Must check if it is an executable.
                case Library:
                case File:
                    path=p.toLocalFile();
                    break;
            }
            break;
        }
    }
    kDebug(9032) << "find file" << file << "into:" << folders << "found at:" << path;
    return path;
}

int CMakeProjectVisitor::visit(const IncludeAst *inc)
{
    const QStringList modulePath = resolveVariables(m_vars->value("CMAKE_MODULE_PATH"), m_vars) + cmakeModulesDirectories();
//     kDebug(9032) << "CMAKE_MODULE_PATH:" << modulePath << cmakeModulesDirectories();

    QStringList possib=resolveVariable(inc->includeFile(), m_vars);
    QString path;
    foreach(QString p, possib) {
        if(p[0]=='/' && QFile::exists(p)) {
            path=p;
            break;
        } else if(!p.endsWith(".cmake"))
            p += ".cmake";
        path=findFile(p, modulePath);
    }

    if(!path.isEmpty()) {
        m_vars->insertMulti("CMAKE_CURRENT_LIST_FILE", QStringList(path));
        CMakeFileContent include = CMakeListsParser::readCMakeFile(path);
        if ( !include.isEmpty() )
        {
            kDebug(9032) << "including:" << path;
            walk(include, 0);
        }
        else
        {
            //FIXME: Put here the error.
            kDebug(9032) << "Include. Parsing error.";
        }
        m_vars->take("CMAKE_CURRENT_LIST_FILE");
    }
    else
    {
        if(!inc->optional())
        {
            //FIXME: Put here the error.
            kDebug(9032) << "error!! Could not find" << inc->includeFile() << "=" << possib << "into" << modulePath;
        }
    }
    kDebug(9032) << "include of" << inc->includeFile() << "done.";
    return 1;
}

int CMakeProjectVisitor::visit(const FindPackageAst *pack)
{
    if(!haveToFind(pack->name()))
        return 1;
    const QStringList modulePath = resolveVariables(m_vars->value("CMAKE_MODULE_PATH"), m_vars) + cmakeModulesDirectories();
    kDebug(9032) << "Find:" << pack->name() << "package.";

    QString possib=pack->name();
    if(!possib.endsWith(".cmake"))
        possib += ".cmake";
    QString path=findFile("Find"+possib, modulePath);
    if(!path.isEmpty())
    {
        m_vars->insertMulti("CMAKE_CURRENT_LIST_FILE", QStringList(path));
        CMakeFileContent package=CMakeListsParser::readCMakeFile( path );
        if ( !package.isEmpty() )
        {
            kDebug(9032) << "================== Found" << path.trimmed() << "===============";
            walk(package, 0);
        } else {
            //FIXME: Put here the error.
            kDebug(9032) << "error: find_package. Parsing error." << path;
        }
        m_vars->take("CMAKE_CURRENT_LIST_FILE");
        //delete include; //FIXME
    }
    else if(pack->isRequired())
    {
        //FIXME: Put here the error.
        kDebug(9032) << "error: Could not find" << pack->name() << "into" << modulePath;
    }
    m_vars->insert(pack->name()+"_FOUND", QStringList());
    kDebug(9032) << "Found:" << pack->name() << ". Exit";
    return 1;
}

bool CMakeProjectVisitor::haveToFind(const QString &varName)
{
    if(m_vars->contains(varName+"_FOUND"))
        return false;
    else if(m_vars->contains(varName+"_NOTFOUND"))
        m_vars->remove(varName+"_NOTFOUND");
    return true;
}

int CMakeProjectVisitor::visit(const FindProgramAst *fprog)
{
    if(!haveToFind(fprog->variableName()))
        return 1;

    QStringList modulePath = resolveVariables(fprog->path(), m_vars);
    if(!fprog->noSystemEnvironmentPath() && !fprog->noDefaultPath())
        modulePath += envVarDirectories("PATH");

    kDebug(9032) << "Find:" << fprog->variableName() << "program"/* into" << modulePath<<":"<< fprog->path()*/;
    QStringList paths;
    foreach(QString file, fprog->filenames())
        paths+=findFile(file, modulePath);
    if(!paths.isEmpty())
        m_vars->insert(fprog->variableName(), paths);
    else
        m_vars->insert(fprog->variableName()+"_NOTFOUND", QStringList());

    kDebug(9032) << "FindProgram:" << fprog->variableName() << "=" << m_vars->value(fprog->variableName()) << modulePath;
    return 1;
}

int CMakeProjectVisitor::visit(const FindPathAst *fpath)
{
    if(!haveToFind(fpath->variableName()))
        return 1;

    bool error=false;
    QStringList locationOptions = resolveVariables(fpath->path(), m_vars);
    QStringList path, files=resolveVariables(fpath->filenames(), m_vars);

    kDebug(9032) << "Find:" << /*locationOptions << "@" <<*/ fpath->variableName() << /*"=" << files <<*/ " path.";
    foreach(QString p, files) {
        QString p1=findFile(p, locationOptions, Location);
        if(p1.isEmpty()) {
            kDebug(9032) << p << "not found";
            error=true;
        } else {
            path += p1;
        }
    }

    if(!path.isEmpty()) {
        m_vars->insert(fpath->variableName(), QStringList(path));
    } else
        kDebug(9032) << "Not found";
//     m_vars->insert(fpath->variableName()+"_NOTFOUND", QStringList());
    return 1;
}

int CMakeProjectVisitor::visit(const FindLibraryAst *flib)
{
    if(!haveToFind(flib->variableName()))
        return 1;

    bool error=false;
    QStringList locationOptions = resolveVariables(flib->path(), m_vars);
    QStringList path, files=resolveVariables(flib->filenames(), m_vars);

    kDebug(9032) << "Find Library:" << flib->filenames();
    foreach(QString p, files) {
        QString p1=findFile(p, locationOptions, Library);
        if(p1.isEmpty()) {
            kDebug(9032) << p << "not found";
            error=true;
        } else {
            path += p1;
        }
    }

    if(!path.isEmpty()) {
        m_vars->insert(flib->variableName(), QStringList(path));
    } else
        kDebug(9032) << "Library" << flib->filenames() << "not found";
//     m_vars->insert(fpath->variableName()+"_NOTFOUND", QStringList());
    return 1;
}

int CMakeProjectVisitor::visit(const FindFileAst *ffile)
{
    if(!haveToFind(ffile->variableName()))
        return 1;

    bool error=false;
    QStringList locationOptions = resolveVariables(ffile->path(), m_vars);
    QStringList path, files=resolveVariables(ffile->filenames(), m_vars);

    kDebug(9032) << "Find File:" << ffile->filenames();
    foreach(QString p, files) {
        QString p1=findFile(p, locationOptions, File);
        if(p1.isEmpty()) {
            kDebug(9032) << p << "not found";
            error=true;
        } else {
            path += p1;
        }
    }

    if(!path.isEmpty()) {
        m_vars->insert(ffile->variableName(), QStringList(path));
    } else
        kDebug(9032) << "File" << ffile->filenames() << "not found";
//     m_vars->insert(fpath->variableName()+"_NOTFOUND", QStringList());
    return 1;
}

int CMakeProjectVisitor::visit(const MacroAst *macro)
{
    kDebug(9032) << "Adding macro:" << macro->macroName();
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
    return lines;
}

int CMakeProjectVisitor::visit(const MacroCallAst *call)
{
    if(m_macros->contains(call->name()))
    {
        const Macro code=m_macros->value(call->name());
        kDebug(9032) << "Running macro:" << call->name() << "params:" << call->arguments() << "=" << code.knownArgs << "for" << code.code.count() << "lines";
        
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
                    kDebug(9032) << "param:" << *mit << "=" << *cit;
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
    
            //Executing
            int len = walk(code.code, 1);
            kDebug(9032) << "visited!" << call->name()  << m_vars->value("ARGV") << "_" << m_vars->value("ARGN") << "..." << len;
    
            //Restoring
            i=1;
            foreach(QString name, code.knownArgs)
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
        kDebug(9032) << "Did not find the macro:" << call->name();
    }
    return 1;
}


int CMakeProjectVisitor::visit(const IfAst *ifast)  //Highly crappy code
{
    kDebug(9032) << "Visiting If" << ifast->condition();

    int lines=ifast->line();
    CMakeCondition cond(m_vars);
    if(cond.condition(ifast->condition()))
    {
//         kDebug(9032) << "if executed, @" << lines; //<< "now:" << ifast->content()[lines+1].writeBack();
        lines+=walk(ifast->content(), lines+1)-lines;
    }
    else
    {
        int inside=0;
//         kDebug(9032) << "if() was false, looking for an else/elseif @" << lines;
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
//                 kDebug(9032) << "found an endif at:" << lines << "but" << inside;
            }
            else if(inside==1 && funcName.startsWith("else"))
            {
                if(funcName.endsWith("if")) //it is an else if
                {
//                     kDebug(9032) << "found an elseif" << it->writeBack();
                    IfAst myIf;
                    if(!myIf.parseFunctionInfo(*it))
                        kDebug(9032) << "elseif not correct";
                    if(cond.condition(myIf.condition()))
                    {
//                         kDebug(9032) << "which was true, calculating";
                        lines = walk(ifast->content(), lines+1);
                        break;
                    }
//                     else kDebug(9032) << "which was false";
                }
                else //it is an else
                {
//                     kDebug(9032) << "Found an else finally";
                    lines = walk(ifast->content(), lines+1);
                    break;
                }
            }
        }
    }
    
//     kDebug(9032) << "looking for the endif now @" << lines;
    bool done=false;
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
//         kDebug(9032) << "endif???" << it->writeBack() << lines;
    }
    
//     kDebug(9032) << "endif==" << ifast->content()[lines-1].writeBack() << "<>" << ifast->condition() << '=' << lines-ifast->line() << "@" << lines;
    return lines-ifast->line();
}

int CMakeProjectVisitor::visit(const ExecProgramAst *exec)
{
    QStringList vars = resolveVariable(exec->executableName(), m_vars);
    if(vars.isEmpty()) {
        kDebug(9032) << "bang!" << exec->executableName();
        return 1;
    }

    QString execName = vars[0];
    QStringList argsTemp = resolveVariables(exec->arguments(), m_vars), args;

    foreach(QString arg, argsTemp)
    {
        if(arg.contains(' '))
        {
            QStringList val=arg.split(' ');
            foreach(QString s, val)
            {
                args.append(s);
            }
        }
        else
        {
            args.append(arg);
        }
    }
    kDebug(9032) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

    KProcess p;
    p.setWorkingDirectory(exec->workingDirectory());
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, resolveVariables(args, m_vars));
    p.start();

    if(!p.waitForFinished())
    {
        kDebug(9032) << "failed to execute:" << execName;
    }

    if(!exec->returnValue().isEmpty())
    {
        m_vars->insert(exec->returnValue(), QStringList(QString(p.exitCode())));
    }

    if(!exec->outputVariable().isEmpty())
    {
        QByteArray b = p.readAllStandardOutput();
        QString t;
        t.prepend(b);
        m_vars->insert(exec->outputVariable(), QStringList(t.trimmed()));
        kDebug(9032) << "executed" << execName << "<" << t;
    }
    return 1;
}

int CMakeProjectVisitor::visit(const FileAst *file)
{
    QStringList val;
    switch(file->type()) //TODO
    {
//         case FileAst::WRITE:
//         case FileAst::APPEND:
//         case FileAst::READ:
//         case FileAst::GLOB:
//         case FileAst::GLOB_RECURSE:
//         case FileAst::REMOVE:
//         case FileAst::REMOVE_RECURSE:
//         case FileAst::MAKE_DIRECTORY:
//         case FileAst::RELATIVE_PATH:
        case FileAst::TO_CMAKE_PATH:
            val=resolveVariable(file->path(), m_vars);
            if(!val.isEmpty()) {
                val=val[0].split(':');
                m_vars->insert(file->variable(), val);
            }
            kDebug(9032) << "file TO_CMAKE_PATH variable:" << file->variable() << "=" << m_vars->value(file->variable()) << "file:" << file->path() << val;
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
    QStringList exps=resolveVariable(math->expression(), m_vars);
    if(exps.isEmpty()) {
        kDebug(9032) << "couldn't resolve" << math->expression();
        return 1;
    }
    KProcess p;
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram("bc", QStringList("-q"));
    p.start();
    
    QString exp=exps[0];
    p.write(qPrintable(exp));
    p.closeWriteChannel();
    if(!p.waitForFinished())
        kDebug(9032) << "error: failed to calculate with bc";

    if(p.exitCode()!=0)
        kDebug(9032) << "error executing math bc. returns" << p.exitCode();

    QByteArray b = p.readAllStandardOutput().trimmed();
    QString t;
    t.prepend(b);
    m_vars->insert(math->outputVariable(), QStringList(t));

    kDebug(9032) << "math." << exp << "=" << m_vars->value(math->outputVariable()) << "=" << b;
    return 1;
}

int CMakeProjectVisitor::visit(const GetFilenameComponentAst *filecomp)
{
    QStringList paths=resolveVariable(filecomp->fileName(), m_vars);
    kDebug(9032) << "filename component" << filecomp->variableName() << "= "
        << m_root << "?" << filecomp->fileName() << "=" << paths << endl;
    if(paths.isEmpty()) {
        kDebug(9032) << "error: -.-" << filecomp->fileName();
        return 1;
    }
    
    QString val, path=paths[0];
    switch(filecomp->type()) {
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
            kDebug(9032) << "error: filenamecopmonent PROGRAM not implemented"; //TODO: <<
            break;
    }
    m_vars->insert(filecomp->variableName(), QStringList(val));
    kDebug(9032) << "filecomp:" << val;
    return 1;
}

int CMakeProjectVisitor::visit(const OptionAst *opt)
{
    //TODO: Save options somewhere
    QStringList vars = resolveVariable(opt->variableName(), m_vars);
    kDebug(9032) << "option" << opt->variableName() << "-" << opt->description();
    
    if(vars.isEmpty())
        return 1;
    QString varname = vars[0];
    if(!m_vars->contains(opt->variableName())) {
        if(opt->defaultValue())
            m_vars->insert(varname, QStringList("ON"));
        else
            m_vars->insert(varname, QStringList("OFF"));
    }
    return 1;
}

int CMakeProjectVisitor::visit(const ListAst *list)
{
    kDebug(9032) << "List!!" << list->output();
    QString output = list->output();
    QStringList theList = m_vars->value(list->list());
    switch(list->type())
    {
        case ListAst::LENGTH:
            m_vars->insert(output, QStringList(QString::number(theList.count())));
            kDebug(9032) << "List length" << m_vars->value(output);
            break;
        case ListAst::GET:
            if(list->index().first()>=theList.count())
                kDebug(9032) << "error! trying to GET an element that doesn't exist!" << list->index().first();
            else
                m_vars->insert(output, QStringList(theList[list->index().first()]));
            break;
        case ListAst::APPEND:
            theList += list->elements();
            m_vars->insert(list->list(), theList);
            break;
        case ListAst::INSERT: {
            int p=list->index().first();
            foreach(QString elem, list->elements())
            {
                theList.insert(p, elem);
                p++;
            }
            
            m_vars->insert(list->list(), theList);
        }   break;
        case ListAst::REMOVE_ITEM:
            foreach(QString elem, list->elements())
            {
                theList.removeAll(elem);
            }

            m_vars->insert(list->list(), theList);
            break;
        case ListAst::REMOVE_AT:
            foreach(int i, list->index())
                theList.removeAt(i);
            m_vars->insert(list->list(), theList);
            break;
        case ListAst::SORT:
            qSort(theList);
            m_vars->insert(list->list(), theList);
            break;
        case ListAst::REVERSE: {
            QStringList reversed;
            foreach(QString elem, theList)
                reversed.prepend(elem);
            m_vars->insert(list->list(), reversed);
            }
            break;
    }
    return 1;
}

int CMakeProjectVisitor::visit(const ForeachAst *fea)
{
    kDebug(9032) << "foreach>" << fea->loopVar() << "=" << fea->arguments() << "range=" << fea->range();
    int end;
    if(fea->range())
    {
        kDebug(9032) << "Ranges not implemented" << endl << "look:" << fea->content()[fea->line()].writeBack();
        //Looping in a range
    }
    else
    {
        //Looping in a list of values
        if(fea->arguments().isEmpty())
            kDebug(9032) << "Error! not enough arguments";
        foreach(QString s, fea->arguments())
        {
            m_vars->insertMulti(fea->loopVar(), QStringList(s));
            end=walk(fea->content(), fea->line()+1);
            m_vars->take(fea->loopVar());
        }
    }
    kDebug(9032) << "EndForeach" << fea->loopVar();
    return end-fea->line()+1;
}

int CMakeProjectVisitor::visit(const StringAst *sast)
{
    kDebug(9032) << "String to" << sast->outputVariable() << sast->input().isEmpty();
    switch(sast->type())
    {
        case StringAst::REGEX:
        case StringAst::REPLACE:
        case StringAst::COMPARE:
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
            break;
    }
    return 1;
}


int CMakeProjectVisitor::visit(const GetCMakePropertyAst *past)
{
    QStringList output;
    switch(past->type())
    {
        case GetCMakePropertyAst::VARIABLES:
            kDebug(9032) << "get cmake prop: variables:" << m_vars->size();
            output = m_vars->keys();
            break;
        case GetCMakePropertyAst::CACHE_VARIABLES: //FIXME: I do not have cache yet
            output = m_vars->keys();
            break;
        case GetCMakePropertyAst::COMMANDS:      //FIXME: I do not have commands yet
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
    kDebug(9032) << "CustomCommand" << ccast->outputs();
    if(ccast->isForTarget())
    {}
    else
    {
        foreach(QString out, ccast->outputs())
        {
            m_generatedFiles[out] = QStringList(ccast->mainDependency())/*+ccast->otherDependencies()*/;
            kDebug(9032) << "Have to generate:" << out << "with" << m_generatedFiles[out];
        }
    }
    return 1;
}

CMakeFunctionDesc CMakeProjectVisitor::resolveVariables(const CMakeFunctionDesc & exp, const VariableMap * vars)
{
    CMakeFunctionDesc ret=exp;
    ret.arguments.clear();
    
    foreach(CMakeFunctionArgument arg, exp.arguments)
    {
        if(hasVariable(arg.value))
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
    //delete me
    /*int i=0;
    CMakeFileContent::const_iterator it2=fc.constBegin()+line, itEnd2=fc.constEnd();
    for(; it2!=itEnd2; ++it2)
    {
        kDebug(9032) << i++ << ":" << it2->writeBack();
    }*/
    
    bool correct=true;
    CMakeFileContent::const_iterator it=fc.constBegin()+line, itEnd=fc.constEnd();
    for(; it!=itEnd; )
    {
        Q_ASSERT( line<fc.count() );
        Q_ASSERT( line>=0 );
//         kDebug(9032) << "@" << line;
//         kDebug(9032) << it->writeBack() << "==" << fc[line].writeBack();
        Q_ASSERT( *it == fc[line] );
//         kDebug(9032) << "At line" << line << "/" << fc.count();
        CMakeAst* element = AstFactory::self()->createAst(it->name);

        if(!element)
        {
            element = new MacroCallAst;
        }
        
//         kDebug(9032) << "resolving:" << it->writeBack();
        CMakeFunctionDesc func = resolveVariables(*it, m_vars); //FIXME not correct in while case
//         kDebug(9032) << "resolved:" << func.writeBack();
        QString funcName=func.name;
        bool err = element->parseFunctionInfo(func);
        if(!err)
        {
            kDebug(9032) << "error! found an error while processing" << func.writeBack() << "was" << it->writeBack() << endl <<
                    " at" << func.filePath << ":" << func.line << endl;
            correct=false;
        }
        
        RecursivityType r = recursivity(funcName);
        if(r==End)
        {
//             kDebug(9032) << "Found an end." << func.writeBack();
            delete element;
            return line;
        }

        element->setContent(fc, line);

        int lines=element->accept(this);
        line+=lines;
        it+=lines;
        delete element;
    }
    kDebug(9032) << "Walk stopped @" << line;
    return line;
}

void CMakeProjectVisitor::setVariableMap(VariableMap * vars)
{
    m_vars=vars;
    if(m_vars && !m_vars->contains("CMAKE_MODULE_PATH"))
    {
        QString cmakeCmd = findFile("cmake", envVarDirectories("PATH"), Executable);
        m_vars->insert("CMAKE_CURRENT_BINARY_DIR", QStringList("#[install_dir]")); //FIXME Must arrange it
        m_vars->insert("CMAKE_COMMAND", QStringList(cmakeCmd));
        m_vars->insert("CMAKE_MODULE_PATH", cmakeModulesDirectories());
        m_vars->insert("CMAKE_SYSTEM_NAME", QStringList("Linux")); //FIXME: Make me multi platform
        m_vars->insert("UNIX", QStringList("TRUE")); //FIXME: Make me multi platform
    }
}

bool generated(const QString& name)
{
    return name.indexOf("#[")>=0;
}

QStringList CMakeProjectVisitor::targetDependencies(const QString & target) const
{
    QStringList ret;
    foreach(QString s, m_filesPerTarget[target])
    {
        if(generated(s))
        {
            kDebug(9032) << "Generated:" << s;
            ret += m_generatedFiles[s];
        }
        else
        {
            ret.append(s);
        }
    }
    return ret;
}



