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

#ifndef CMAKEVISITOR_H
#define CMAKEVISITOR_H

class CustomCommandAst;
class CustomTargetAst;
class AddDefinitionsAst;
class AddDependenciesAst;
class AddExecutableAst;
class AddLibraryAst;
class AddSubdirectoryAst;
class AddTestAst;
class AuxSourceDirectoryAst;
class BuildCommandAst;
class BuildNameAst;
class CMakeMinimumRequiredAst;
class ConfigureFileAst;
class IncludeAst;
class SetAst;

class CMakeAst;

class CMakeAstVisitor {
    public: //maybe protected?
	virtual ~CMakeAstVisitor();
	virtual void visit( const CustomCommandAst * ) const = 0;
	virtual void visit( const CustomTargetAst * ) const = 0;
	virtual void visit( const AddDefinitionsAst * ) const = 0;
	virtual void visit( const AddDependenciesAst * ) const = 0;
	virtual void visit( const AddExecutableAst * ) const = 0;
	virtual void visit( const AddLibraryAst * ) const = 0;
	virtual void visit( const AddSubdirectoryAst * ) const = 0;
	virtual void visit( const AddTestAst * ) const = 0;
	virtual void visit( const AuxSourceDirectoryAst * ) const = 0;
	virtual void visit( const BuildCommandAst * ) const = 0;
	virtual void visit( const BuildNameAst * ) const = 0;
	virtual void visit( const CMakeMinimumRequiredAst * ) const = 0;
	virtual void visit( const ConfigureFileAst * ) const = 0;
	virtual void visit( const IncludeAst * ) const = 0;
	virtual void visit( const SetAst * ) const = 0;
	virtual void visit( const CMakeAst * ) const = 0;
};

#endif
