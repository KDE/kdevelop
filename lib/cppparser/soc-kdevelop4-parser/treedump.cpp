/* This file is part of KDevelop
    Copyright (C) 2002,2003,2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "treedump.h"
#include "ast.h"
#include "lexer.h"

#include <cassert>

std::string TreeDump::nodeType(int tp)
{
    switch (tp) {
#define DECLARE_AST_NODE(n) \
   case NodeType_##n: return (#n);
#include "ast.table"
    default:
        assert (0);
    }

#undef DECLARE_AST_NODE

    return "";
}

TreeDump::TreeDump(TokenStream *tokenStream)
{
    this->tokenStream = tokenStream;
}

std::string TreeDump::textOf(AST *node, TokenStream *tokenStream)
{
    std::string text;
    for (int i=node->startToken(); i<node->endToken(); ++i) {
        text += tokenStream->tokenText(i);
        if (i+1 < node->endToken())
            text += " ";
    }

    return text;
}

void TreeDump::dumpNode(AST *node)
{
    static int ind = 0;
    int i = ind;
    while (i--)
        putchar(' ');

    printf("(%s)", nodeType(node->nodeType()).c_str());

    if (node->children()) {
        printf("\n");
        ind += 2;
        const ListNode<AST*> *it = node->children()->toFront(), *end = it;
        do {
            AST *child = it->element;
            dumpNode(child);
            it = it->next;
        } while (it != end);
        ind -= 2;
    } else { // leaf
        printf("[%s]\n", textOf(node, tokenStream).c_str());
    }
}

