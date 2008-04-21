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

#ifndef QMAKEAST_H
#define QMAKEAST_H

#include <QtCore/QString>
#include <QtCore/QList>

#include "parserexport.h"

namespace KDevelop
{
class DUContext;
}

namespace QMake
{
    class ValueAST;

    class KDEVQMAKEPARSER_EXPORT AST
    {
        public:

            enum Type
            {
                Project = 0,
                ScopeBody = 1,
                Assignment = 2,
                FunctionCall = 3,
                SimpleScope = 4,
                Or = 5,
                Value = 6,
                Invalid = 7
            };

            AST( AST* parent, AST::Type type );
            virtual ~AST();
            AST::Type type;
            int startLine;
            int endLine;
            int startColumn;
            int endColumn;
            int start;
            int end;
            AST* parent;
            KDevelop::DUContext* context;
    };


    class KDEVQMAKEPARSER_EXPORT StatementAST : public AST
    {
        public:
            StatementAST( AST* parent, AST::Type type );
            ~StatementAST();
            ValueAST* identifier;
    };


    class KDEVQMAKEPARSER_EXPORT ScopeBodyAST: public AST
    {
        public:
            ScopeBodyAST( AST* parent, AST::Type type = AST::ScopeBody );
            ~ScopeBodyAST();
            QList<StatementAST*> statements;
    };

    class KDEVQMAKEPARSER_EXPORT ProjectAST : public ScopeBodyAST
    {
        public:
            explicit ProjectAST();
            ~ProjectAST();
            QString filename;


    };

    class KDEVQMAKEPARSER_EXPORT AssignmentAST : public StatementAST
    {
        public:
            explicit AssignmentAST( AST* parent );
            ~AssignmentAST();

            ValueAST* op;
            QList<ValueAST*> values;
    };


    class KDEVQMAKEPARSER_EXPORT ScopeAST : public StatementAST
    {
        public:
            explicit ScopeAST( AST* parent, AST::Type type);
            ~ScopeAST();
            ScopeBodyAST* body;
    };

    class KDEVQMAKEPARSER_EXPORT FunctionCallAST : public ScopeAST
    {
        public:
            explicit FunctionCallAST( AST* parent );
            ~FunctionCallAST();
            QList<ValueAST*> args;
    };


    class KDEVQMAKEPARSER_EXPORT SimpleScopeAST : public ScopeAST
    {
        public:
            explicit SimpleScopeAST( AST* parent );
            ~SimpleScopeAST();
    };

    class KDEVQMAKEPARSER_EXPORT OrAST : public ScopeAST
    {
        public:
            explicit OrAST( AST* parent );
            ~OrAST();
            QList<ScopeAST*> scopes;
    };

    class KDEVQMAKEPARSER_EXPORT ValueAST : public AST
    {
        public:
            explicit ValueAST( AST* parent );
            QString value;
    };

}


#endif

