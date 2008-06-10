/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef __DUCHAIN_STRINGHELPERS_H__
#define __DUCHAIN_STRINGHELPERS_H__

#include "language/languageexport.h"

#include <QtCore/QChar>

class QString;
class QStringList;

namespace KDevelop
{

/**
searches a fitting closing brace from left to right: a ')' for '(', ']' for '[', ...
*/
int KDEVPLATFORMLANGUAGE_EXPORT findClose( const QString& str , int pos );

/**
 * Searches in the given string for a ',' or closing brace,
 * while skipping everything between opened braces.
 * @param str string to search
 * @param pos position where to start searching
 * @param validEnd when this is set differently, the function will stop when it finds a comma or the given character, and not at closing-braces.
 * @return  On fail, str.length() is returned, else the position of the closing character.
 * */
int KDEVPLATFORMLANGUAGE_EXPORT findCommaOrEnd( const QString& str , int pos, QChar validEnd = ' ' );

/**
 * Can be used to iterate through different kinds of parameters, for example template-parameters(By giving it "<>:")
 * */
class KDEVPLATFORMLANGUAGE_EXPORT ParamIterator
{
  public:
    /**
     * @param parens should be a string containing the two parens between which the parameters are searched. Example: "<>" or "()" Optionally it can also contain one third end-character. If that end-character is encountered in the prefix, the iteration will be stopped.
     *
     * Example: When "<>:" is given, ParamIterator will only parse the first identifier of a c++ scope
     * */
    ParamIterator( QString parens, QString source, int start = 0);

    ParamIterator& operator ++();

    /**
     * Returns current found parameter
     * */
    QString operator *();

    /**
     * Returns whether there is a current found parameter
     * */
    operator bool() const;

    /**
     * Returns the text in front of the first opening-paren(if none found then the whole text)
     * */
    QString prefix() const;

    uint position() const;

  private:
    class ParamIteratorPrivate* const d;
};

}

#endif
