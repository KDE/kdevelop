/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

class CMakeFunctionDesc;

class CMakeAst
{
public:
    CMakeAst() { }
    CMakeAst( const CMakeAst& ast ) : m_children( ast.m_children ) {}
    virtual ~CMakeAst() { qDeleteAll( m_children ); }

    /**
     * Adds a child Ast Node to this node. This will only have uses in cases
     * where custom macros are used since CMakeLists.txt files generally have
     * a pretty flat tree structure
     */
    virtual void addChildAst( CMakeAst* node ) { m_children.append( node ); }

    /**
     * Indicates if the Ast has children
     * @return true if the Ast has children
     * @return false if the Ast has no children
     */
    bool hasChildren() const { return m_children.isEmpty(); }

    /**
     * Get the children of this ast
     * @return the list of this ast's children
     */
    QList<CMakeAst*> children() const  { return m_children; }

    /**
     * Writes the information stored in the Ast into the @p buffer.
     * All Asts that are a child of this Ast are written back as well.
     */
    virtual void writeBack(QString& buffer);

    virtual bool parseFunctionInfo( const CMakeFunctionDesc& ) { return false; }

protected:
    QList<CMakeAst*> m_children;

};

class CustomCommandAst : public CMakeAst
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

    virtual void writeBack( QString& );
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
        bool astId = AstFactory::self()->registerAst( QLatin1String( #astId ), Create##klassName ); }

#define CMAKE_BEGIN_AST_CLASS( klassName ) class klassName : public CMakeAst {  \
    public:                                                  \
        klassName();                                         \
       ~klassName();                                         \
                                                             \
        virtual void writeBack( QString& buffer );           \
        virtual bool parseFunctionInfo( const CMakeFunctionDesc& );

#define CMAKE_ADD_AST_MEMBER( returnType, setterType, returnName, setterName ) \
    public:                                              \
        returnType returnName() const;                         \
        void set##setterName( setterType );                    \
    private:                                             \
        returnType m_##returnName;

#define CMAKE_ADD_AST_FUNCTION( function ) \
    public:                                \
       function;

#define CMAKE_END_AST_CLASS( klassName ) };


CMAKE_BEGIN_AST_CLASS( CustomTargetAst )
CMAKE_ADD_AST_MEMBER( QString, const QString&, target, Target )
CMAKE_ADD_AST_MEMBER( bool, bool, buildAlways, BuildAlways )
CMAKE_ADD_AST_MEMBER( QStringList, const QStringList&, commands, Commands )
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
CMAKE_ADD_AST_MEMBER( float, float, version, Version )
CMAKE_ADD_AST_MEMBER( bool, bool, wrongVersionIsFatal, WrongVersionIsFatal )
CMAKE_END_AST_CLASS( CMakeMinimumRequiredAst )


CMAKE_BEGIN_AST_CLASS( ConfigureFileAst )
CMAKE_END_AST_CLASS( ConfigureFileAst )


CMAKE_BEGIN_AST_CLASS( CreateTestSourcelistAst )
CMAKE_END_AST_CLASS( CreateTestSourcelistAst )


CMAKE_BEGIN_AST_CLASS( EnableLanguageAst )
CMAKE_END_AST_CLASS( EnableLanguageAst )


CMAKE_BEGIN_AST_CLASS( EnableTestingAst )
CMAKE_END_AST_CLASS( EnableTestingAst )


CMAKE_BEGIN_AST_CLASS( ExecProgramAst )
CMAKE_END_AST_CLASS( ExecProgramAst )


CMAKE_BEGIN_AST_CLASS( ExecuteProcessAst )
CMAKE_END_AST_CLASS( ExecuteProcessAst )


CMAKE_BEGIN_AST_CLASS( ExportLibraryDepsAst )
CMAKE_END_AST_CLASS( ExportLibraryDepsAst )


CMAKE_BEGIN_AST_CLASS( FileAst )
CMAKE_END_AST_CLASS( FileAst )


CMAKE_BEGIN_AST_CLASS( FindFileAst )
CMAKE_END_AST_CLASS( FindFileAst )


CMAKE_BEGIN_AST_CLASS( FindLibraryAst )
CMAKE_END_AST_CLASS( FindLibraryAst )


CMAKE_BEGIN_AST_CLASS( FindPackageAst )
CMAKE_END_AST_CLASS( FindPackageAst )


CMAKE_BEGIN_AST_CLASS( FindPathAst )
CMAKE_END_AST_CLASS( FindPathAst )


CMAKE_BEGIN_AST_CLASS( FindProgramAst )
CMAKE_END_AST_CLASS( FindProgramAst )


CMAKE_BEGIN_AST_CLASS( FltkWrapUiAst )
CMAKE_END_AST_CLASS( FltkWrapUiAst )


CMAKE_BEGIN_AST_CLASS( ForeachAst )
CMAKE_END_AST_CLASS( ForeachAst )


CMAKE_BEGIN_AST_CLASS( GetCMakePropertyAst )
CMAKE_END_AST_CLASS( GetCMakePropertyAst )


CMAKE_BEGIN_AST_CLASS( GetDirPropertyAst )
CMAKE_END_AST_CLASS( GetDirPropertyAst )


CMAKE_BEGIN_AST_CLASS( GetFilenameComponentAst )
CMAKE_END_AST_CLASS( GetFilenameComponentAst )


CMAKE_BEGIN_AST_CLASS( GetSourceFilePropAst )
CMAKE_END_AST_CLASS( GetSourceFilePropAst )


CMAKE_BEGIN_AST_CLASS( GetTargetPropAst )
CMAKE_END_AST_CLASS( GetTargetPropAst )


CMAKE_BEGIN_AST_CLASS( GetTestPropAst )
CMAKE_END_AST_CLASS( GetTestPropAst )


CMAKE_BEGIN_AST_CLASS( IfAst )
CMAKE_END_AST_CLASS( IfAst )


CMAKE_BEGIN_AST_CLASS( IncludeAst )
CMAKE_END_AST_CLASS( IncludeAst )


CMAKE_BEGIN_AST_CLASS( IncludeDirectoriesAst )
CMAKE_END_AST_CLASS( IncludeDirectoriesAst )


CMAKE_BEGIN_AST_CLASS( IncludeExternalMsProjectAst )
CMAKE_END_AST_CLASS( IncludeExternalMsProjectAst )


CMAKE_BEGIN_AST_CLASS( IncludeRegularExpressionAst )
CMAKE_END_AST_CLASS( IncludeRegularExpressionAst )


CMAKE_BEGIN_AST_CLASS( InstallAst )
CMAKE_END_AST_CLASS( InstallAst )


CMAKE_BEGIN_AST_CLASS( InstallFilesAst )
CMAKE_END_AST_CLASS( InstallFilesAst )


CMAKE_BEGIN_AST_CLASS( InstallProgramsAst )
CMAKE_END_AST_CLASS( InstallProgramsAst )


CMAKE_BEGIN_AST_CLASS( InstallTargetsAst )
CMAKE_END_AST_CLASS( InstallTargetsAst )


CMAKE_BEGIN_AST_CLASS( LinkDirectoriesAst )
CMAKE_END_AST_CLASS( LinkDirectoriesAst )


CMAKE_BEGIN_AST_CLASS( LinkLibrariesAst )
CMAKE_END_AST_CLASS( LinkLibrariesAst )


CMAKE_BEGIN_AST_CLASS( ListAst )
CMAKE_END_AST_CLASS( ListAst )


CMAKE_BEGIN_AST_CLASS( LoadCacheAst )
CMAKE_END_AST_CLASS( LoadCacheAst )


CMAKE_BEGIN_AST_CLASS( LoadCommandAst )
CMAKE_END_AST_CLASS( LoadCommandAst )


CMAKE_BEGIN_AST_CLASS( MacroAst )
CMAKE_END_AST_CLASS( MacroAst )


CMAKE_BEGIN_AST_CLASS( MakeDirectoryAst )
CMAKE_END_AST_CLASS( MakeDirectoryAst )


CMAKE_BEGIN_AST_CLASS( MarkAsAdvancedAst )
CMAKE_END_AST_CLASS( MarkAsAdvancedAst )


CMAKE_BEGIN_AST_CLASS( MathAst )
CMAKE_END_AST_CLASS( MathAst )


CMAKE_BEGIN_AST_CLASS( MessageAst )
CMAKE_END_AST_CLASS( MessageAst )


CMAKE_BEGIN_AST_CLASS( OptionAst )
CMAKE_END_AST_CLASS( OptionAst )


CMAKE_BEGIN_AST_CLASS( OutputRequiredFilesAst )
CMAKE_END_AST_CLASS( OutputRequiredFilesAst )


CMAKE_BEGIN_AST_CLASS( ProjectAst )
CMAKE_END_AST_CLASS( ProjectAst )


CMAKE_BEGIN_AST_CLASS( QtWrapCppAst )
CMAKE_END_AST_CLASS( QtWrapCppAst )


CMAKE_BEGIN_AST_CLASS( QtWrapUiAst )
CMAKE_END_AST_CLASS( QtWrapUiAst )


CMAKE_BEGIN_AST_CLASS( RemoveAst )
CMAKE_END_AST_CLASS( RemoveAst )


CMAKE_BEGIN_AST_CLASS( RemoveDefinitionsAst )
CMAKE_END_AST_CLASS( RemoveDefinitionsAst )


CMAKE_BEGIN_AST_CLASS( SeparateArgumentsAst )
CMAKE_END_AST_CLASS( SeparateArgumentsAst )


CMAKE_BEGIN_AST_CLASS( SetAst )
CMAKE_END_AST_CLASS( SetAst )


CMAKE_BEGIN_AST_CLASS( SetDirectoryPropsAst )
CMAKE_END_AST_CLASS( SetDirectoryPropsAst )


CMAKE_BEGIN_AST_CLASS( SetSourceFilesPropsAst )
CMAKE_END_AST_CLASS( SetSourceFilesPropsAst )


CMAKE_BEGIN_AST_CLASS( SetTargetPropsAst )
CMAKE_END_AST_CLASS( SetTargetPropsAst )


CMAKE_BEGIN_AST_CLASS( SetTestsPropsAst )
CMAKE_END_AST_CLASS( SetTestsPropsAst )


CMAKE_BEGIN_AST_CLASS( SiteNameAst )
CMAKE_END_AST_CLASS( SiteNameAst )


CMAKE_BEGIN_AST_CLASS( SourceGroupAst )
CMAKE_END_AST_CLASS( SourceGroupAst )


CMAKE_BEGIN_AST_CLASS( StringAst )
CMAKE_END_AST_CLASS( StringAst )


CMAKE_BEGIN_AST_CLASS( SubdirDependsAst )
CMAKE_END_AST_CLASS( SubdirDependsAst )


CMAKE_BEGIN_AST_CLASS( SubdirsAst )
CMAKE_END_AST_CLASS( SubdirsAst )


CMAKE_BEGIN_AST_CLASS( TargetLinkLibrariesAst )
CMAKE_END_AST_CLASS( TargetLinkLibrariesAst )


CMAKE_BEGIN_AST_CLASS( TryCompileAst )
CMAKE_END_AST_CLASS( TryCompileAst )


CMAKE_BEGIN_AST_CLASS( TryRunAst )
CMAKE_END_AST_CLASS( TryRunAst )


CMAKE_BEGIN_AST_CLASS( UseMangledMesaAst )
CMAKE_END_AST_CLASS( UseMangledMesaAst )


CMAKE_BEGIN_AST_CLASS( UtilitySourceAst )
CMAKE_END_AST_CLASS( UtilitySourceAst )


CMAKE_BEGIN_AST_CLASS( VariableRequiresAst )
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
CMAKE_END_AST_CLASS( WhileAst )


CMAKE_BEGIN_AST_CLASS( WriteFileAst )
CMAKE_END_AST_CLASS( WriteFileAst )

#endif












































































