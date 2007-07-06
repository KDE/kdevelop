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

#include "cmakeastvisitor.h"
#include <QString>
#include <QStringList>
#include <QMap>
#include <QHash>

class CMakeProjectVisitor : public CMakeAstVisitor
{
    public:
        CMakeProjectVisitor(QHash<QString, QStringList> *vars, QHash<QString, MacroAst*> *macros);
        virtual ~CMakeProjectVisitor() {}
        
        virtual void visit( const CustomCommandAst * ) { notImplemented(); }
        virtual void visit( const CustomTargetAst * ) { notImplemented(); }
        virtual void visit( const AddDefinitionsAst * ) { notImplemented(); }
        virtual void visit( const AddDependenciesAst * ) { notImplemented(); }
        virtual void visit( const AddTestAst * ) { notImplemented(); }
        virtual void visit( const AuxSourceDirectoryAst * ) { notImplemented(); }
        virtual void visit( const BuildCommandAst * ) { notImplemented(); }
        virtual void visit( const BuildNameAst * ) { notImplemented(); }
        virtual void visit( const CMakeMinimumRequiredAst * ) { notImplemented(); }
        virtual void visit( const ConfigureFileAst * ) { notImplemented(); }
        virtual void visit( const AddExecutableAst * );
        virtual void visit( const AddLibraryAst * );
        virtual void visit( const AddSubdirectoryAst * );
        virtual void visit( const IncludeDirectoriesAst * );
        virtual void visit( const IncludeAst * );
        virtual void visit( const MacroCallAst * call);
        virtual void visit( const FindPackageAst * );
        virtual void visit( const FindProgramAst * );
        virtual void visit( const FindPathAst * );
        virtual void visit( const IfAst * );
        virtual void visit( const ExecProgramAst * );
        virtual void visit( MacroAst * );
        
        virtual void visit( const SetAst * );
        virtual void visit( const ProjectAst * );
        virtual void visit( const CMakeAst * );
        
        QString projectName() const { return m_projectName; }
        QStringList subdirectories() const { return m_subdirectories; }
        QStringList targets() const { return m_filesPerTarget.keys(); }
        QStringList files(const QString &target) const { return m_filesPerTarget[target]; }
        QStringList includeDirectories() const { return m_includeDirectories; }
        
        static bool hasVariable(const QString &name);
        static QString variableName(const QString &name);
	static QStringList resolveVariables(const QStringList & vars, const QHash<QString, QStringList> *values);
    private:
        static QStringList resolveVariable(const QString &exp, const QHash<QString, QStringList> *values);
        static QString findFile(const QString &file, const QStringList &folders);
        void notImplemented() const;
        bool haveToFind(const QString &varName);
        
        QString m_projectName;
        QStringList m_subdirectories;
        QStringList m_includeDirectories;
        QMap<QString, QStringList> m_filesPerTarget;
        QString m_root;
        QHash<QString, QStringList> *m_vars;
        QHash<QString, MacroAst*> *m_macros;
};

#endif
