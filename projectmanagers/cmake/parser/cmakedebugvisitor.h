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

#ifndef CMAKEDEBUGVISITOR_H
#define CMAKEDEBUGVISITOR_H

#include "cmakeastvisitor.h"
#include "cmakeast.h"
#include "cmakecommonexport.h"

class KDEVCMAKECOMMON_EXPORT CMakeAstDebugVisitor : public CMakeAstVisitor{
    public: 
        CMakeAstDebugVisitor();
        virtual ~CMakeAstDebugVisitor();

        virtual int visit( const CustomCommandAst * );
        virtual int visit( const CustomTargetAst * );
        virtual int visit( const AddDefinitionsAst * );
        virtual int visit( const AddDependenciesAst * );
        virtual int visit( const AddExecutableAst * );
        virtual int visit( const AddLibraryAst * );
        virtual int visit( const AddSubdirectoryAst * );
        virtual int visit( const AddTestAst * );
        virtual int visit( const AuxSourceDirectoryAst * );
        virtual int visit( const BuildCommandAst * );
        virtual int visit( const BuildNameAst * );
        virtual int visit( const CMakeMinimumRequiredAst * );
        virtual int visit( const ConfigureFileAst * );
        virtual int visit( const IncludeAst * );
        virtual int visit( const SetAst * );
        virtual int visit( const ProjectAst * );
        virtual int visit( const ExecuteProcessAst * );
        virtual int visit( const IncludeDirectoriesAst * );
        virtual int visit( const MacroCallAst * );
        virtual int visit( const MarkAsAdvancedAst * );
        virtual int visit( const FindPackageAst * );
        virtual int visit( const FindProgramAst * );
        virtual int visit( const FindPathAst * );
        virtual int visit( const FindLibraryAst * );
        virtual int visit( const FindFileAst * );
        virtual int visit( const ExecProgramAst * );
        virtual int visit( const IfAst * );
        virtual int visit( const FileAst * );
        virtual int visit( const MessageAst * );
        virtual int visit( const MathAst * );
        virtual int visit( const MacroAst * );
        virtual int visit( const GetFilenameComponentAst * );
        virtual int visit( const OptionAst * );
        virtual int visit( const ListAst * );
        virtual int visit( const ForeachAst * );
        virtual int visit( const StringAst * );
        virtual int visit( const SubdirsAst * );
        virtual int visit( const GetCMakePropertyAst * );
        virtual int visit( const CMakeAst * );

        int walk(const QString&, const CMakeFileContent & fc, int line);
        
    private:
};


#endif

