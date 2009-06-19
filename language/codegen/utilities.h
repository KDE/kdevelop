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

#include <language/languageexport.h>

#include <QtGui/QValidator>

#ifndef CODEGEN_UTILITIES_H
#define CODEGEN_UTILITIES_H

namespace KDevelop
{

class IndexedString;
class DUContext;
class Declaration;
    
namespace CodeGenUtils
{

/*!
 * A validator object that  verifies if a string would be an acceptable identifier
 * If inserted into the given context, including if it conflicts with any other identifier
 */
class KDEVPLATFORMLANGUAGE_EXPORT IdentifierValidator : public QValidator
{
  public:
    IdentifierValidator( DUContext * context);
    ~IdentifierValidator(void);
    
    virtual State validate(QString & input, int &) const;
      
  private:
    
    DUContext * m_context;
};

/**
 * Priority for searching a class implementation file, each of these concepts will be searched for in order,
 * the first found will be returned.
 */
enum SearchPriority
{
    DefaultConstructor,
    CustomConstructor,
    Destructor,
    ClassNameExtension,
    DeclarationOnly,
};

/**
 * @brief Search for the file that contains the implementation of a specified type
 *
 * Search for the file that contains the implementation of @p targetClass. For languages that
 * allow implementation of a type through multiple files, use @p priority to specify which file that contains
 * the specified definition should be used. If not found it will search for the next definition as declared
 * in SearchPriority. For ClassNameExtension, specify an @p extension, then the file that matches the
 * target class' name with @p extension will be used if found.
 *
 * @note If called with a Forward declaration, the real declaration will be searched for.
 *
 * @return The file that matched the top Priority first, or an empty indexed string if none was found
 */
KDEVPLATFORMLANGUAGE_EXPORT IndexedString
fetchImplementationFileForClass(const Declaration & targetClass, SearchPriority priority = DeclarationOnly, const QString & extension = "");
    
//TODO Generalized Renaming

//TODO Interface to verify DuContext Encapsulation

//TODO Expression Morphing

//TODO Detect Conflicts

}


}

#endif  //CODEGEN_UTILITIES_H
