/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CPPUTILS_H
#define CPPUTILS_H

class QString;

namespace KDevelop
{
class Declaration;

}

namespace CppUtils
{
  
/**
 * @param objectToAllocate The declaration of the object to allocate memory for
 * @return the stringirized form of the user's preferred method of mamory allocation
 *
 * @todo Make this configurable by the user to use eg: new, malloc, or a custom memory allocation
 */
QString insertMemoryAllocation(const KDevelop::Declaration & objectToAllocate);


QString insertMemoryDeallocation(const KDevelop::Declaration & objectToDeallocate);
  
}


#endif	//CPPUTILS_H
