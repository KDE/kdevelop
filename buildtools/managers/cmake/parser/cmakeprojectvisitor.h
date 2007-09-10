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
#include <QMap>
#include <QHash>
#include "cmakeastvisitor.h"
#include "cmakelistsparser.h"
#include "cmaketypes.h"

class CMakeFunctionDesc;

class CMakeProjectVisitor : CMakeAstVisitor
{
    public:
        explicit CMakeProjectVisitor(const QString& root);
        virtual ~CMakeProjectVisitor() {}
        
        virtual int visit( const CustomCommandAst * );
        virtual int visit( const CustomTargetAst * );
        virtual int visit( const AddDefinitionsAst * );
        virtual int visit( const AddDependenciesAst * ) { return notImplemented("AddDependenciesAst"); }
        virtual int visit( const AddTestAst * ) { return notImplemented("AddTestAst"); }
        virtual int visit( const AuxSourceDirectoryAst * ) { return notImplemented("AuxSourceDirectoryAst"); }
        virtual int visit( const BuildCommandAst * ) { return notImplemented("BuildCommandAst"); }
        virtual int visit( const BuildNameAst * ) { return notImplemented("BuildNameAst"); }
        virtual int visit( const CMakeMinimumRequiredAst * ) { return notImplemented("CMakeMinimumRequiredAst"); }
        virtual int visit( const ConfigureFileAst * ) { return notImplemented("ConfigureFileAst"); }
        virtual int visit( const AddExecutableAst * );
        virtual int visit( const AddLibraryAst * );
        virtual int visit( const AddSubdirectoryAst * );
        virtual int visit( const IncludeDirectoriesAst * );
        virtual int visit( const IncludeAst * );
        virtual int visit( const MacroCallAst * call);
        virtual int visit( const FindPackageAst * );
        virtual int visit( const FindProgramAst * );
        virtual int visit( const FindPathAst * );
        virtual int visit( const FindLibraryAst * );
        virtual int visit( const FindFileAst * );
        virtual int visit( const IfAst * );
        virtual int visit( const ExecProgramAst * );
        virtual int visit( const FileAst * );
        virtual int visit( const MessageAst * );
        virtual int visit( const MathAst * );
        virtual int visit( const MacroAst * );
        virtual int visit( const ListAst * );
        virtual int visit( const GetFilenameComponentAst * );
        virtual int visit( const OptionAst * );
        virtual int visit( const SetAst * );
        virtual int visit( const ForeachAst * );
        virtual int visit( const ProjectAst * );
        virtual int visit( const StringAst * );
        virtual int visit( const SubdirsAst * );
        virtual int visit( const GetCMakePropertyAst * );
        virtual int visit( const CMakeAst * );
        
        void setVariableMap( VariableMap* vars );
        void setMacroMap( MacroMap* macros ) { m_macros=macros; }
        void setModulePath(const QStringList& mp) { m_modulePath=mp; }
        
        QString projectName() const { return m_projectName; }
        QStringList subdirectories() const { return m_subdirectories; }
        QStringList targets() const { return m_filesPerTarget.keys(); }
        QStringList files(const QString &target) const { return m_filesPerTarget[target]; }
        QStringList targetDependencies(const QString & target) const;
        QStringList includeDirectories() const { return m_includeDirectories; }
        
        int walk(const CMakeFileContent& fc, int line);
        
        enum VariableType { NoVar, CMake, ENV };
        static VariableType hasVariable(const QString &name);
        static QString variableName(const QString &name, VariableType& isEnv);
        static QStringList resolveVariables(const QStringList & vars, const VariableMap *values);
        static QStringList resolveVariable(const QString &exp, const VariableMap *values);
        static CMakeFunctionDesc resolveVariables(const CMakeFunctionDesc &exp, const VariableMap *values);
        static QStringList envVarDirectories(const QString &varName);
        
        enum FileType { Location, File, Executable, Library };
        static QString findFile(const QString &file, const QStringList &folders, FileType t=File);
        
    private:
        int notImplemented(const QString& n) const;
        bool haveToFind(const QString &varName);
        
        QStringList m_modulePath;
        QString m_projectName;
        QStringList m_subdirectories;
        QStringList m_includeDirectories;
        QMap<QString, QStringList> m_filesPerTarget;
        QMap<QString, QStringList> m_generatedFiles;
        QString m_root;
        VariableMap *m_vars;
        MacroMap *m_macros;
        DefineList m_defs;
};

#endif
