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

class CMakeProjectVisitor : public CMakeAstVisitor {
    public:
	virtual ~CMakeProjectVisitor();
	
	virtual void visit( const CMakeAst * ) const { notImplemented(); }
	virtual void visit( const CustomCommandAst * ) const { notImplemented(); }
	virtual void visit( const CustomTargetAst * ) const { notImplemented(); }
	virtual void visit( const AddDefinitionsAst * ) const { notImplemented(); }
	virtual void visit( const AddDependenciesAst * ) const { notImplemented(); }
	virtual void visit( const AddExecutableAst * ) const { notImplemented(); }
	virtual void visit( const AddLibraryAst * ) const { notImplemented(); }
	virtual void visit( const AddSubdirectoryAst * ) const { notImplemented(); }
	virtual void visit( const AddTestAst * ) const { notImplemented(); }
	virtual void visit( const AuxSourceDirectoryAst * ) const { notImplemented(); }
	virtual void visit( const BuildCommandAst * ) const { notImplemented(); }
	virtual void visit( const BuildNameAst * ) const { notImplemented(); }
	virtual void visit( const CMakeMinimumRequiredAst * ) const { notImplemented(); }
	virtual void visit( const ConfigureFileAst * ) const { notImplemented(); }
	virtual void visit( const IncludeAst * ) const { notImplemented(); }
	virtual void visit( const SetAst * ) const { notImplemented(); }
	
	//catch all
	virtual void visit( CMakeAst * ) { notImplemented(); }
    private:
	void notImplemented() const;
};

#endif
