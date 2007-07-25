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

#ifndef __STRINGHELPERS_H__
#define __STRINGHELPERS_H__

#include <duchain/stringhelpers.h>

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

}
#endif 
