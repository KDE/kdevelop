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

#ifndef CMAKEPROJECTVISITOR_H
#define CMAKEPROJECTVISITOR_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <util/stack.h>

#include "cmaketypes.h"
#include <language/duchain/topducontext.h>

class CMakeFunctionDesc;

namespace KDevelop
{
    class TopDUContext;
    class Declaration;
class ITestController;
}

class KDEVCMAKECOMMON_EXPORT CMakeProjectVisitor : CMakeAstVisitor
{
    public:
        typedef void (*message_callback)(const QString& message);
        
        explicit CMakeProjectVisitor(const QString& root, KDevelop::ReferencedTopDUContext parent);
        virtual ~CMakeProjectVisitor() {}
        
        int visit( const CustomCommandAst * ) override;
        int visit( const CustomTargetAst * ) override;
        int visit( const AddDefinitionsAst * ) override;
        int visit( const AddTestAst * ) override;
        int visit( const AddExecutableAst * ) override;
        int visit( const AddLibraryAst * ) override;
        int visit( const AddSubdirectoryAst * ) override;
        int visit( const IncludeDirectoriesAst * ) override;
        int visit( const IncludeAst * ) override;
        int visit( const MacroCallAst * call) override;
        int visit( const FindPackageAst * ) override;
        int visit( const MarkAsAdvancedAst * ) override;
        int visit( const FindProgramAst * ) override;
        int visit( const FunctionAst * ) override;
        int visit( const FindPathAst * ) override;
        int visit( const FindLibraryAst * ) override;
        int visit( const FindFileAst * ) override;
        int visit( const IfAst * ) override;
        int visit( const ExecProgramAst * ) override;
        int visit( const ExecuteProcessAst * ) override;
        int visit( const FileAst * ) override;
        int visit( const MessageAst * ) override;
        int visit( const MathAst * ) override;
        int visit( const MacroAst * ) override;
        int visit( const ListAst * ) override;
        int visit( const GetDirPropertyAst * ) override;
        int visit( const GetFilenameComponentAst * ) override;
        int visit( const GetSourceFilePropAst * ) override;
        int visit( const GetTargetPropAst * ) override;
        int visit( const OptionAst * ) override;
        int visit( const SetAst * ) override;
        int visit( const ForeachAst * ) override;
        int visit( const ProjectAst * ) override;
        int visit( const SetPropertyAst* ) override;
        int visit( const SetTargetPropsAst * ) override;
        int visit( const SetDirectoryPropsAst * ) override;
        int visit( const StringAst * ) override;
        int visit( const SubdirsAst * ) override;
        int visit( const SetTestsPropsAst* ) override;
        int visit( const TargetLinkLibrariesAst * ) override;
        int visit( const TargetIncludeDirectoriesAst* ) override;
        int visit( const TryCompileAst * ) override;
        int visit( const GetCMakePropertyAst * ) override;
        int visit( const GetPropertyAst* ) override;
        int visit( const RemoveDefinitionsAst * ) override;
        int visit( const SeparateArgumentsAst * ) override;
        int visit( const UnsetAst * ) override;
        int visit( const WhileAst * ) override;
        int visit( const CMakeAst * ) override;
        
        
        void setCacheValues( CacheValues* cache);
        void setVariableMap( VariableMap* vars );
        void setMacroMap( MacroMap* macros ) { m_macros=macros; }
        void setModulePath(const QStringList& mp) { m_modulePath=mp; }
        void setDefinitions(const CMakeDefinitions& defs) { m_defs=defs; }
        
        /** sets the @p profile env variables that will be used to override those in the current system */
        void setEnvironmentProfile(const QMap<QString, QString>& profile) { m_environmentProfile = profile; }

        const VariableMap* variables() const { return m_vars; }
        const CacheValues* cache() const { return m_cache; }
        CMakeDefinitions definitions() const { return m_defs; }
        
        QString projectName() const { return m_projectName; }
        QVector<Subdirectory> subdirectories() const { return m_subdirectories; }
        QVector<Target> targets() const { return m_targetForId.values().toVector(); }
        QStringList resolveDependencies(const QStringList& target) const;
        QVector<Test> testSuites() const { return m_testSuites; }
            
        int walk(const CMakeFileContent& fc, int line, bool isClean=false);
        
//         enum FileType { Location, File, Executable, Library };
        static QString findFile(const QString& files, const QStringList &folders,
                                    const QStringList& suffixes=QStringList(), bool location=false);
        
        QString findExecutable(const QString& filenames, const QStringList& dirs,
                                    const QStringList& pathSuffixes=QStringList()) const;
        
        KDevelop::ReferencedTopDUContext context() const { return m_topctx; }
        QStringList resolveVariable(const CMakeFunctionArgument &exp);

        bool hasMacro(const QString& name) const;

        struct VisitorState
        {
            const CMakeFileContent* code;
            int line;
            KDevelop::ReferencedTopDUContext context;
        };
        CMakeProperties properties() { return m_props; }
        
        static void setMessageCallback(message_callback f) { s_msgcallback=f; }
        
        QStringList variableValue(const QString& var) const;
        void setProperties(const CMakeProperties& properties) { m_props = properties; }
        QHash<QString, QString> targetAlias() { return m_targetAlias; }
        
    protected:
        struct IntPair
        {
            IntPair(int f, int s, int l) : first(f), second(s), level(l) {}
            int first, second, level;
            QString print() const { return QString("(%1, %2 : %3)").arg(first).arg(second).arg(level); }
            static QStringList printList(const QList<IntPair>& l)
            {
                QStringList r;
                foreach(const IntPair& i, l)
                    r+= i.print();
                return r;
            }
        };
        
        static QList<IntPair> parseArgument(const QString &exp);
        
    private:
        QStringList envVarDirectories(const QString &varName) const;
        static message_callback s_msgcallback;
        
        static KDevelop::ReferencedTopDUContext
            createContext(const KDevelop::IndexedString& path, KDevelop::ReferencedTopDUContext aux, int endl ,int endc, bool isClean);
        
        void macroDeclaration(const CMakeFunctionDesc& def, const CMakeFunctionDesc& end, const QStringList& args);
        CMakeFunctionDesc resolveVariables(const CMakeFunctionDesc &exp);
        QStringList value(const QString& exp, const QList<IntPair>& poss, int& desired) const;
        QStringList theValue(const QString& exp, const IntPair& p) const;
        
        void defineTarget(const QString& id, const QStringList& sources, Target::Type t);
        bool haveToFind(const QString &varName);
        void createDefinitions(const CMakeAst* ast);
        void createUses(const CMakeFunctionDesc& ast);
        void printBacktrace(const KDevelop::Stack<VisitorState> &backtrace);
        VisitorState stackTop() const;
        QStringList dependees(const QString& s) const;
        int declareFunction(Macro m, const CMakeFileContent& content, int initial, const QString& end);

        QStringList traverseGlob(const QString& startPath, const QString& expression,
            bool recursive = false, bool followSymlinks = false);
        
        CMakeProperties m_props;
        QStringList m_modulePath;
        QString m_projectName;
        QVector<Subdirectory> m_subdirectories;
        QHash<QString, QStringList> m_generatedFiles;
        QHash<QString, Target> m_targetForId;
        
        KDevelop::Stack<VisitorState> m_backtrace;
        QString m_root;
        VariableMap *m_vars;
        MacroMap *m_macros;
        const CacheValues* m_cache;
        CMakeDefinitions m_defs;
        KDevelop::ReferencedTopDUContext m_topctx;
        KDevelop::ReferencedTopDUContext m_parentCtx;
        bool m_hitBreak;
        bool m_hitReturn;
        QMap<QString, QString> m_environmentProfile;
        QHash<QString, QString> m_targetAlias;

        QVector<Test> m_testSuites;
};

#endif
