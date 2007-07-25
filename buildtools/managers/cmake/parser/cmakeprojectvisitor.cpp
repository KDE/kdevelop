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

#include <KProcess>
#include <KDebug>
#include <QHash>
#include <QFile>
#include <QByteArray>

QStringList envVarDirectories(const QString &varName)
{
    QStringList env = QProcess::systemEnvironment().filter(varName+'=');
    char separator;
    if(!env.isEmpty()) {
        separator = env[0].contains(';') ? ';' : ':';
        env=env[0].split('=')[1].split(separator);
    } else
        return QStringList();
    return env;
}

CMakeProjectVisitor::VariableType CMakeProjectVisitor::hasVariable(const QString &name)
{
    int CMakeIdx=name.indexOf("${"), envIdx=name.indexOf("$ENV{");
    if(name.indexOf('}')>=0) {
        if(CMakeIdx>=0 && envIdx>=0) {
            if(CMakeIdx<envIdx)
                return CMake;
            else
                return ENV;
        } else if(CMakeIdx<0)
            return ENV;
        else
            return CMake;
    } else
        return None;
}

QString CMakeProjectVisitor::variableName(const QString &name, VariableType &type)
{
    type = hasVariable(name);
    if(type==None)
        return QString();
    int begin = (type==CMake) ? name.indexOf("${")+2 : name.indexOf("$ENV{")+5, end=name.indexOf('}');

    return name.mid(begin, end-begin);
}

QStringList CMakeProjectVisitor::resolveVariable(const QString &exp, const QHash<QString, QStringList> *values)
{
//     kDebug(9032) << "lol!" << exp << " @t " << *values << endl;
    if(hasVariable(exp)) {
        VariableType type;
        QStringList ret;
        QString var = variableName(exp, type);
        if(type==ENV) {
            kDebug(9032) << "env var " << var << endl;
            foreach(QString s, envVarDirectories(var)) {
                QString res=exp;
                ret += res.replace(QString("$ENV{%1}").arg(var), s);
            }
        } else {
            if(!values->contains(var))
                kDebug(9032) << "error: Variable " << var << " not defined" << endl;
            foreach(QString s, values->value(var)) {
                QString res=exp;
                ret += resolveVariable(res.replace(QString("${%1}").arg(var), s), values);
            }
        }
        return ret;
    }
    return QStringList(exp);
}

QStringList CMakeProjectVisitor::resolveVariables(const QStringList & vars, const QHash<QString, QStringList> *values)
{
//     kDebug(9032) << "resolving: " << vars << " into " << *values << endl;
    QStringList rvars;
    for(QStringList::const_iterator i=vars.begin(); i!=vars.end(); ++i)
        rvars += resolveVariable(*i, values);

    return rvars;
}

CMakeProjectVisitor::CMakeProjectVisitor(QHash<QString, QStringList> *vars, QHash<QString, MacroAst*> *macros)
	: m_vars(vars), m_macros(macros)
{
}

void CMakeProjectVisitor::notImplemented() const {
    kDebug(9032) << "not implemented!" << endl;
}

void CMakeProjectVisitor::visit(const CMakeAst *ast)
{
    QList<CMakeAst*> children = ast->children();
    QList<CMakeAst*>::const_iterator it = children.begin();
    for(; it!=children.end(); it++) {
        if(*it)
            (*it)->accept(this);
        else
            kWarning(9032) << "Oops!!! found a null object in the AST!" << endl;
    }
}

void CMakeProjectVisitor::visit(const ProjectAst *project)
{
    m_projectName = project->projectName();
}

void CMakeProjectVisitor::visit(const AddSubdirectoryAst *subd)
{
    m_subdirectories += subd->sourceDir();
}

void CMakeProjectVisitor::visit(const AddExecutableAst *exec)
{
    m_filesPerTarget.insert(exec->executable(), exec->sourceLists());
    kDebug(9032) << "exec: " << exec->executable() << endl;
}

void CMakeProjectVisitor::visit(const AddLibraryAst *lib)
{
    m_filesPerTarget.insert(lib->libraryName(), lib->sourceLists());
    kDebug(9032) << "lib: " << lib->libraryName() << endl;
}

void CMakeProjectVisitor::visit(const SetAst *set)
{
    kDebug(9032) << "set: " << set->variableName() << "=" << set->values() << endl;

    QStringList old=m_vars->value(set->variableName());
    m_vars->insert(set->variableName(), old+resolveVariables(set->values(), m_vars));
    kDebug(9032) << set->variableName() << "-result:-" << *m_vars << endl;
}

void CMakeProjectVisitor::visit(const IncludeDirectoriesAst * dirs)
{
    kDebug(9032) << "including " << dirs->includedDirectories() << endl;
    IncludeDirectoriesAst::IncludeType t = dirs->includeType();

    QStringList toInclude = resolveVariables(dirs->includedDirectories(), m_vars);

    if(t==IncludeDirectoriesAst::DEFAULT) {
        if(m_vars->contains("CMAKE_INCLUDE_DIRECTORIES_BEFORE") && m_vars->value("CMAKE_INCLUDE_DIRECTORIES_BEFORE")[0]=="ON")
            t = IncludeDirectoriesAst::BEFORE;
        else
            t = IncludeDirectoriesAst::AFTER;
    }

    if(t==IncludeDirectoriesAst::AFTER)
        m_includeDirectories += toInclude;
    else
        m_includeDirectories = toInclude + m_includeDirectories;
}

QStringList cmakeModulesDirectories()
{
    QStringList env = envVarDirectories("CMAKEDIR");

    QStringList::iterator it=env.begin();
    for(; it!=env.end(); it++)
        *it += "/Modules";
    return env;
}

QString CMakeProjectVisitor::findFile(const QString &file, const QStringList &folders/*, Type of file to search*/)
{
    QString path;
    foreach(QString mpath, folders) {
        KUrl p(mpath);
        p.addPath(file);

        QString possib=p.toLocalFile();

//         kDebug(9032) << "Trying: " << possib << endl;
        if(QFile::exists(possib)) {
            path=p.toLocalFile();
            break;
        }
    }
    kDebug(9032) << "find file" << file << " into: " << file << " found at: " << path << endl;
    return path;
}

void CMakeProjectVisitor::visit(const IncludeAst *inc)
{
    const QStringList modulePath = resolveVariables(m_vars->value("CMAKE_MODULE_PATH"), m_vars) + cmakeModulesDirectories();
//     kDebug(9032) << "CMAKE_MODULE_PATH: " << modulePath << cmakeModulesDirectories() << endl;

    QString possib=inc->includeFile();
    if(!possib.endsWith(".cmake"))
        possib += ".cmake";
    QString path=findFile(possib, modulePath);
    if(!path.isEmpty()) {
        CMakeAst *include = new CMakeAst;
        if ( !CMakeListsParser::parseCMakeFile( include, path ) )
        {
            kDebug(9032) << "including: " << path << endl;
            include->accept(this);
        } else {
            //FIXME: Put here the error.
            kDebug(9032) << "Include. Parsing error." << endl;
        }
        //delete include; //FIXME
    } else {
        if(!inc->optional()) {
            //FIXME: Put here the error.
            kDebug(9032) << "Could not find " << inc->includeFile() << " into " << modulePath << endl;
        }
    }
}

void CMakeProjectVisitor::visit(const FindPackageAst *pack)
{
    if(!haveToFind(pack->name()+"-FOUND"))
        return;
    kDebug(9032) << "Find: " << pack->name() << " package." << endl;
    const QStringList modulePath = resolveVariables(m_vars->value("CMAKE_MODULE_PATH"), m_vars) + cmakeModulesDirectories();

    QString possib=pack->name();
    if(!possib.endsWith(".cmake"))
        possib += ".cmake";
    QString path=findFile("Find"+possib, modulePath);
    if(!path.isEmpty()) {
        CMakeAst *package = new CMakeAst;
        if ( !CMakeListsParser::parseCMakeFile( package, path ) )
        {
            kDebug(9032) << "Found " << path << ". Analizing..." << endl;
            package->accept(this);
        } else {
            //FIXME: Put here the error.
            kDebug(9032) << "find_package. Parsing error." << endl;
        }
        //delete include; //FIXME
    } else if(pack->isRequired()) {
        //FIXME: Put here the error.
        kDebug(9032) << "Could not find " << pack->name() << " into " << modulePath << endl;
    }
    m_vars->insert(pack->name()+"-FOUND", QStringList());
    kDebug(9032) << "Found: " << pack->name() << ". Exit" << endl;
}

bool CMakeProjectVisitor::haveToFind(const QString &varName)
{
    if(m_vars->contains(varName))
        return false;
    else if(m_vars->contains(varName+"-NOTFOUND"))
        m_vars->remove(varName+"-NOTFOUND");
    return true;
}

void CMakeProjectVisitor::visit(const FindProgramAst *fprog)
{
    if(!haveToFind(fprog->variableName()))
        return;

    QStringList modulePath = resolveVariables(fprog->path(), m_vars);
    if(!fprog->noSystemEnvironmentPath())
        modulePath += envVarDirectories("PATH");

    kDebug(9032) << "Find: " << fprog->variableName() << " program into " << modulePath << endl;
    QString path=findFile(fprog->variableName(), modulePath);
    if(!path.isEmpty())
        m_vars->insert(fprog->variableName(), QStringList(path));
    else
        m_vars->insert(fprog->variableName()+"-NOTFOUND", QStringList());
}

void CMakeProjectVisitor::visit(const FindPathAst *fpath)
{
    if(!haveToFind(fpath->variableName()))
        return;

    QStringList modulePath = resolveVariables(fpath->path(), m_vars);

    kDebug(9032) << "Find: " << fpath->variableName() << " path." << endl;//FIXME Should review search
    QString path=findFile(fpath->variableName(), modulePath);
    if(!path.isEmpty())
        m_vars->insert(fpath->variableName(), QStringList(path));
    else
        m_vars->insert(fpath->variableName()+"-NOTFOUND", QStringList());
}

void CMakeProjectVisitor::visit(MacroAst *macro)
{
    kDebug(9032) << "Adding macro: " << macro->macroName() << endl;
    m_macros->insert(macro->macroName(), macro);
}

void CMakeProjectVisitor::visit(const MacroCallAst *call)
{
    if(m_macros->contains(call->name())) {
        kDebug(9032) << "Running macro: " << call->name() << endl;
        MacroAst *ast=m_macros->value(call->name());
        if(ast->knownArgs().count() == call->arguments().count()) {
            //Giving value to parameters
            QStringList::const_iterator mit = ast->knownArgs().begin();
            QStringList::const_iterator cit = call->arguments().begin();
            while(mit!=ast->knownArgs().end() && cit != call->arguments().end()) {
                m_vars->insert(*mit, QStringList(*cit));
                mit++;
                cit++;
            }

            //Executing
            visit(call->children().first());

            //Restoring
            foreach(QString name, ast->knownArgs())
                m_vars->remove(name);
        }
    } else {
        kDebug(9032) << "Did not find the macro: " << call->name() << endl;
    }
}

enum conditionToken { variable, NOT, AND, OR, COMMAND, EXISTS, IS_NEWER_THAN, IS_DIRECTORY, MATCHES,
                        LESS, GREATER, EQUAL, STRLESS, STRGREATER, STREQUAL, DEFINED };

bool condition(const QStringList &expression, const QHash<QString, QStringList> *vars)
{
    //FIXME: Should do lots of things, I'll check only if it is declared for the moment
//     return vars->contains(expression[0]);
    return true;
}

void CMakeProjectVisitor::visit(const IfAst *ifast)
{
    kDebug(9032) << "Visiting If" << ifast->conditions() << endl;
    int nConditions = ifast->conditions().count();
    int nChild = ifast->children().count();
    if(nConditions>nChild || nConditions<nChild-1) {
        kDebug(9032) << "Something weird is happening. nConditions: " << nConditions << ", nChild: " << nChild << endl;
        return;
    }

    bool done=false;

    QList<QStringList>::iterator it=ifast->conditions().begin();
    QList<CMakeAst*>::const_iterator itch=ifast->children().constBegin();

    for(; !done && itch != ifast->children().constEnd(); ++it, ++itch) {
        if(it==ifast->conditions().end() || condition(*it, m_vars)) {
            (*itch)->accept(this);
            done=true;
        }
    }
}

void CMakeProjectVisitor::visit(const ExecProgramAst *exec)
{
    //Commented until find_* work properly
    kDebug(9032) << "Executing: " << exec->executableName() << "::" << exec->arguments() << " into " << *m_vars << endl;
    /*QString execName = resolveVariable(exec->executableName(), m_vars)[0];

    KProcess p;
    p.setWorkingDirectory(exec->workingDirectory());
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, resolveVariables(exec->arguments(), m_vars));
    p.start();

    if(!p.waitForFinished())
        kDebug(9032) << "failed to execute: " << execName << endl;

    if(!exec->returnValue().isEmpty())
        m_vars->insert(exec->returnValue(), QStringList(QString(p.exitCode())));
    if(!exec->outputVariable().isEmpty()) {
        QByteArray b = p.readAllStandardOutput();
        QString t;
        t.fromAscii(b, b.length());
        QStringList res = t.split(':');
        m_vars->insert(exec->outputVariable(), res);
    }*/
}

void CMakeProjectVisitor::visit(const FileAst *file)
{
    switch(file->type()) {
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
            kDebug(9032) << "file TO_CMAKE_PATH" << " variable: " << file->variable() << " file: " << file->path() << endl;
            m_vars->insert(file->variable(), resolveVariable(file->path(), m_vars));
            kDebug(9032) << "file: " << file->variable() << "=" << m_vars->value(file->variable()) << endl;
            break;
//         case FileAst::TO_NATIVE_PATH:
//             break;
        default:
            kDebug(9032) << "error: not implemented. file: " << file->type() << " variable: " << file->variable() << " file: " << file->path() << endl;
            break;
    }
}



