/* KDevelop QMake Support
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef QMAKELEXER_H
#define QMAKELEXER_H

#include <cstddef>
#include <QtCore/QStack>

class QString;
class kdev_pg_location_table;

namespace QMake
{

class parser;

class Lexer {
public:
    Lexer(parser* _parser, const QString& contents);

    int getNextTokenKind();
    std::size_t getTokenBegin() const;
    std::size_t getTokenEnd() const;

private:
    QString mContent;
    parser* mParser;
    int curpos;
    int mContentSize;
    std::size_t mTokenBegin;
    std::size_t mTokenEnd;

    int state() const;
    void pushState(int state);
    void popState();

    static bool isIdOrValueCharacter(QChar* c);
    static bool isIdentifierCharacter(QChar* c);
    bool isWhitespaceOrComment(QChar* c);

    QStack<int> mState;
    enum State
    {
        DefaultState = 0,
        QuoteState = 1,
        ContState = 2,
        CommentState = 3
    };

};

}

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
