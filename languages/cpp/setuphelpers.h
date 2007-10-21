/*
* This file is part of KDevelop
*
* Copyright 2007 Kris Wong <kris.p.wong@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef CPPTOOLSSETUPHELPERS_H
#define CPPTOOLSSETUPHELPERS_H

class QStringList;
namespace Cpp {
    class MacroSet;
}

namespace CppTools {

    /**
    * This function attempts to run a gcc command to retrieve the standard
    * c++ include paths for this machine.
    */
    bool setupStandardIncludePaths(QStringList& includePaths);

    /**
    * This function attempts to run a gcc command to retrieve the standard
    * c++ macro definitions.
    */
    bool setupStandardMacros(Cpp::MacroSet& macros);

}

#endif

