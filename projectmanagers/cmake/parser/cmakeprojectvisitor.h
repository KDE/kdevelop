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
#include <QStack>

#include "cmakeastvisitor.h"
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
        
        virtual int visit( const CustomCommandAst * ) override;
        virtual int visit( const CustomTargetAst * ) override;
        virtual int visit( const AddDefinitionsAst * ) override;
        virtual int visit( const AddTestAst * ) override;
        virtual int visit( const AddExecutableAst * ) override;
        virtual int visit( const AddLibraryAst * ) override;
        virtual int visit( const AddSubdirectoryAst * ) override;
        virtual int visit( const IncludeDirectoriesAst * ) override;
        virtual int visit( const IncludeAst * ) override;
        virtual int visit( const MacroCallAst * call) override;
        virtual int visit( const FindPackageAst * ) override;
        virtual int visit( const MarkAsAdvancedAst * ) override;
        virtual int visit( const FindProgramAst * ) override;
        virtual int visit( const FunctionAst * ) override;
        virtual int visit( const FindPathAst * ) override;
        virtual int visit( const FindLibraryAst * ) override;
        virtual int visit( const FindFileAst * ) override;
        virtual int visit( const IfAst * ) override;
        virtual int visit( const ExecProgramAst * ) override;
        virtual int visit( const ExecuteProcessAst * ) override;
        virtual int visit( const FileAst * ) override;
        virtual int visit( const MessageAst * ) override;
        virtual int visit( const MathAst * ) override;
        virtual int visit( const MacroAst * ) override;
        virtual int visit( const ListAst * ) override;
        virtual int visit( const GetDirPropertyAst * ) override;
        virtual int visit( const GetFilenameComponentAst * ) override;
        virtual int visit( const GetSourceFilePropAst * ) override;
        virtual int visit( const GetTargetPropAst * ) override;
        virtual int visit( const OptionAst * ) override;
        virtual int visit( const SetAst * ) override;
        virtual int visit( const ForeachAst * ) override;
        virtual int visit( const ProjectAst * ) override;
        virtual int visit( const SetPropertyAst* ) override;
        virtual int visit( const SetTargetPropsAst * ) override;
        virtual int visit( const SetDirectoryPropsAst * ) override;
        virtual int visit( const StringAst * ) override;
        virtual int visit( const SubdirsAst * ) override;
        virtual int visit( const SetTestsPropsAst* ) override;
        virtual int visit( const TargetLinkLibrariesAst * ) override;
        virtual int visit( const TargetIncludeDirectoriesAst* ) override;
        virtual int visit( const TryCompileAst * ) override;
        virtual int visit( const GetCMakePropertyAst * ) override;
        virtual int visit( const GetPropertyAst* ) override;
        virtual int visit( const RemoveDefinitionsAst * ) override;
        virtual int visit( const SeparateArgumentsAst * ) override;
        virtual int visit( const UnsetAst * ) override;
        virtual int visit( const WhileAst * ) override;
        virtual int visit( const CMakeAst * ) override;
        
        
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
        void printBacktrace(const QStack<VisitorState> &backtrace);
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
        
        QStack<VisitorState> m_backtrace;
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
