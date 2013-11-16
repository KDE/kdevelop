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
#include "cmaketypes.h"

class KDEVCMAKECOMMON_EXPORT CMakeAst /*Should consider making it abstract. */
{
    public:
        CMakeAst() : m_line(-1) { }
        virtual ~CMakeAst() { /*qDeleteAll( m_children );*/ }

        virtual int accept(CMakeAstVisitor * v) const { return v->visit(this); }

        /**
         * Returns \c true if this command is deprecated.
         */
        virtual bool isDeprecated() const { return false; }

        virtual bool parseFunctionInfo( const CMakeFunctionDesc& func )=0;

        int line() const { return m_line; }
        const CMakeFileContent & content() const { return m_content; }
        void setContent(const CMakeFileContent &cont, int nline=0) { m_content=cont; m_line=nline; }
        const QList<CMakeFunctionArgument> & outputArguments() const { return m_outputArguments; }
    private:
        CMakeAst( const CMakeAst&  ) {}

        QList<CMakeFunctionArgument> m_outputArguments;
        CMakeFileContent m_content;

        int m_line;
    protected:
        void addOutputArgument(const CMakeFunctionArgument& arg) { m_outputArguments.append(arg); }
};

#define CMAKE_BEGIN_AST_CLASS( klassName ) class klassName : public CMakeAst {  \
    public:                                                  \
        klassName();                                         \
       ~klassName();                                         \
                                                             \
        virtual int accept(CMakeAstVisitor * visitor) const { return visitor->visit(this); } \
        virtual bool parseFunctionInfo( const CMakeFunctionDesc& func );

#define CMAKE_ADD_AST_MEMBER( returnType, returnName )              \
    public:                                                         \
        returnType returnName() const { return m_##returnName; }    \
    private:                                                        \
        returnType m_##returnName;

#define CMAKE_MARK_AS_DEPRECATED() virtual bool isDeprecated() const { return true; }

#define CMAKE_END_AST_CLASS( klassName ) };

CMAKE_BEGIN_AST_CLASS( CustomCommandAst )
    enum BuildStage {
        PreBuild,
        PreLink,
        PostBuild };

    CMAKE_ADD_AST_MEMBER( bool, isForTarget )
    CMAKE_ADD_AST_MEMBER( QString, targetName )
    CMAKE_ADD_AST_MEMBER( BuildStage, buildStage )
    CMAKE_ADD_AST_MEMBER( QStringList, outputs )
    CMAKE_ADD_AST_MEMBER( QStringList, commands )
    CMAKE_ADD_AST_MEMBER( QString, mainDependency )
    CMAKE_ADD_AST_MEMBER( QStringList, otherDependencies )
    CMAKE_ADD_AST_MEMBER( QString, workingDirectory )
    CMAKE_ADD_AST_MEMBER( QString, comment )
    CMAKE_ADD_AST_MEMBER( QString, source )
    CMAKE_ADD_AST_MEMBER( bool, isVerbatim )
    CMAKE_ADD_AST_MEMBER( bool, append )
CMAKE_END_AST_CLASS( CustomCommandAst )

CMAKE_BEGIN_AST_CLASS( MacroCallAst )
CMAKE_ADD_AST_MEMBER( QString, name )
CMAKE_ADD_AST_MEMBER( QStringList, arguments )
CMAKE_END_AST_CLASS( MacroCallAst )

CMAKE_BEGIN_AST_CLASS( CustomTargetAst )
typedef QMap<QString, QStringList> cmdAndArgs; //Just to make preprocessor not cry
CMAKE_ADD_AST_MEMBER( QString, target )
CMAKE_ADD_AST_MEMBER( bool, buildAlways )
CMAKE_ADD_AST_MEMBER( cmdAndArgs, commandArgs )
CMAKE_ADD_AST_MEMBER( QStringList, dependencies )
CMAKE_ADD_AST_MEMBER( QString, workingDir )
CMAKE_ADD_AST_MEMBER( QString, comment )
CMAKE_ADD_AST_MEMBER( bool, isVerbatim )
CMAKE_ADD_AST_MEMBER( QStringList, sourceLists )
CMAKE_END_AST_CLASS( CustomTargetAst )

CMAKE_BEGIN_AST_CLASS( AddDefinitionsAst )
CMAKE_ADD_AST_MEMBER( QStringList, definitions )
CMAKE_END_AST_CLASS( AddDefinitionsAst )

CMAKE_BEGIN_AST_CLASS( AddDependenciesAst )
CMAKE_ADD_AST_MEMBER( QString, target )
CMAKE_ADD_AST_MEMBER( QStringList, dependencies )
CMAKE_END_AST_CLASS( AddDependenciesAst )

CMAKE_BEGIN_AST_CLASS( AddExecutableAst )
CMAKE_ADD_AST_MEMBER( QString, executable )
CMAKE_ADD_AST_MEMBER( bool, isWin32 )
CMAKE_ADD_AST_MEMBER( bool, isOsXBundle )
CMAKE_ADD_AST_MEMBER( bool, excludeFromAll )
CMAKE_ADD_AST_MEMBER( bool, isImported )
CMAKE_ADD_AST_MEMBER( QStringList, sourceLists )
CMAKE_END_AST_CLASS( AddExecutableAst )

CMAKE_BEGIN_AST_CLASS( AddLibraryAst )
enum LibraryType {Shared, Static, Module, Object, Unknown};
CMAKE_ADD_AST_MEMBER( QString, libraryName )
CMAKE_ADD_AST_MEMBER( LibraryType, type )
CMAKE_ADD_AST_MEMBER( bool, isImported )
CMAKE_ADD_AST_MEMBER( bool, isAlias )
CMAKE_ADD_AST_MEMBER( QString, aliasTarget )
CMAKE_ADD_AST_MEMBER( bool, excludeFromAll )
CMAKE_ADD_AST_MEMBER( QStringList, sourceLists )
static QMap<QString, LibraryType> s_typeForName;
CMAKE_END_AST_CLASS( AddLibraryAst )

CMAKE_BEGIN_AST_CLASS( AddSubdirectoryAst )
CMAKE_ADD_AST_MEMBER( QString, sourceDir )
CMAKE_ADD_AST_MEMBER( QString, binaryDir )
CMAKE_ADD_AST_MEMBER( bool, excludeFromAll )
CMAKE_END_AST_CLASS( AddSubdirectoryAst )


CMAKE_BEGIN_AST_CLASS( AddTestAst )
CMAKE_ADD_AST_MEMBER( QString, testName )
CMAKE_ADD_AST_MEMBER( QString, exeName )
CMAKE_ADD_AST_MEMBER( QStringList, testArgs )
CMAKE_END_AST_CLASS( AddTestAst )


CMAKE_BEGIN_AST_CLASS( AuxSourceDirectoryAst )
CMAKE_ADD_AST_MEMBER( QString, dirName )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_END_AST_CLASS( AuxSourceDirectoryAst )


CMAKE_BEGIN_AST_CLASS( BuildCommandAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QString, makeCommand )
CMAKE_END_AST_CLASS( BuildCommandAst )


CMAKE_BEGIN_AST_CLASS( BuildNameAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, buildName )
CMAKE_END_AST_CLASS( BuildNameAst )


CMAKE_BEGIN_AST_CLASS( CMakeMinimumRequiredAst )
CMAKE_ADD_AST_MEMBER( QList<int>, version )
CMAKE_ADD_AST_MEMBER( bool, wrongVersionIsFatal )
CMAKE_END_AST_CLASS( CMakeMinimumRequiredAst )


CMAKE_BEGIN_AST_CLASS( ConfigureFileAst )
CMAKE_ADD_AST_MEMBER( QString, inputFile )
CMAKE_ADD_AST_MEMBER( QString, outputFile )
CMAKE_ADD_AST_MEMBER( bool, copyOnly )
CMAKE_ADD_AST_MEMBER( bool, escapeQuotes )
CMAKE_ADD_AST_MEMBER( bool, atsOnly )
CMAKE_ADD_AST_MEMBER( bool, immediate )
CMAKE_END_AST_CLASS( ConfigureFileAst )


CMAKE_BEGIN_AST_CLASS( CreateTestSourcelistAst )
CMAKE_ADD_AST_MEMBER( QString, name )
CMAKE_ADD_AST_MEMBER( QString, driverName )
CMAKE_ADD_AST_MEMBER( QStringList, tests )
CMAKE_ADD_AST_MEMBER( QStringList, extraIncludes )
CMAKE_ADD_AST_MEMBER( QString, function )
CMAKE_END_AST_CLASS( CreateTestSourcelistAst )


CMAKE_BEGIN_AST_CLASS( EnableLanguageAst )
CMAKE_ADD_AST_MEMBER( QString, language )
CMAKE_END_AST_CLASS( EnableLanguageAst )


CMAKE_BEGIN_AST_CLASS( EnableTestingAst )
CMAKE_END_AST_CLASS( EnableTestingAst )


CMAKE_BEGIN_AST_CLASS( ExecProgramAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, executableName )
CMAKE_ADD_AST_MEMBER( QString, workingDirectory )
CMAKE_ADD_AST_MEMBER( QStringList, arguments )
CMAKE_ADD_AST_MEMBER( QString, outputVariable )
CMAKE_ADD_AST_MEMBER( QString, returnValue )
CMAKE_END_AST_CLASS( ExecProgramAst )


CMAKE_BEGIN_AST_CLASS( ExecuteProcessAst )
CMAKE_ADD_AST_MEMBER( QList<QStringList>, commands )
CMAKE_ADD_AST_MEMBER( QString, resultVariable )
CMAKE_ADD_AST_MEMBER( QString, outputVariable )
CMAKE_ADD_AST_MEMBER( QString, errorVariable )
CMAKE_ADD_AST_MEMBER( QString, inputFile )
CMAKE_ADD_AST_MEMBER( QString, outputFile )
CMAKE_ADD_AST_MEMBER( QString, errorFile )
CMAKE_ADD_AST_MEMBER( QString, workingDirectory )
CMAKE_ADD_AST_MEMBER( float, timeout )
CMAKE_ADD_AST_MEMBER( bool, isOutputQuiet )
CMAKE_ADD_AST_MEMBER( bool, isErrorQuiet )
CMAKE_ADD_AST_MEMBER( bool, isOutputStrip )
CMAKE_ADD_AST_MEMBER( bool, isErrorStrip )
CMAKE_END_AST_CLASS( ExecuteProcessAst )


CMAKE_BEGIN_AST_CLASS( ExportLibraryDepsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, file )
CMAKE_ADD_AST_MEMBER( bool, append )
CMAKE_END_AST_CLASS( ExportLibraryDepsAst )


CMAKE_BEGIN_AST_CLASS( FileAst )
enum TypeFile { Write, Append, Read, Glob, GlobRecurse, Remove, RemoveRecurse,
            MakeDirectory, RelativePath, ToCmakePath, ToNativePath, Strings, Download };
CMAKE_ADD_AST_MEMBER( TypeFile, type )
CMAKE_ADD_AST_MEMBER( QString, path )
CMAKE_ADD_AST_MEMBER( QString, variable )
CMAKE_ADD_AST_MEMBER( QString, directory )
CMAKE_ADD_AST_MEMBER( QString, message )
CMAKE_ADD_AST_MEMBER( QStringList, globbingExpressions )
CMAKE_ADD_AST_MEMBER( bool, isFollowingSymlinks )
CMAKE_ADD_AST_MEMBER( QStringList, directories )

CMAKE_ADD_AST_MEMBER( KUrl, url )
CMAKE_ADD_AST_MEMBER( int, timeout )

CMAKE_ADD_AST_MEMBER( int, limitCount )
CMAKE_ADD_AST_MEMBER( int, limitInput )
CMAKE_ADD_AST_MEMBER( int, limitOutput )
CMAKE_ADD_AST_MEMBER( int, lengthMinimum )
CMAKE_ADD_AST_MEMBER( int, lengthMaximum )
CMAKE_ADD_AST_MEMBER( bool, newlineConsume )
CMAKE_ADD_AST_MEMBER( bool, noHexConversion )
CMAKE_ADD_AST_MEMBER( QString, regex )
CMAKE_END_AST_CLASS( FileAst )


CMAKE_BEGIN_AST_CLASS( FindFileAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QStringList, filenames )
CMAKE_ADD_AST_MEMBER( QStringList, path )
CMAKE_ADD_AST_MEMBER( QStringList, hints )
CMAKE_ADD_AST_MEMBER( QStringList, pathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, documentation )

CMAKE_ADD_AST_MEMBER( bool, noDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakePath )
CMAKE_ADD_AST_MEMBER( bool, noSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeSystemPath )
CMAKE_END_AST_CLASS( FindFileAst )


CMAKE_BEGIN_AST_CLASS( FindLibraryAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QStringList, filenames )
CMAKE_ADD_AST_MEMBER( QStringList, path )
CMAKE_ADD_AST_MEMBER( QStringList, hints )
CMAKE_ADD_AST_MEMBER( QStringList, pathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, documentation )

CMAKE_ADD_AST_MEMBER( bool, noDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakePath )
CMAKE_ADD_AST_MEMBER( bool, noSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeSystemPath )
CMAKE_ADD_AST_MEMBER( bool, cmakeFindRootPath )
CMAKE_END_AST_CLASS( FindLibraryAst )


CMAKE_BEGIN_AST_CLASS( FindPackageAst ) //FIXME: there are things that I don't understand
CMAKE_ADD_AST_MEMBER( QString, name )
CMAKE_ADD_AST_MEMBER( QString, version )
CMAKE_ADD_AST_MEMBER( QStringList, components )
CMAKE_ADD_AST_MEMBER( QStringList, paths )
CMAKE_ADD_AST_MEMBER( bool, isQuiet )
CMAKE_ADD_AST_MEMBER( bool, noModule )
CMAKE_ADD_AST_MEMBER( bool, isRequired )
CMAKE_END_AST_CLASS( FindPackageAst )


CMAKE_BEGIN_AST_CLASS( FindPathAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QStringList, filenames )
CMAKE_ADD_AST_MEMBER( QStringList, path )
CMAKE_ADD_AST_MEMBER( QStringList, hints )
CMAKE_ADD_AST_MEMBER( QStringList, pathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, documentation )

CMAKE_ADD_AST_MEMBER( bool, noDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakePath )
CMAKE_ADD_AST_MEMBER( bool, noSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeSystemPath )
CMAKE_END_AST_CLASS( FindPathAst )


CMAKE_BEGIN_AST_CLASS( FindProgramAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QStringList, filenames )
CMAKE_ADD_AST_MEMBER( QStringList, path )
CMAKE_ADD_AST_MEMBER( QStringList, hints )
CMAKE_ADD_AST_MEMBER( QStringList, pathSuffixes )
CMAKE_ADD_AST_MEMBER( QString, documentation )

CMAKE_ADD_AST_MEMBER( bool, noDefaultPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakePath )
CMAKE_ADD_AST_MEMBER( bool, noSystemEnvironmentPath )
CMAKE_ADD_AST_MEMBER( bool, noCmakeSystemPath )
CMAKE_ADD_AST_MEMBER( bool, noCMakeFindRootPath )
CMAKE_END_AST_CLASS( FindProgramAst )


CMAKE_BEGIN_AST_CLASS( FltkWrapUiAst )
CMAKE_END_AST_CLASS( FltkWrapUiAst )


CMAKE_BEGIN_AST_CLASS( ForeachAst )
    enum ForeachType { Range, InItems, InLists };
    struct rangeValues { int start, stop, step; };
CMAKE_ADD_AST_MEMBER( QString, loopVar )
CMAKE_ADD_AST_MEMBER( rangeValues, ranges )
CMAKE_ADD_AST_MEMBER( QStringList, arguments )
CMAKE_ADD_AST_MEMBER( ForeachType, type )
CMAKE_END_AST_CLASS( ForeachAst )


CMAKE_BEGIN_AST_CLASS( GetCMakePropertyAst )
    enum PropertyType { Variables, CacheVariables, Commands, Macros, Components };
CMAKE_ADD_AST_MEMBER( PropertyType, type )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_END_AST_CLASS( GetCMakePropertyAst )


CMAKE_BEGIN_AST_CLASS( SetPropertyAst )
CMAKE_ADD_AST_MEMBER( PropertyType, type )
CMAKE_ADD_AST_MEMBER( bool, append )
CMAKE_ADD_AST_MEMBER( QStringList, args )
CMAKE_ADD_AST_MEMBER( QString, name )
CMAKE_ADD_AST_MEMBER( QStringList, values )
CMAKE_END_AST_CLASS( SetPropertyAst )


CMAKE_BEGIN_AST_CLASS( GetPropertyAst )
CMAKE_ADD_AST_MEMBER( PropertyType, type )
CMAKE_ADD_AST_MEMBER( QString, outputVariable )
CMAKE_ADD_AST_MEMBER( QString, typeName )
CMAKE_ADD_AST_MEMBER( QString, name )

enum Behaviour { None, Set, Defined, BriefDocs, FullDocs };
CMAKE_ADD_AST_MEMBER( Behaviour, behaviour )
CMAKE_END_AST_CLASS( GetPropertyAst )


CMAKE_BEGIN_AST_CLASS( GetDirPropertyAst )
CMAKE_ADD_AST_MEMBER( QString, propName )
CMAKE_ADD_AST_MEMBER( QString, outputVariable )
CMAKE_ADD_AST_MEMBER( QString, directory )
CMAKE_END_AST_CLASS( GetDirPropertyAst )


CMAKE_BEGIN_AST_CLASS( GetFilenameComponentAst )
    enum ComponentType { Path, RealPath, Absolute, Name, Ext, NameWe, Program };
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QString, fileName )
CMAKE_ADD_AST_MEMBER( ComponentType, type )
CMAKE_ADD_AST_MEMBER( QStringList, programArgs )
CMAKE_ADD_AST_MEMBER( bool, cache )
CMAKE_END_AST_CLASS( GetFilenameComponentAst )


CMAKE_BEGIN_AST_CLASS( GetSourceFilePropAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QString, filename )
CMAKE_ADD_AST_MEMBER( QString, property )
CMAKE_END_AST_CLASS( GetSourceFilePropAst )


CMAKE_BEGIN_AST_CLASS( GetTargetPropAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QString, target )
CMAKE_ADD_AST_MEMBER( QString, property )
CMAKE_END_AST_CLASS( GetTargetPropAst )


CMAKE_BEGIN_AST_CLASS( GetTestPropAst )
CMAKE_ADD_AST_MEMBER( QString, test )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QString, property )
CMAKE_END_AST_CLASS( GetTestPropAst )


CMAKE_BEGIN_AST_CLASS( IfAst )
CMAKE_ADD_AST_MEMBER( QStringList, condition )
CMAKE_ADD_AST_MEMBER( QString, kind )
CMAKE_END_AST_CLASS( IfAst )


CMAKE_BEGIN_AST_CLASS( IncludeAst )
CMAKE_ADD_AST_MEMBER( QString, includeFile )
CMAKE_ADD_AST_MEMBER( bool, optional )
CMAKE_ADD_AST_MEMBER( QString, resultVariable )
CMAKE_END_AST_CLASS( IncludeAst )


CMAKE_BEGIN_AST_CLASS( IncludeDirectoriesAst )
    enum IncludeType { Default=0, After, Before };
CMAKE_ADD_AST_MEMBER( IncludeType, includeType )
CMAKE_ADD_AST_MEMBER( bool, isSystem )
CMAKE_ADD_AST_MEMBER( QStringList, includedDirectories )
CMAKE_END_AST_CLASS( IncludeDirectoriesAst )


CMAKE_BEGIN_AST_CLASS( IncludeExternalMsProjectAst )
CMAKE_END_AST_CLASS( IncludeExternalMsProjectAst )


CMAKE_BEGIN_AST_CLASS( IncludeRegularExpressionAst )
CMAKE_ADD_AST_MEMBER( QString, match )
CMAKE_ADD_AST_MEMBER( QString, complain )
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

CMAKE_ADD_AST_MEMBER( QList<InstallTargetForm>, targets )
CMAKE_END_AST_CLASS( InstallAst )


CMAKE_BEGIN_AST_CLASS( InstallFilesAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, directory )
CMAKE_ADD_AST_MEMBER( QString, extension )
CMAKE_ADD_AST_MEMBER( QStringList, files )
CMAKE_ADD_AST_MEMBER( QString, regex )
CMAKE_END_AST_CLASS( InstallFilesAst )


CMAKE_BEGIN_AST_CLASS( InstallProgramsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, directory )
CMAKE_ADD_AST_MEMBER( QStringList, files )
CMAKE_ADD_AST_MEMBER( QString, regex )
CMAKE_END_AST_CLASS( InstallProgramsAst )


CMAKE_BEGIN_AST_CLASS( InstallTargetsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, directory )
CMAKE_ADD_AST_MEMBER( QStringList, targets )
CMAKE_ADD_AST_MEMBER( QString, runtimeDir )
CMAKE_END_AST_CLASS( InstallTargetsAst )


CMAKE_BEGIN_AST_CLASS( LinkDirectoriesAst )
    CMAKE_ADD_AST_MEMBER( QStringList, directories )
CMAKE_END_AST_CLASS( LinkDirectoriesAst )


CMAKE_BEGIN_AST_CLASS( LinkLibrariesAst )
    CMAKE_MARK_AS_DEPRECATED()
    enum BuildType { Optimized, Debug, None };
    typedef QPair<QString, BuildType> LibraryType;
    CMAKE_ADD_AST_MEMBER( QList<LibraryType>, libraries )
CMAKE_END_AST_CLASS( LinkLibrariesAst )


CMAKE_BEGIN_AST_CLASS( ListAst )
    enum ListType { Length, Get, Append, Find, Insert, RemoveItem, RemoveAt, Sort, Reverse, RemoveDuplicates };
    CMAKE_ADD_AST_MEMBER( ListType, type )
    CMAKE_ADD_AST_MEMBER( QString, list )
    CMAKE_ADD_AST_MEMBER( QString, output )
    CMAKE_ADD_AST_MEMBER( QList<int>, index )
    CMAKE_ADD_AST_MEMBER( QStringList, elements )
CMAKE_END_AST_CLASS( ListAst )


CMAKE_BEGIN_AST_CLASS( LoadCacheAst )
    typedef QPair<QString, QString> PrefixEntry;
    CMAKE_ADD_AST_MEMBER( QString, cachePath )
    CMAKE_ADD_AST_MEMBER( PrefixEntry, prefixes )
    
    CMAKE_ADD_AST_MEMBER( QStringList, exclude )
    CMAKE_ADD_AST_MEMBER( QStringList, includeInternals )
CMAKE_END_AST_CLASS( LoadCacheAst )


CMAKE_BEGIN_AST_CLASS( LoadCommandAst )
    CMAKE_ADD_AST_MEMBER( QString, cmdName )
    CMAKE_ADD_AST_MEMBER( QStringList, location )
CMAKE_END_AST_CLASS( LoadCommandAst )


CMAKE_BEGIN_AST_CLASS( MacroAst )
CMAKE_ADD_AST_MEMBER( QString, macroName )
CMAKE_ADD_AST_MEMBER( QStringList, knownArgs )
CMAKE_END_AST_CLASS( MacroAst )


CMAKE_BEGIN_AST_CLASS( MakeDirectoryAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, directory )
CMAKE_END_AST_CLASS( MakeDirectoryAst )


CMAKE_BEGIN_AST_CLASS( MarkAsAdvancedAst )
CMAKE_ADD_AST_MEMBER( QStringList, advancedVars )
CMAKE_ADD_AST_MEMBER( bool, isClear )
CMAKE_ADD_AST_MEMBER( bool, isForce )
CMAKE_END_AST_CLASS( MarkAsAdvancedAst )


CMAKE_BEGIN_AST_CLASS( MathAst )
CMAKE_ADD_AST_MEMBER( QString, outputVariable )
CMAKE_ADD_AST_MEMBER( QString, expression )
CMAKE_END_AST_CLASS( MathAst )


CMAKE_BEGIN_AST_CLASS( MessageAst )
enum MessageType { SendError, Status, FatalError };
CMAKE_ADD_AST_MEMBER( MessageType, type )
CMAKE_ADD_AST_MEMBER( QStringList, message )
CMAKE_END_AST_CLASS( MessageAst )


CMAKE_BEGIN_AST_CLASS( OptionAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QString, description )
CMAKE_ADD_AST_MEMBER( QString, defaultValue )
CMAKE_END_AST_CLASS( OptionAst )


CMAKE_BEGIN_AST_CLASS( OutputRequiredFilesAst )
    CMAKE_ADD_AST_MEMBER( QString, srcFile )
    CMAKE_ADD_AST_MEMBER( QString, outputFile )
CMAKE_END_AST_CLASS( OutputRequiredFilesAst )


CMAKE_BEGIN_AST_CLASS( ProjectAst )
CMAKE_ADD_AST_MEMBER( QString, projectName )
CMAKE_ADD_AST_MEMBER( bool, useCpp )
CMAKE_ADD_AST_MEMBER( bool, useC )
CMAKE_ADD_AST_MEMBER( bool, useJava )
CMAKE_END_AST_CLASS( ProjectAst )


CMAKE_BEGIN_AST_CLASS( QtWrapCppAst )
CMAKE_END_AST_CLASS( QtWrapCppAst )


CMAKE_BEGIN_AST_CLASS( QtWrapUiAst )
CMAKE_END_AST_CLASS( QtWrapUiAst )


CMAKE_BEGIN_AST_CLASS( RemoveAst )
    CMAKE_MARK_AS_DEPRECATED()
    CMAKE_ADD_AST_MEMBER( QString, variableName )
    CMAKE_ADD_AST_MEMBER( QStringList, values )
CMAKE_END_AST_CLASS( RemoveAst )


CMAKE_BEGIN_AST_CLASS( RemoveDefinitionsAst )
    CMAKE_ADD_AST_MEMBER( QStringList, definitions )
CMAKE_END_AST_CLASS( RemoveDefinitionsAst )


CMAKE_BEGIN_AST_CLASS( SeparateArgumentsAst )
    CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_END_AST_CLASS( SeparateArgumentsAst )


CMAKE_BEGIN_AST_CLASS( SetAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( QStringList, values )
CMAKE_ADD_AST_MEMBER( bool, storeInCache )
CMAKE_ADD_AST_MEMBER( bool, forceStoring )
CMAKE_ADD_AST_MEMBER( QString, entryType )
CMAKE_ADD_AST_MEMBER( QString, documentation )
CMAKE_ADD_AST_MEMBER( bool, parentScope )
CMAKE_END_AST_CLASS( SetAst )

CMAKE_BEGIN_AST_CLASS( UnsetAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_ADD_AST_MEMBER( bool, cache )
CMAKE_ADD_AST_MEMBER( bool, env )
CMAKE_END_AST_CLASS( SetAst )

CMAKE_BEGIN_AST_CLASS( SetDirectoryPropsAst )
    typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QList<PropPair>, properties )
CMAKE_END_AST_CLASS( SetDirectoryPropsAst )


CMAKE_BEGIN_AST_CLASS( SetSourceFilesPropsAst )
    typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QStringList, files )
CMAKE_ADD_AST_MEMBER( QList<PropPair>, properties )
CMAKE_END_AST_CLASS( SetSourceFilesPropsAst )


CMAKE_BEGIN_AST_CLASS( SetTargetPropsAst )
    typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QStringList, targets )
CMAKE_ADD_AST_MEMBER( QList<PropPair>, properties )
CMAKE_END_AST_CLASS( SetTargetPropsAst )


CMAKE_BEGIN_AST_CLASS( SetTestsPropsAst )
    typedef QPair<QString, QString> PropPair;
CMAKE_ADD_AST_MEMBER( QStringList, tests )
CMAKE_ADD_AST_MEMBER( QList<PropPair>, properties )
CMAKE_END_AST_CLASS( SetTestsPropsAst )


CMAKE_BEGIN_AST_CLASS( SiteNameAst )
CMAKE_ADD_AST_MEMBER( QString, variableName )
CMAKE_END_AST_CLASS( SiteNameAst )


CMAKE_BEGIN_AST_CLASS( SourceGroupAst )
CMAKE_ADD_AST_MEMBER( QString, name )
CMAKE_ADD_AST_MEMBER( QString, regex )
CMAKE_ADD_AST_MEMBER( QStringList, files )
CMAKE_END_AST_CLASS( SourceGroupAst )


CMAKE_BEGIN_AST_CLASS( StringAst )
enum StringAstType { Regex, Replace, Compare, Ascii, Configure,
    ToUpper, ToLower, Length, Substring, Strip, Random };
enum CommandType { Match, MatchAll, RegexReplace, Equal, NotEqual, Less, Greater };
CMAKE_ADD_AST_MEMBER( StringAstType, type )
CMAKE_ADD_AST_MEMBER( CommandType, cmdType )
CMAKE_ADD_AST_MEMBER( QString, outputVariable )
CMAKE_ADD_AST_MEMBER( QString, regex )
CMAKE_ADD_AST_MEMBER( QString, string )
CMAKE_ADD_AST_MEMBER( QString, replace )
CMAKE_ADD_AST_MEMBER( QStringList, input )
CMAKE_ADD_AST_MEMBER( bool, only )
CMAKE_ADD_AST_MEMBER( bool, escapeQuotes )
CMAKE_ADD_AST_MEMBER( int, begin )
CMAKE_ADD_AST_MEMBER( int, length )
CMAKE_END_AST_CLASS( StringAst )


CMAKE_BEGIN_AST_CLASS( SubdirDependsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, subdir )
CMAKE_ADD_AST_MEMBER( QStringList, directories )
CMAKE_END_AST_CLASS( SubdirDependsAst )


CMAKE_BEGIN_AST_CLASS( SubdirsAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QStringList, directories )
CMAKE_ADD_AST_MEMBER( QStringList, exluceFromAll )
CMAKE_ADD_AST_MEMBER( bool, preorder )
CMAKE_END_AST_CLASS( SubdirsAst )

CMAKE_BEGIN_AST_CLASS( TargetIncludeDirectoriesAst )
enum Visibility { Interface, Public, Private };
struct Item {
    Visibility visibility;
    QString item;
};

CMAKE_ADD_AST_MEMBER( QString, target )
CMAKE_ADD_AST_MEMBER( bool, before )
CMAKE_ADD_AST_MEMBER( QList<Item>, items )
CMAKE_END_AST_CLASS( TargetLinkLibrariesAst )

CMAKE_BEGIN_AST_CLASS( TargetLinkLibrariesAst )
CMAKE_ADD_AST_MEMBER( QString, target )
CMAKE_ADD_AST_MEMBER( QStringList, otherLibs )
CMAKE_ADD_AST_MEMBER( QStringList, debugLibs )
CMAKE_ADD_AST_MEMBER( QStringList, optimizedLibs )
CMAKE_END_AST_CLASS( TargetLinkLibrariesAst )


CMAKE_BEGIN_AST_CLASS( TryCompileAst )
CMAKE_ADD_AST_MEMBER( QString, resultName )
CMAKE_ADD_AST_MEMBER( QString, binDir )
CMAKE_ADD_AST_MEMBER( QString, source )
CMAKE_ADD_AST_MEMBER( QString, projectName )
CMAKE_ADD_AST_MEMBER( QString, targetName )
CMAKE_ADD_AST_MEMBER( QStringList, cmakeFlags )
CMAKE_ADD_AST_MEMBER( QStringList, compileDefinitions )
CMAKE_ADD_AST_MEMBER( QString, outputName )
CMAKE_ADD_AST_MEMBER( QString, copyFile )
CMAKE_END_AST_CLASS( TryCompileAst )


CMAKE_BEGIN_AST_CLASS( TryRunAst )
CMAKE_ADD_AST_MEMBER( QString, runResultVar )
CMAKE_ADD_AST_MEMBER( QString, compileResultVar )
CMAKE_ADD_AST_MEMBER( QString, binDir )
CMAKE_ADD_AST_MEMBER( QString, srcFile )
CMAKE_ADD_AST_MEMBER( QStringList, cmakeFlags )
CMAKE_ADD_AST_MEMBER( QStringList, compileDefs )
CMAKE_ADD_AST_MEMBER( QString, outputVar )
CMAKE_ADD_AST_MEMBER( QStringList, args )
CMAKE_END_AST_CLASS( TryRunAst )


CMAKE_BEGIN_AST_CLASS( UseMangledMesaAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, pathToMesa )
CMAKE_ADD_AST_MEMBER( QString, outputDir )
CMAKE_END_AST_CLASS( UseMangledMesaAst )


CMAKE_BEGIN_AST_CLASS( UtilitySourceAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, cacheEntry )
CMAKE_ADD_AST_MEMBER( QString, executableName )
CMAKE_ADD_AST_MEMBER( QString, pathToSource )
CMAKE_ADD_AST_MEMBER( QStringList, fileList )
CMAKE_END_AST_CLASS( UtilitySourceAst )


CMAKE_BEGIN_AST_CLASS( VariableRequiresAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, testVariable )
CMAKE_ADD_AST_MEMBER( QString, resultVariable )
CMAKE_ADD_AST_MEMBER( QStringList, requiredVariables )
CMAKE_END_AST_CLASS( VariableRequiresAst )


CMAKE_BEGIN_AST_CLASS( VtkMakeInstantiatorAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_END_AST_CLASS( VtkMakeInstantiatorAst )


CMAKE_BEGIN_AST_CLASS( VtkWrapJavaAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_END_AST_CLASS( VtkWrapJavaAst )


CMAKE_BEGIN_AST_CLASS( VtkWrapPythonAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_END_AST_CLASS( VtkWrapPythonAst )


CMAKE_BEGIN_AST_CLASS( VtkWrapTclAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_END_AST_CLASS( VtkWrapTclAst )


CMAKE_BEGIN_AST_CLASS( WhileAst )
CMAKE_ADD_AST_MEMBER( QStringList, condition )
CMAKE_END_AST_CLASS( WhileAst )


CMAKE_BEGIN_AST_CLASS( WriteFileAst )
CMAKE_MARK_AS_DEPRECATED()
CMAKE_ADD_AST_MEMBER( QString, filename )
CMAKE_ADD_AST_MEMBER( QString, message )
CMAKE_ADD_AST_MEMBER( bool, append )
CMAKE_END_AST_CLASS( WriteFileAst )

CMAKE_BEGIN_AST_CLASS( CustomInvokationAst )
CMAKE_ADD_AST_MEMBER( CMakeFunctionDesc, function )
CMAKE_ADD_AST_MEMBER( QList<CMakeFunctionArgument>, arguments )
CMAKE_END_AST_CLASS( CustomInvokationAst )

//CMake 2.6
CMAKE_BEGIN_AST_CLASS( BreakAst )
CMAKE_END_AST_CLASS( BreakAst )

CMAKE_BEGIN_AST_CLASS( CMakePolicyAst )
enum Action { Version, Set, Push, Pop };
CMAKE_ADD_AST_MEMBER( Action, action )
CMAKE_ADD_AST_MEMBER( QList<int>, version )
CMAKE_ADD_AST_MEMBER( int, policyNum )
CMAKE_ADD_AST_MEMBER( bool, isNew )
CMAKE_END_AST_CLASS( CMakePolicyAst )


CMAKE_BEGIN_AST_CLASS( ExportAst )
CMAKE_ADD_AST_MEMBER( QStringList, targets )
CMAKE_ADD_AST_MEMBER( QString, targetNamespace )
CMAKE_ADD_AST_MEMBER( bool, append )
CMAKE_ADD_AST_MEMBER( QString, filename )
CMAKE_END_AST_CLASS( ExportAst )

CMAKE_BEGIN_AST_CLASS( FunctionAst )
CMAKE_ADD_AST_MEMBER( QString, name )
CMAKE_ADD_AST_MEMBER( QStringList, knownArgs )
CMAKE_END_AST_CLASS( FunctionAst )

CMAKE_BEGIN_AST_CLASS( ReturnAst )
CMAKE_END_AST_CLASS( ReturnAst )

#undef CMAKE_END_AST_CLASS

#undef CMAKE_MARK_AS_DEPRECATED

#undef CMAKE_ADD_AST_MEMBER

#undef CMAKE_BEGIN_AST_CLASS

#endif

