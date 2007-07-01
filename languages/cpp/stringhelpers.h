/* 
   Copyright (C) 2007 David Nolden <user@host.de>
   (where user = david.nolden.kdevelop, host = art-master)

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

#ifndef __STRINGHELPERS_H__
#define __STRINGHELPERS_H__

class QString;
class QChar;
class QStringList;

namespace Utils {
/**
 * Fills all comments  within the given c++ code with the given 'replacement' character
 * */
QString clearComments( QString str, QChar replacement = ' ' );
/**
 * Fills all strings within the given c++ code with the given 'replacement' character
 * */
QString clearStrings( QString str, QChar replacement = ' ' );

/**
 * Removes white space at the beginning and end, and replaces contiguous inner white-spaces with single white-spaces. Newlines are treated as whitespaces, the returned text will have no more newlines.
 * */
QString reduceWhiteSpace(QString str);

QString stripFinalWhitespace(QString str);

/**
 * Skips in the string backwards over function-arguments, and stops at the right side of a "("
 * @param skippedArguments Will contain all skipped arguments
 * @param argumentsStart Should be set to the position where the seeking should start, will be changed to the right side of a "(" when found. Should be at the right side of a '(', and may be max. str.length()
 * */
void skipFunctionArguments(QString str, QStringList& skippedArguments, int& argumentsStart );
/**
 * Copied from kdevelop-3.4, should be redone
 * @param index should be the index BEHIND the expression
 * */
int expressionAt( const QString& contents, int index );

/**
searches a fitting closing brace from left to right: a ')' for '(', ']' for '[', ...
*/
int findClose( const QString& str , int pos ); 

/**
 * Searches in the given string for a ',' or closing brace,
 * while skipping everything between opened braces.
 * @param str string to search
 * @param pos position where to start searching
 * @param validEnd when this is set differently, the function will stop when it finds a comma or the given character, and not at closing-braces.
 * @return  On fail, str.length() is returned, else the position of the closing character.
 * */
int findCommaOrEnd( const QString& str , int pos, QChar validEnd = ' ' );
}
#endif 
