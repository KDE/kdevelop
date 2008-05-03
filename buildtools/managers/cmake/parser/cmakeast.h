/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#ifndef CMAKEAST_H
#define CMAKEAST_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "cmakeexport.h"
#include "cmakelistsparser.h"
#include "cmakeastvisitor.h"

class KDEVCMAKECOMMON_EXPORT CMakeAst /*Should considerate making it abstract. */
{
public:
    CMakeAst() : m_line(-1) { }
    virtual ~CMakeAst() { /*qDeleteAll( m_children );*/ }

    /**
     * Adds a child Ast Node to this node. This will only have uses in cases
     * where custom macros are used since CMakeLists.txt files generally have
     * a pretty flat tree structure
     */
//     virtual void addChildAst( CMakeAst* node ) { m_children.append( node ); }

    /**
     * Indicates if the Ast has children
     * @return true if the Ast has children
     * @return false if the Ast has no children
     */
//     bool hasChildren() const { return m_children.isEmpty(); }

    /**
     * Get the children of this ast
     * @return the list of this ast's children
     */
//     QList<CMakeAst*> children() const  { return m_children; }
    
    virtual int accept(CMakeAstVisitor * v) const { return v->visit(this); }

    /**
     * Writes the information stored in the Ast into the @p buffer.
     * All Asts that are a child of this Ast are written back as well.
     */
    virtual void writeBack(QString& buffer) const;
    
    virtual bool isDeprecated() const { return false; }

    virtual bool parseFunctionInfo( const CMakeFunctionDesc& ) { return false; }

    
    int line() const { return m_line; }
    const CMakeFileContent & content() const { return m_content; }
    void setContent(const CMakeFileContent &cont, int nline=0) { m_content=cont; m_line=nline; }
    const QList<CMakeFunctionArgument> & outputArguments() const { return m_outputArguments; }
    private:
        CMakeAst( const CMakeAst&  ) /*: m_children( ast.m_children )*/ {}
        
        QList<CMakeFunctionArgument> m_outputArguments;
    protected:
        void addOutputArgument(const CMakeFunctionArgument& arg) { m_outputArguments.append(arg); }

        CMakeFileContent m_content;
        int m_line;

};

class KDEVCMAKECOMMON_EXPORT CustomCommandAst : public CMakeAst
{
public:
    CustomCommandAst();
    ~CustomCommandAst();

    enum BuildStage {
        PreBuild,
        PreLink,
        PostBuild };
    bool isForTarget() const { return m_forTarget; }

    QString targetName() const { return m_target; }
    void setTargetName( const QString& target ) { m_target = target; }

    BuildStage buildStage() const { return m_buildStage; }
    void setBuildStage( BuildStage bs ) { m_buildStage = bs; }

    void setOutputs( const QStringList& outputs ) { m_outputs = outputs; }
    QStringList outputs() const { return m_outputs; }

    void setCommands( const QStringList& commands ) { m_commands = commands; }
    QStringList commands() const { return m_commands; }

    void setMainDependency( const QString& mainDep ) { m_mainDep = mainDep; }
    QString mainDependency() const { return m_mainDep; }

    void setOtherDependencies( const QStringList& otherDeps) { m_otherDeps = otherDeps; }
    QStringList otherDependencies() const { return m_otherDeps; }

    void setWorkingDirectory( const QString& workingDir ) { m_workingDir = workingDir; }
    QString workingDirectory() const { return m_workingDir; }

    void setComment( const QString& comment ) { m_comment = comment; }
    QString comment() const { return m_comment; }
    
    virtual int accept(CMakeAstVisitor* visitor) const { return visitor->visit(this); }

    virtual void writeBack( QString& ) const;
    virtual bool parseFunctionInfo( const CMakeFunctionDesc& );

private:
    QString m_target;
    bool m_forTarget;
    BuildStage m_buildStage;
    QStringList m_outputs;
    QStringList m_commands;
    QString m_mainDep;
    QStringList m_otherDeps;
    QString m_workingDir;
    QString m_comment;
    bool m_isVerbatim;
    bool m_append;
    QString m_source;
};

#define CMAKE_REGISTER_AST( klassName, astId ) namespace {                 \
        CMakeAst* Create##klassName() { return new klassName; }            \
        bool b_##astId = AstFactory::self()->registerAst( QLatin1String( #astId ), Create##klassName ); }

#define CMAKE_BEGIN_AST_CLASS( klassName ) class KDEVCMAKECOMMON_EXPORT klassName : public CMakeAst {  \
    public:                                                  \
        klassName();                                         \
       ~klassName();                                         \
                                                             \
        virtual void writeBack( QString& buffer ) const;           \
        virtual int accept(CMakeAstVisitor * visitor) const { return visitor->visit(this); } \
        virtual bool parseFunctionInfo( const CMakeFunctionDesc& ); \
        QList<CMakeFunctionArgument> outputArguments;

#define CMAKE_ADD_AST_MEMBER( returnType, setterType, returnName, setterName ) \
    public:                                                         \
        returnType returnName() const { return m_##returnName; }    \
        void set##setterName( setterType );                         \
    private:                                                        \
        returnType m_##returnName;

#define CMAKE_ADD_AST_FUNCTION( function ) \
    public:                                \
       function;

#define CMAKE_MARK_AS_DEPRECATED() virtual bool isDeprecated() const { return true; }

#define CMAKE_END_AST_CLASS( klassName ) };

CMAKE_BEGIN_AST_CLASS( MacroCallAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, name, Name )
CMAKE_ADD_AST_MEMBER( QStringList, const QString&, arguments, Arguments )
CMAKE_END_AST_CLASS( MacroCallAst )

CMAKE_BEGIN_AST_CLASS( CustomTargetAst )
typedef QMap<QString, QStringList> cmdAndArgs; //Just to make preprocessor not to cry
CMAKE_ADD_AST_MEMBER( QString, const QString&, target, Target )
CMAKE_ADD_AST_MEMBER( bool, bool, buildAlways, BuildAlways )
CMAKE_ADD_AST_MEMBER( cmdAndArgs, const cmdAndArgs &, commandArgs, CommandArgs )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, dependencies, Dependencies )
CMAKE_ADD_AST_MEMBER( QString, const QString&, workingDir, WorkingDir )
CMAKE_ADD_AST_MEMBER( QString, const QString&, comment, Comment )
CMAKE_ADD_AST_MEMBER( bool, bool, isVerbatim, Verbatim )
CMAKE_END_AST_CLASS( CustomTargetAst )

CMAKE_BEGIN_AST_CLASS( AddDefinitionsAst )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, definitions, Definitions )
CMAKE_END_AST_CLASS( AddDefinitionsAst )

CMAKE_BEGIN_AST_CLASS( AddDependenciesAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, target, Target )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, dependencies, Dependencies )
CMAKE_END_AST_CLASS( AddDependenciesAst )

CMAKE_BEGIN_AST_CLASS( AddExecutableAst )
CMAKE_ADD_AST_MEMBER( QString,  const QString&, executable, Executable )
CMAKE_ADD_AST_MEMBER( bool, bool, isWin32, Win32 )
CMAKE_ADD_AST_MEMBER( bool, bool, isOsXBundle, OsXBundle )
CMAKE_ADD_AST_MEMBER( bool, bool, excludeFromAll, ExcludeFromAll )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, sourceLists, SourceLists )
CMAKE_END_AST_CLASS( AddExecutableAst )

CMAKE_BEGIN_AST_CLASS( AddLibraryAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, libraryName, LibraryName )
CMAKE_ADD_AST_MEMBER( bool, bool, isShared, IsShared )
CMAKE_ADD_AST_MEMBER( bool, bool, isStatic, IsStatic )
CMAKE_ADD_AST_MEMBER( bool, bool, isModule, IsModule )
CMAKE_ADD_AST_MEMBER( bool, bool, excludeFromAll, ExcludeFromAll )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, sourceLists, SourceLists )
CMAKE_END_AST_CLASS( AddLibraryAst )

CMAKE_BEGIN_AST_CLASS( AddSubdirectoryAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, sourceDir, SourceDir )
CMAKE_ADD_AST_MEMBER( QString, const QString&, binaryDir, BinaryDir )
CMAKE_ADD_AST_MEMBER( bool, bool, excludeFromAll, ExcludeFromAll )
CMAKE_END_AST_CLASS( AddSubdirectoryAst )


CMAKE_BEGIN_AST_CLASS( AddTestAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, testName, TestName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, exeName, exeName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, testArgs, TestArgs )
CMAKE_END_AST_CLASS( AddTestAst )


CMAKE_BEGIN_AST_CLASS( AuxSourceDirectoryAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, dirName, DirName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, variableName )
CMAKE_END_AST_CLASS( AuxSourceDirectoryAst )


CMAKE_BEGIN_AST_CLASS( BuildCommandAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, makeCommand, MakeCommand )
CMAKE_END_AST_CLASS( BuildCommandAst )


CMAKE_BEGIN_AST_CLASS( BuildNameAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, buildName, BuildName )
CMAKE_END_AST_CLASS( BuildNameAst )


CMAKE_BEGIN_AST_CLASS( CMakeMinimumRequiredAst )
CMAKE_ADD_AST_MEMBER( QList<int>, QList<int>, version, Version )
CMAKE_ADD_AST_MEMBER( bool, bool, wrongVersionIsFatal, WrongVersionIsFatal )
CMAKE_END_AST_CLASS( CMakeMinimumRequiredAst )


CMAKE_BEGIN_AST_CLASS( ConfigureFileAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, inputFile, InputFile )
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputFile, OutputFile )
CMAKE_ADD_AST_MEMBER( bool, bool, copyOnly, CopyOnly )
CMAKE_ADD_AST_MEMBER( bool, bool, escapeQuotes, EscapeQuotes )
CMAKE_ADD_AST_MEMBER( bool, bool, atsOnly, AtsOnly )
CMAKE_ADD_AST_MEMBER( bool, bool, immediate, Immediate )
CMAKE_END_AST_CLASS( ConfigureFileAst )


CMAKE_BEGIN_AST_CLASS( CreateTestSourcelistAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, name, Name )
CMAKE_ADD_AST_MEMBER( QString, const QString&, driverName, DriverName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, tests, Tests )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, extraIncludes, ExtraIncludes)
CMAKE_ADD_AST_MEMBER( QString, const QString&, function, Function)
CMAKE_END_AST_CLASS( CreateTestSourcelistAst )


CMAKE_BEGIN_AST_CLASS( EnableLanguageAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, language, Language )
CMAKE_END_AST_CLASS( EnableLanguageAst )


CMAKE_BEGIN_AST_CLASS( EnableTestingAst )
CMAKE_END_AST_CLASS( EnableTestingAst )


CMAKE_BEGIN_AST_CLASS( ExecProgramAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, const QString&, executableName, ExecutableName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, workingDirectory, WorkingDirectory )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, arguments, Arguments )
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputVariable, OutputVariable )
CMAKE_ADD_AST_MEMBER( QString, const QString&, returnValue, ReturnValue )
CMAKE_END_AST_CLASS( ExecProgramAst )


CMAKE_BEGIN_AST_CLASS( ExecuteProcessAst )
CMAKE_ADD_AST_MEMBER( QList<QStringList>, const QList<QStringList>&, commands, Commands )
CMAKE_ADD_AST_MEMBER( QString, const QString&, resultVariable, ResultVariable )
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputVariable, OutputVariable )
CMAKE_ADD_AST_MEMBER( QString, const QString&, errorVariable, ErrorVariable )
CMAKE_ADD_AST_MEMBER( QString, const QString&, inputFile, InputFile )
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputFile, OutputFile )
CMAKE_ADD_AST_MEMBER( QString, const QString&, errorFile, ErrorFile )
CMAKE_ADD_AST_MEMBER( QString, const QString&, workingDirectory, WorkingDirectory )
CMAKE_ADD_AST_MEMBER( float, float, timeout, Timeout )
CMAKE_ADD_AST_MEMBER( bool, bool, isOutputQuiet, OutputQuiet )
CMAKE_ADD_AST_MEMBER( bool, bool, isErrorQuiet, ErrorQuiet )
CMAKE_ADD_AST_MEMBER( bool, bool, isOutputStrip, OutputStrip )
CMAKE_ADD_AST_MEMBER( bool, bool, isErrorStrip, ErrorStrip )
CMAKE_END_AST_CLASS( ExecuteProcessAst )


CMAKE_BEGIN_AST_CLASS( ExportLibraryDepsAst )
CMAKE_ADD_AST_MEMBER(QString, const QString&, file, File)
CMAKE_ADD_AST_MEMBER(bool, bool, append, Append)
CMAKE_END_AST_CLASS( ExportLibraryDepsAst )


CMAKE_BEGIN_AST_CLASS( FileAst )
enum TypeFile { WRITE, APPEND, READ, GLOB, GLOB_RECURSE, REMOVE, REMOVE_RECURSE,
            MAKE_DIRECTORY, RELATIVE_PATH, TO_CMAKE_PATH, TO_NATIVE_PATH };
CMAKE_ADD_AST_MEMBER(TypeFile, TypeFile, type, Type)
CMAKE_ADD_AST_MEMBER(QString, const QString&, path, Path )
CMAKE_ADD_AST_MEMBER(QString, const QString&, variable, Variable )
CMAKE_ADD_AST_MEMBER(QString, const QString&, directory, Directory)
CMAKE_ADD_AST_MEMBER(QString, const QString&, message, Message )
CMAKE_ADD_AST_MEMBER(QStringList, const QStringList&, globbingExpressions, GlobbingExpressions )
CMAKE_ADD_AST_MEMBER(QStringList, const QStringList&, directories, Directories )
CMAKE_END_AST_CLASS( FileAst )


CMAKE_BEGIN_AST_CLASS( FindFileAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, filenames, Filenames )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, path, Path )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, pathSuffixes, PathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, const QString&, documentation, Documentation )

CMAKE_ADD_AST_MEMBER( bool, bool, noDefaultPath, NoDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeEnvironmentPath, NoCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakePath, NoCmakePath )
CMAKE_ADD_AST_MEMBER( bool, bool, noSystemEnvironmentPath, NoSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeSystemPath, NoCmakeSystemPath )
CMAKE_END_AST_CLASS( FindFileAst )


CMAKE_BEGIN_AST_CLASS( FindLibraryAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, filenames, Filenames )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, path, Path )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, pathSuffixes, PathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, const QString&, documentation, Documentation )

CMAKE_ADD_AST_MEMBER( bool, bool, noDefaultPath, NoDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeEnvironmentPath, NoCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakePath, NoCmakePath )
CMAKE_ADD_AST_MEMBER( bool, bool, noSystemEnvironmentPath, NoSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeSystemPath, NoCmakeSystemPath )
CMAKE_END_AST_CLASS( FindLibraryAst )


CMAKE_BEGIN_AST_CLASS( FindPackageAst ) //FIXME: there are things that I don't understand
CMAKE_ADD_AST_MEMBER( QString, const QString&, name, Name )
CMAKE_ADD_AST_MEMBER( int, int, minorVersion, MinorVersion )
CMAKE_ADD_AST_MEMBER( int, int, majorVersion, MajorVersion )
CMAKE_ADD_AST_MEMBER( bool, bool, isQuiet, Quiet )
CMAKE_ADD_AST_MEMBER( bool, bool, noModule, NoModule )
CMAKE_ADD_AST_MEMBER( bool, bool, isRequired, Required )
CMAKE_END_AST_CLASS( FindPackageAst )


CMAKE_BEGIN_AST_CLASS( FindPathAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, filenames, Filenames )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, path, Path )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, pathSuffixes, PathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, const QString&, documentation, Documentation )

CMAKE_ADD_AST_MEMBER( bool, bool, noDefaultPath, NoDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeEnvironmentPath, NoCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakePath, NoCmakePath )
CMAKE_ADD_AST_MEMBER( bool, bool, noSystemEnvironmentPath, NoSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeSystemPath, NoCmakeSystemPath )
CMAKE_END_AST_CLASS( FindPathAst )


CMAKE_BEGIN_AST_CLASS( FindProgramAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, filenames, Filenames )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, path, Path )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, pathSuffixes, PathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, const QString&, documentation, Documentation )

CMAKE_ADD_AST_MEMBER( bool, bool, noDefaultPath, NoDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeEnvironmentPath, NoCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakePath, NoCmakePath )
CMAKE_ADD_AST_MEMBER( bool, bool, noSystemEnvironmentPath, NoSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, bool, noCmakeSystemPath, NoCmakeSystemPath )
CMAKE_END_AST_CLASS( FindProgramAst )


CMAKE_BEGIN_AST_CLASS( FltkWrapUiAst )
CMAKE_END_AST_CLASS( FltkWrapUiAst )


CMAKE_BEGIN_AST_CLASS( ForeachAst )
    struct rangeValues { int start, stop, step; };
CMAKE_ADD_AST_MEMBER( QString, const QString&, loopVar, LoopVar )
CMAKE_ADD_AST_MEMBER( rangeValues, const rangeValues&, ranges, Ranges )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, arguments, arguments )
CMAKE_ADD_AST_MEMBER( bool, bool, range, Range )
CMAKE_END_AST_CLASS( ForeachAst )


CMAKE_BEGIN_AST_CLASS( GetCMakePropertyAst )
        enum PropertyType { VARIABLES, CACHE_VARIABLES, COMMANDS, MACROS };
CMAKE_ADD_AST_MEMBER( PropertyType, PropertyType, type, Type )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_END_AST_CLASS( GetCMakePropertyAst )


CMAKE_BEGIN_AST_CLASS( GetDirPropertyAst )
CMAKE_END_AST_CLASS( GetDirPropertyAst )


CMAKE_BEGIN_AST_CLASS( GetFilenameComponentAst )
        enum ComponentType { PATH, ABSOLUTE, NAME, EXT, NAME_WE, PROGRAM };
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, fileName, FileName )
CMAKE_ADD_AST_MEMBER( ComponentType, ComponentType, type, Type )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, programArgs, ProgramArgs )
CMAKE_ADD_AST_MEMBER( bool, bool, cache, Cache )
CMAKE_END_AST_CLASS( GetFilenameComponentAst )


CMAKE_BEGIN_AST_CLASS( GetSourceFilePropAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, filename, Filename)
CMAKE_ADD_AST_MEMBER( QString, const QString&, property, Property)
CMAKE_END_AST_CLASS( GetSourceFilePropAst )


CMAKE_BEGIN_AST_CLASS( GetTargetPropAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, target, Target)
CMAKE_ADD_AST_MEMBER( QString, const QString&, property, Property)
CMAKE_END_AST_CLASS( GetTargetPropAst )


CMAKE_BEGIN_AST_CLASS( GetTestPropAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, test, Test)
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, property, Property)
CMAKE_END_AST_CLASS( GetTestPropAst )


CMAKE_BEGIN_AST_CLASS( IfAst )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, condition, Condition )
CMAKE_ADD_AST_MEMBER( QString, const QString&, kind, Kind )
CMAKE_END_AST_CLASS( IfAst )


CMAKE_BEGIN_AST_CLASS( IncludeAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, includeFile, IncludeFile )
CMAKE_ADD_AST_MEMBER( bool, bool, optional, Optional )
CMAKE_ADD_AST_MEMBER( QString, const QString&, resultVariable, ResultVariable)
CMAKE_END_AST_CLASS( IncludeAst )


CMAKE_BEGIN_AST_CLASS( IncludeDirectoriesAst )
        enum IncludeType { DEFAULT=0, AFTER, BEFORE };
CMAKE_ADD_AST_MEMBER( IncludeType, IncludeType, includeType, IncludeType )
CMAKE_ADD_AST_MEMBER( bool, bool, isSystem, System )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, includedDirectories, IncludedDirectories )
CMAKE_END_AST_CLASS( IncludeDirectoriesAst )


CMAKE_BEGIN_AST_CLASS( IncludeExternalMsProjectAst )
CMAKE_END_AST_CLASS( IncludeExternalMsProjectAst )


CMAKE_BEGIN_AST_CLASS( IncludeRegularExpressionAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, match, Match)
CMAKE_ADD_AST_MEMBER( QString, const QString&, complain, Complain)
CMAKE_END_AST_CLASS( IncludeRegularExpressionAst )


CMAKE_BEGIN_AST_CLASS( InstallAst )
struct InstallTargetForm {    //Targets
    enum DestType { };
    QStringList targets;
    QString destination;
    QString permissions;
    QString configuration;
    QString component;
    QString optional;
};

struct InstallFileForm {
    QStringList files;
    
};
//TODO

CMAKE_ADD_AST_MEMBER(QList<InstallTargetForm>, const QList<InstallTargetForm>&, targets, Targets)
CMAKE_END_AST_CLASS( InstallAst )


CMAKE_BEGIN_AST_CLASS( InstallFilesAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER(QString, const QString&, directory, Directory)
CMAKE_ADD_AST_MEMBER(QString, const QString&, extension, Extension)
CMAKE_ADD_AST_MEMBER(QStringList, const QStringList&, files, Files)
CMAKE_ADD_AST_MEMBER(QString, const QString&, regex, Regex)
CMAKE_END_AST_CLASS( InstallFilesAst )


CMAKE_BEGIN_AST_CLASS( InstallProgramsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER(QString, const QString&, directory, Directory)
CMAKE_ADD_AST_MEMBER(QStringList, const QStringList&, files, Files)
CMAKE_ADD_AST_MEMBER(QString, const QString&, regex, Regex)
CMAKE_END_AST_CLASS( InstallProgramsAst )


CMAKE_BEGIN_AST_CLASS( InstallTargetsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER(QString, const QString&, directory, Directory)
CMAKE_ADD_AST_MEMBER(QStringList, const QStringList&, targets, Targets)
CMAKE_ADD_AST_MEMBER(QString, const QString&, runtimeDir, runtimeDir)
CMAKE_END_AST_CLASS( InstallTargetsAst )


CMAKE_BEGIN_AST_CLASS( LinkDirectoriesAst )
    CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, directories, Directories)
CMAKE_END_AST_CLASS( LinkDirectoriesAst )


CMAKE_BEGIN_AST_CLASS( LinkLibrariesAst )
    CMAKE_MARK_AS_DEPRECATED()
    enum BuildType { Optimized, Debug, None };
    typedef QPair<QString, BuildType> LibraryType;
    CMAKE_ADD_AST_MEMBER( QList<LibraryType>, const QList<LibraryType>&, libraries, Libraries)
CMAKE_END_AST_CLASS( LinkLibrariesAst )


CMAKE_BEGIN_AST_CLASS( ListAst )
    enum ListType { LENGTH, GET, APPEND, FIND, INSERT, REMOVE_ITEM, REMOVE_AT, SORT, REVERSE };
    CMAKE_ADD_AST_MEMBER( ListType, ListType, type, type)
    CMAKE_ADD_AST_MEMBER( QString, const QString&, list, List)
    CMAKE_ADD_AST_MEMBER( QString, const QString&, output, Output)
    CMAKE_ADD_AST_MEMBER( QList<int>, const QList<int> &, index, Index)
    CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, elements, Elements)
CMAKE_END_AST_CLASS( ListAst )


CMAKE_BEGIN_AST_CLASS( LoadCacheAst )
    typedef QPair<QString, QString> PrefixEntry;
    CMAKE_ADD_AST_MEMBER( QString, const QString&, cachePath, CachePath)
    CMAKE_ADD_AST_MEMBER( PrefixEntry, const PrefixEntry&, prefixes, Prefixes)
    
    CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, exclude, Exclude)
    CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, includeInternals, IncludeInternals)
CMAKE_END_AST_CLASS( LoadCacheAst )


CMAKE_BEGIN_AST_CLASS( LoadCommandAst )
    CMAKE_ADD_AST_MEMBER( QString, const QString&, cmdName, cmdName)
    CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, location, Location)
CMAKE_END_AST_CLASS( LoadCommandAst )


CMAKE_BEGIN_AST_CLASS( MacroAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, macroName, MacroName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, knownArgs, KnownArgs )
CMAKE_END_AST_CLASS( MacroAst )


CMAKE_BEGIN_AST_CLASS( MakeDirectoryAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, const QString&, directory, Directory )
CMAKE_END_AST_CLASS( MakeDirectoryAst )


CMAKE_BEGIN_AST_CLASS( MarkAsAdvancedAst )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, advancedVars, AdvancedVars)
CMAKE_ADD_AST_MEMBER( bool, bool, isClear, Clear)
CMAKE_ADD_AST_MEMBER( bool, bool, isForce, Force)
CMAKE_END_AST_CLASS( MarkAsAdvancedAst )


CMAKE_BEGIN_AST_CLASS( MathAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputVariable, OutputVariable )
CMAKE_ADD_AST_MEMBER( QString, const QString&, expression, Expression)
CMAKE_END_AST_CLASS( MathAst )


CMAKE_BEGIN_AST_CLASS( MessageAst )
enum MessageType { SEND_ERROR, STATUS, FATAL_ERROR };
CMAKE_ADD_AST_MEMBER( MessageType, MessageType, type, Type)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, message, Message )
CMAKE_END_AST_CLASS( MessageAst )


CMAKE_BEGIN_AST_CLASS( OptionAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QString, const QString&, description, Description )
CMAKE_ADD_AST_MEMBER( QString, const QString&, defaultValue, defaultValue )
CMAKE_END_AST_CLASS( OptionAst )


CMAKE_BEGIN_AST_CLASS( OutputRequiredFilesAst )
    CMAKE_ADD_AST_MEMBER( QString, const QString&, srcFile, SrcFile)
    CMAKE_ADD_AST_MEMBER( QString, const QString&, outputFile, OutputFile)
CMAKE_END_AST_CLASS( OutputRequiredFilesAst )


CMAKE_BEGIN_AST_CLASS( ProjectAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, projectName, ProjectName )
CMAKE_ADD_AST_MEMBER( bool, bool, useCpp, UseCpp )
CMAKE_ADD_AST_MEMBER( bool, bool, useC, UseC )
CMAKE_ADD_AST_MEMBER( bool, bool, useJava, UseJava )
CMAKE_END_AST_CLASS( ProjectAst )


CMAKE_BEGIN_AST_CLASS( QtWrapCppAst )
CMAKE_END_AST_CLASS( QtWrapCppAst )


CMAKE_BEGIN_AST_CLASS( QtWrapUiAst )
CMAKE_END_AST_CLASS( QtWrapUiAst )


CMAKE_BEGIN_AST_CLASS( RemoveAst )
    CMAKE_MARK_AS_DEPRECATED()
    CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
    CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, values, Values)
CMAKE_END_AST_CLASS( RemoveAst )


CMAKE_BEGIN_AST_CLASS( RemoveDefinitionsAst )
    CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, definitions, Definitions)
CMAKE_END_AST_CLASS( RemoveDefinitionsAst )


CMAKE_BEGIN_AST_CLASS( SeparateArgumentsAst )
    CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName)
CMAKE_END_AST_CLASS( SeparateArgumentsAst )


CMAKE_BEGIN_AST_CLASS( SetAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, values, Values )
CMAKE_ADD_AST_MEMBER( bool, bool, storeInCache, StoreInCache )
CMAKE_ADD_AST_MEMBER( bool, bool, forceStoring, ForceStoring )
CMAKE_ADD_AST_MEMBER( QString, const QString&, entryType, EntryType )
CMAKE_ADD_AST_MEMBER( QString, const QString&, documentation, Documentation )
CMAKE_ADD_AST_MEMBER( bool, bool, parentScope, ParentScope)
CMAKE_END_AST_CLASS( SetAst )


CMAKE_BEGIN_AST_CLASS( SetDirectoryPropsAst )
        typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QList<PropPair>, const QList<PropPair>&, properties, Properties)
CMAKE_END_AST_CLASS( SetDirectoryPropsAst )


CMAKE_BEGIN_AST_CLASS( SetSourceFilesPropsAst )
        typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, files, Files)
CMAKE_ADD_AST_MEMBER( QList<PropPair>, const QList<PropPair>&, properties, Properties)
CMAKE_END_AST_CLASS( SetSourceFilesPropsAst )


CMAKE_BEGIN_AST_CLASS( SetTargetPropsAst )
        typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, targets, Targets )
CMAKE_ADD_AST_MEMBER( QList<PropPair>, const QList<PropPair>&, properties, Properties)
CMAKE_END_AST_CLASS( SetTargetPropsAst )


CMAKE_BEGIN_AST_CLASS( SetTestsPropsAst )
        typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, tests, Tests)
CMAKE_ADD_AST_MEMBER( QList<PropPair>, const QList<PropPair>&, properties, Properties)
CMAKE_END_AST_CLASS( SetTestsPropsAst )


CMAKE_BEGIN_AST_CLASS( SiteNameAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, variableName, VariableName )
CMAKE_END_AST_CLASS( SiteNameAst )


CMAKE_BEGIN_AST_CLASS( SourceGroupAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, name, Name )
CMAKE_ADD_AST_MEMBER( QString, const QString&, regex, Regex )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, files, Files)
CMAKE_END_AST_CLASS( SourceGroupAst )


CMAKE_BEGIN_AST_CLASS( StringAst )
enum StringAstType { REGEX, REPLACE, COMPARE, ASCII, CONFIGURE,
    TOUPPER, TOLOWER, LENGTH, SUBSTRING };
enum CommandType { MATCH, MATCHALL, REGEX_REPLACE, EQUAL, NOTEQUAL, LESS, GREATER };
CMAKE_ADD_AST_MEMBER( StringAstType, StringAstType, type, Type )
CMAKE_ADD_AST_MEMBER( CommandType, CommandType, cmdType, CmdType )
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputVariable, OutputVariable )
CMAKE_ADD_AST_MEMBER( QString, const QString&, regex, Regex )
CMAKE_ADD_AST_MEMBER( QString, const QString&, replace, Replace )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, input, Input)
CMAKE_ADD_AST_MEMBER( bool, bool, only, Only )
CMAKE_ADD_AST_MEMBER( bool, bool, escapeQuotes, EscapeQuotes )
CMAKE_ADD_AST_MEMBER( int, int, begin, Begin )
CMAKE_ADD_AST_MEMBER( int, int, length, Length )
CMAKE_END_AST_CLASS( StringAst )


CMAKE_BEGIN_AST_CLASS( SubdirDependsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, const QString&, subdir, Subdir)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, directories, Directories)
CMAKE_END_AST_CLASS( SubdirDependsAst )


CMAKE_BEGIN_AST_CLASS( SubdirsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, directories, Directories)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, exluceFromAll, ExcludeFromAll)
CMAKE_ADD_AST_MEMBER( bool, bool, preorder, Preorder)
CMAKE_END_AST_CLASS( SubdirsAst )


CMAKE_BEGIN_AST_CLASS( TargetLinkLibrariesAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, target, Target )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, otherLibs, OtherLibs )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, debugLibs, DebugLibs )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, optimizedLibs, OptimizedLibs )
CMAKE_END_AST_CLASS( TargetLinkLibrariesAst )


CMAKE_BEGIN_AST_CLASS( TryCompileAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, resultName, ResultName)
CMAKE_ADD_AST_MEMBER( QString, const QString&, binDir, BinDir)
CMAKE_ADD_AST_MEMBER( QString, const QString&, source, source)
CMAKE_ADD_AST_MEMBER( QString, const QString&, projectName, ProjectName)
CMAKE_ADD_AST_MEMBER( QString, const QString&, targetName, TargetName)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, cmakeFlags, CMakeFlags)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, compileDefinitions, compileDefinitions)
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputName, OutputName)
CMAKE_ADD_AST_MEMBER( QString, const QString&, copyFile, CopyFile)
CMAKE_END_AST_CLASS( TryCompileAst )


CMAKE_BEGIN_AST_CLASS( TryRunAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, runResultVar, RunResultVar)
CMAKE_ADD_AST_MEMBER( QString, const QString&, compileResultVar, CompileResultVar )
CMAKE_ADD_AST_MEMBER( QString, const QString&, binDir, BinDir)
CMAKE_ADD_AST_MEMBER( QString, const QString&, srcFile, SrcFile )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, cmakeFlags, CMakeFlags )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, compileDefs, CompileDefs )
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputVar, OutputVar)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, args, Args )
CMAKE_END_AST_CLASS( TryRunAst )


CMAKE_BEGIN_AST_CLASS( UseMangledMesaAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, pathToMesa, PathToMesa)
CMAKE_ADD_AST_MEMBER( QString, const QString&, outputDir, OutputDir)
CMAKE_END_AST_CLASS( UseMangledMesaAst )


CMAKE_BEGIN_AST_CLASS( UtilitySourceAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, cacheEntry, CacheEntry)
CMAKE_ADD_AST_MEMBER( QString, const QString&, executableName, ExecutableName)
CMAKE_ADD_AST_MEMBER( QString, const QString&, pathToSource, PathToSource)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, fileList, FileList)
CMAKE_END_AST_CLASS( UtilitySourceAst )


CMAKE_BEGIN_AST_CLASS( VariableRequiresAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, testVariable, TestVariable)
CMAKE_ADD_AST_MEMBER( QString, const QString&, resultVariable, ResultVariable)
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, requiredVariables, requiredVariables)
CMAKE_END_AST_CLASS( VariableRequiresAst )


CMAKE_BEGIN_AST_CLASS( VtkMakeInstantiatorAst )
CMAKE_END_AST_CLASS( VtkMakeInstantiatorAst )


CMAKE_BEGIN_AST_CLASS( VtkWrapJavaAst )
CMAKE_END_AST_CLASS( VtkWrapJavaAst )


CMAKE_BEGIN_AST_CLASS( VtkWrapPythonAst )
CMAKE_END_AST_CLASS( VtkWrapPythonAst )


CMAKE_BEGIN_AST_CLASS( VtkWrapTclAst )
CMAKE_END_AST_CLASS( VtkWrapTclAst )


CMAKE_BEGIN_AST_CLASS( WhileAst )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, condition, Condition )
CMAKE_END_AST_CLASS( WhileAst )


CMAKE_BEGIN_AST_CLASS( WriteFileAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, const QString&, filename, Filename )
CMAKE_ADD_AST_MEMBER( QString, const QString&, message, Message )
CMAKE_ADD_AST_MEMBER( bool, bool, append, Append )
CMAKE_END_AST_CLASS( WriteFileAst )

CMAKE_BEGIN_AST_CLASS( CustomInvokationAst )
CMAKE_ADD_AST_MEMBER( CMakeFunctionDesc, const CMakeFunctionDesc&,
                      function, Function )
CMAKE_ADD_AST_MEMBER( QList<CMakeFunctionArgument>, const QList<CMakeFunctionArgument>&,
                      arguments, Arguments )
CMAKE_END_AST_CLASS( CustomInvokationAst )

//CMake 2.6
CMAKE_BEGIN_AST_CLASS( BreakAst )
CMAKE_END_AST_CLASS( BreakAst )

CMAKE_BEGIN_AST_CLASS( CMakePolicyAst )
enum Action { VERSION, SET, PUSH, POP };
CMAKE_ADD_AST_MEMBER( Action, Action, action, Action )
        //VERSION
        CMAKE_ADD_AST_MEMBER( QList<int>, const QList<int>&, version, Version )
        
        //SET
        CMAKE_ADD_AST_MEMBER( int, int, policyNum, PolicyNum)
        CMAKE_ADD_AST_MEMBER( bool, bool, isNew, New)
CMAKE_END_AST_CLASS( CMakePolicyAst )


CMAKE_BEGIN_AST_CLASS( ExportAst )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, targets, Targets)
CMAKE_ADD_AST_MEMBER( QString, const QString &, targetNamespace, TargetNamespace)
CMAKE_ADD_AST_MEMBER( bool, bool, append, append)
CMAKE_ADD_AST_MEMBER( QString, const QString &, filename, filename)
CMAKE_END_AST_CLASS( ExportAst )


CMAKE_BEGIN_AST_CLASS( ReturnAst )
CMAKE_END_AST_CLASS( ReturnAst )

#endif






