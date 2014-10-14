/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "cmakedebugvisitor.h"
#include "astfactory.h"
#include "../debug.h"


#define WRITEOUT qCDebug(CMAKE) << ast->line()

enum RecursivityType { No, Yes, End };

extern RecursivityType recursivity(const QString& functionName);

int CMakeAstDebugVisitor::visit( const CMakeAst * ast )
{
    WRITEOUT << "CMAKEAST: (" << "NOT IMPLEMENTED";
    return 1;
}

int CMakeAstDebugVisitor::visit( const GetCMakePropertyAst * ast )
{
    WRITEOUT << "GETCMAKEPROPERTY: (" << "NOT IMPLEMENTED";
    return 1;
}

int CMakeAstDebugVisitor::visit( const SubdirsAst * ast )
{
    WRITEOUT << "SUBDIRS: " << "(excludeFromAll, directories, preorder, isDeprecated ) = (" << ast->exluceFromAll() << "," << ast->directories() << "," << ast->preorder() << "," << ast->isDeprecated() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const StringAst * ast )
{
    WRITEOUT << "STRING: " << "(cmdType,only,outputVariable,escapeQuotes,regex,type,begin,replace,input,length) = (" << ast->cmdType() << "," << ast->only() << "," << ast->outputVariable() << "," << ast->escapeQuotes() << "," << ast->regex() << "," << ast->type() << "," << ast->begin() << "," << ast->replace() << "," << ast->input() << "," << ast->length() << "," << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const ForeachAst * ast )
{
    WRITEOUT << "FOREACH: " << "(loopVar,arguments,range,ranges-start,ranges-stop,ranges-step) = (" << ast->loopVar() << "," << ast->arguments() << "," << ast->type() << "," << ast->ranges().start << "," << ast->ranges().stop << "," << ast->ranges().step << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const ListAst * ast )
{
    WRITEOUT << "LIST: " << "(index,list,elements,output,type) = (" << ast->index() << "," << ast->list() << "," << ast->elements() << "," << ast->output() << "," << ast->type() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const OptionAst * ast )
{
    WRITEOUT << "OPTION: " << "(description,variableName,defaultValue) = (" << ast->description() << "," << ast->variableName() << "," << ast->defaultValue() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const GetFilenameComponentAst * ast )
{
    WRITEOUT << "GETFILENAMECOMPONENT: " << "(fileName,programArts,type,variableName) = (" << ast->fileName() << "," << ast->programArgs() << "," << ast->type() << "," << ast->variableName() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const MacroAst * ast )
{
    WRITEOUT << "MACRO: " << "(macroName,knownArgs) = (" << ast->macroName() << "," << ast->knownArgs() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const MathAst * ast )
{
    WRITEOUT << "MATH: " << "(outputVariable,expression) = (" << ast->outputVariable() << "," << ast->expression() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const MessageAst * ast )
{
    WRITEOUT << "MESSAGE: " << "(message,type) = (" << ast->message() << "," << ast->type() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const FileAst * ast )
{
    WRITEOUT << "FILE: " << "(type,variable,directory,path,globbingExpressions,message,directories,followSymlinks) = (" << ast->type() << "," << ast->variable() << "," << ast->directory() << "," << ast->path() << "," << ast->globbingExpressions() << "," << ast->message() << "," << ast->directories() << "," << ast->isFollowingSymlinks() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const IfAst * ast )
{
    WRITEOUT << "IF: " << "(kind,condition) = (" << ast->kind() << "," << ast->condition() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const ExecProgramAst * ast )
{
    WRITEOUT << "EXECPROGRAM: " << "(executableName,returnValue,outputVariable,arguments,workingDirectory) = (" << ast->executableName() << "," << ast->returnValue() << "," << ast->outputVariable() << "," << ast->arguments() << "," << ast->workingDirectory() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const FindFileAst * ast )
{
    WRITEOUT << "FINDFILE: " << "(filenames,noDefaultPath,noSystemEnvironmentPath,noCmakeEnvironmentPath,path,variableName,documentation,pathSuffixes,noCmakePath,noCMakeSystemPath) = (" << ast->filenames() << "," << ast->noDefaultPath() << "," << ast->noSystemEnvironmentPath() << "," << ast->noCmakeEnvironmentPath() << "," << ast->path() << "," << ast->variableName() << "," << ast->documentation() << "," << ast->pathSuffixes() << "," << ast->noCmakePath() << "," << ast->noCmakeSystemPath() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const FindLibraryAst * ast )
{
    WRITEOUT << "FINDLIBRARY: " << "(filenames,noDefaultPath,noSystemEnvironmentPath,noCmakeEnvironmentPath,path,variableName,documentation,pathSuffixes,noCmakePath,noCMakeSystemPath) = (" << ast->filenames() << "," << ast->noDefaultPath() << "," << ast->noSystemEnvironmentPath() << "," << ast->noCmakeEnvironmentPath() << "," << ast->path() << "," << ast->variableName() << "," << ast->documentation() << "," << ast->pathSuffixes() << "," << ast->noCmakePath() << "," << ast->noCmakeSystemPath() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const FindPathAst * ast )
{
    WRITEOUT << "FINDPATH: " << "(filenames,noDefaultPath,noSystemEnvironmentPath,noCmakeEnvironmentPath,path,variableName,documentation,pathSuffixes,noCmakePath,noCMakeSystemPath) = (" << ast->filenames() << "," << ast->noDefaultPath() << "," << ast->noSystemEnvironmentPath() << "," << ast->noCmakeEnvironmentPath() << "," << ast->path() << "," << ast->variableName() << "," << ast->documentation() << "," << ast->pathSuffixes() << "," << ast->noCmakePath() << "," << ast->noCmakeSystemPath() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const FindProgramAst * ast )
{
    WRITEOUT << "FINDPROGRAM: " << "(filenames,noDefaultPath,noSystemEnvironmentPath,noCMakeEnvironmentPath,path,variableName,documentation,pathSuffixes,noCmakePath,noCmakeSystemPath) = (" << ast->filenames() << "," << ast->noDefaultPath() << "," << ast->noSystemEnvironmentPath() << "," << ast->noCmakeEnvironmentPath() << "," << ast->path() << "," << ast->variableName() << "," << ast->documentation() << "," << ast->pathSuffixes() << "," << ast->noCmakePath() << "," << ast->noCmakeSystemPath() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const FindPackageAst * ast )
{
    WRITEOUT << "FINDPACKAGE: " << "(noModule,isRequired,version,isQuiet,name) = (" << ast->noModule() << "," << ast->isRequired() << "," << ast->version() << "," << ast->isQuiet() << "," << ast->name() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const MarkAsAdvancedAst * ast )
{
    WRITEOUT << "MARKASADVANCED: " << "(isClear,isForce,advancedVars) = (" << ast->isClear() << "," << ast->isForce() << "," << ast->advancedVars() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const MacroCallAst * ast )
{
    WRITEOUT << "MACROCALL: " << "(name,arguments) = (" << ast->name() << "," << ast->arguments() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const IncludeDirectoriesAst * ast )
{
    WRITEOUT << "INCLUDEDIRECTORIES: " << "(includeDirectories,isSystem,includeType) = (" << ast->includedDirectories() << "," << ast->isSystem() << "," << ast->includeType() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const ExecuteProcessAst * ast )
{
    WRITEOUT << "EXECUTEPROCESS: " << "(errorFile,isOutputQuiet,isOutputStrip,errorVariable,outputFile,outputVariable,inputFile,resultVariable,isErrorQuiet,timeout,workingDirectory,commands) = (" << ast->errorFile() << "," << ast->isOutputQuiet() << "," << ast->isOutputStrip() << "," << ast->errorVariable() << "," << ast->outputFile() << "," << ast->outputVariable() << "," << ast->inputFile() << "," << ast->resultVariable() << "," << ast->isErrorQuiet() << "," << ast->timeout() << "," << ast->workingDirectory() << "," << ast->commands() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const ProjectAst * ast )
{
    WRITEOUT << "PROJECT: " << "(projectname,isCpp,isC,isJava) = (" << ast->projectName() << "," << ast->useCpp() << "," << "," << ast->useC() << "," << "," << ast->useJava() << "," << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const SetAst * ast )
{
    WRITEOUT << "SET: " << "(entryType,forceStoring,storeInCache,documentation,values,variableName) = (" << ast->entryType() << "," << ast->forceStoring() << "," << ast->storeInCache() << "," << ast->documentation() << "," << ast->values() << "," << ast->variableName() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const IncludeAst * ast )
{
    WRITEOUT << "INCLUDE: " << "(optional,resultVariable,includeFile) = (" << ast->optional() << "," << ast->resultVariable() << "," << ast->includeFile() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const ConfigureFileAst * ast )
{
    WRITEOUT << "CONFIGUREFILE: " << "(outputFile,escapeQuotes,copyOnly,immediate,atsOnly,inputFile) = (" << ast->outputFile() << "," << ast->escapeQuotes() << "," << ast->copyOnly() << "," << ast->immediate() << "," << ast->atsOnly() << "," << ast->inputFile() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const CMakeMinimumRequiredAst * ast )
{
    WRITEOUT << "CMAKEMINIMUMREQUIRED: " << "(wrongVersionIsFatal,version) = (" << ast->wrongVersionIsFatal() << "," << ast->version() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const BuildNameAst * ast )
{
    WRITEOUT << "BUILDNAME: " << "(buildName) = (" << ast->buildName() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const BuildCommandAst * ast )
{
    WRITEOUT << "BUILDCOMMAND: " << "(makeCommand,variableName) = (" << ast->makeCommand() << "," << ast->variableName() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const AuxSourceDirectoryAst * ast )
{
    WRITEOUT << "AUXSOURCEDIRECTORY: " << "(dirName,variableName) = (" << ast->dirName() << "," << ast->variableName() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const AddTestAst * ast )
{
    WRITEOUT << "ADDTEST: " << "(testArgs,exeName,testName) = (" << ast->testArgs() << "," << ast->exeName() << "," << ast->testName() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const AddSubdirectoryAst * ast )
{
    WRITEOUT << "ADDSUBDIRECTORY: " << "(excludeFromAll,sourcedir,binaryDir) = (" << ast->excludeFromAll() << "," << ast->sourceDir() << "," << ast->binaryDir() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const AddLibraryAst * ast )
{
    WRITEOUT << "ADDLIBRARY: " << "(type,excludeFromAll,libraryName,sourceLists) = (" << ast->type() << "," << ast->excludeFromAll() << "," << ast->libraryName() << "," << ast->sourceLists() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const AddExecutableAst * ast )
{
    WRITEOUT << "ADDEXECUTABLE: " << "(executable,isOSXBundle,excludeFromAll,isWin32,sourceLists) = (" << ast->executable() << "," << ast->isOsXBundle() << "," << ast->excludeFromAll() << "," << ast->isWin32() << "," << ast->sourceLists() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const AddDependenciesAst * ast )
{
    WRITEOUT << "ADDDEPENDECIES: " << "(dependecies,target) = (" << ast->dependencies() << "," << ast->target() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const AddDefinitionsAst * ast )
{
    WRITEOUT << "ADDEFINITIONS: " << "(definitions) = (" << ast->definitions() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const CustomTargetAst * ast )
{
    WRITEOUT << "CUSTOMTARGET: " << "(target,workingDir,commandArgs,comment,dependecies,buildAlways,isVerbatim) = (" << ast->target() << "," << "," << ast->workingDir() << "," << "," << ast->commandArgs() << "," << "," << ast->comment() << "," << ast->dependencies() << "," << ast->buildAlways() << "," << ast->isVerbatim() << ")";
    return 1;
}

int CMakeAstDebugVisitor::visit( const CustomCommandAst * ast )
{
    WRITEOUT << "CUSTOMCOMMAND: " << "(otherDependecies,outputs,targetName,workingDirectory,commands,comment,isForTarget,buildStage,mainDependecy) = (" << ast->otherDependencies() << "," << "," << ast->outputs() << "," << "," << ast->targetName() << "," << "," << ast->workingDirectory() << "," << "," << ast->commands() << "," << "," << ast->comment() << "," << "," << ast->isForTarget() << "," << "," << ast->buildStage() << "," << "," << ast->mainDependency() << ")";
    return 1;
}

CMakeAstDebugVisitor::~CMakeAstDebugVisitor( )
{
}

CMakeAstDebugVisitor::CMakeAstDebugVisitor( )
{
}

int CMakeAstDebugVisitor::walk(const QString& filename, const CMakeFileContent & fc, int line)
{
    qCDebug(CMAKE) << "-----------------------------------------------------------";
    qCDebug(CMAKE) << "Walking file:" << filename;
    CMakeFileContent::const_iterator it=fc.constBegin()+line, itEnd=fc.constEnd();
    for(; it!=itEnd; )
    {
        Q_ASSERT( line<fc.count() );
        Q_ASSERT( line>=0 );
//         qCDebug(CMAKE) << "@" << line;
//         qCDebug(CMAKE) << it->writeBack() << "==" << fc[line].writeBack();
        Q_ASSERT( *it == fc[line] );
//         qCDebug(CMAKE) << "At line" << line << "/" << fc.count();
        CMakeAst* element = AstFactory::self()->createAst(it->name);

        if(!element)
        {
            element = new MacroCallAst;
        }

        CMakeFunctionDesc func = *it;

        QString funcName=func.name;
        bool correct = element->parseFunctionInfo(func);
        if(!correct)
        {
            qCDebug(CMAKE) << "error! found an error while processing" << func.writeBack() << "was" << it->writeBack() << endl <<
                    " at" << func.filePath << ":" << func.line << endl;
            //FIXME: Should avoid to run
        }

        RecursivityType r = recursivity(funcName);
        if(r==End)
        {
//             qCDebug(CMAKE) << "Found an end." << func.writeBack();
            delete element;
            return line;
        }
        if(element->isDeprecated())
            qCDebug(CMAKE) << "Warning: Using the function: " << funcName << " which is deprecated by cmake.";
        element->setContent(fc, line);


        int lines=element->accept(this);

        line+=lines;
        it+=lines;
        delete element;
    }
    qCDebug(CMAKE) << "Walk stopped @" << line;
    qCDebug(CMAKE) << "-----------------------------------------------------------";
    return line;
}

