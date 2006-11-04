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

    virtual bool parseFunctionInfo( const CMakeFunctionDesc& function ) { return false; }

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



#endif
