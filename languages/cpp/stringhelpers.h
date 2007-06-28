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

namespace Utils {

bool parenFits( QChar c1, QChar c2 );

bool isParen( QChar c1 );

bool isTypeParen( QChar c1 );

bool isTypeOpenParen( QChar c1 );

bool isTypeCloseParen( QChar c1 );

bool isLeftParen( QChar c1 );

void fillString( QString& str, int start, int end, QChar replacement );

QString clearComments( QString str, QChar replacement = QChar() );

QString reduceWhiteSpace(QString str);

QString clearStrings( QString str, QChar replacement = QChar() );

int expressionAt( const QString& text, int index );

QString reverse( const QString& str );

int findClose( const QString& str , int pos );

int findCommaOrEnd( const QString& str , int pos, QChar validEnd = QChar());

void skipFunctionArguments(QString str, QStringList& skippedArguments, int& argumentsStart );

}
