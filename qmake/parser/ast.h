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
                Value = 6
            };

            explicit AST( AST* parent = 0 );
            virtual ~AST() = 0;
            virtual AST::Type type() const = 0;
            AST* parent() const;
            void setColumn( int );
            void setLine( int );
            virtual int column() const;
            virtual int line() const;
            KDevelop::DUContext* context;
        private:
            int m_line;
            int m_column;
            AST* m_parent;
    };


    class KDEVQMAKEPARSER_EXPORT StatementAST : public AST
    {
        public:
            explicit StatementAST( AST* parent = 0 );
            ~StatementAST();
            ValueAST* identifier() const;
            virtual void setIdentifier( ValueAST* );
            int line() const;
            int column() const;
        private:
            ValueAST* m_identifier;
    };


    class KDEVQMAKEPARSER_EXPORT ScopeBodyAST: public AST
    {
        public:
            explicit ScopeBodyAST( AST* parent = 0 );
            ~ScopeBodyAST();
            void insertStatement( int i, StatementAST* );
            void addStatement( StatementAST* );
            QList<StatementAST*> statements() const;
            void removeStatement( int i );
            AST::Type type() const;
            int line() const;
            int column() const;
        private:
            QList<StatementAST*> m_statements;
    };

    /**
     * Represents a QMake Project file
     */
    class KDEVQMAKEPARSER_EXPORT ProjectAST : public ScopeBodyAST
    {
        public:
            explicit ProjectAST( AST* parent = 0 );
            ~ProjectAST();

            /**
            * Returns the filename of the project file, or an empty string if the project was parser from a string
            */
            QString filename() const;
            void setFilename( const QString& );
            AST::Type type() const;
        private:
            QString m_filename;


    };

    class KDEVQMAKEPARSER_EXPORT AssignmentAST : public StatementAST
    {
        public:
            explicit AssignmentAST( AST* parent = 0 );
            ~AssignmentAST();

            void addValue( ValueAST* );
            void insertValue( int i, ValueAST* );
            QList<ValueAST*> values() const;
            void removeValue( int i );
            ValueAST* variable() const;
            void setVariable( ValueAST* );
            ValueAST* op() const;
            void setOp( ValueAST* );
            AST::Type type() const;
        private:
            ValueAST* m_op;
            QList<ValueAST*> m_values;
    };


    class KDEVQMAKEPARSER_EXPORT ScopeAST : public StatementAST
    {
        public:
            explicit ScopeAST( AST* parent = 0 );
            ~ScopeAST();
            void setScopeBody( ScopeBodyAST* );
            ScopeBodyAST* scopeBody() const;
        private:
            ScopeBodyAST* m_body;
    };

    class KDEVQMAKEPARSER_EXPORT FunctionCallAST : public ScopeAST
    {
        public:
            explicit FunctionCallAST( AST* parent = 0 );
            ~FunctionCallAST();
            QList<ValueAST*> arguments() const;
            void addArgument( ValueAST* );
            void insertArgument( int i, ValueAST* );
            void removeArgument( int i );
            ValueAST* functionName() const;
            void setFunctionName( ValueAST* );
            AST::Type type() const;
        private:
            QList<ValueAST*> m_args;
    };


    class KDEVQMAKEPARSER_EXPORT SimpleScopeAST : public ScopeAST
    {
        public:
            explicit SimpleScopeAST( AST* parent = 0);
            ~SimpleScopeAST();
            ValueAST* scopeName() const;
            void setScopeName( ValueAST* );
            AST::Type type() const;
        private:
    };

    class KDEVQMAKEPARSER_EXPORT OrAST : public ScopeAST
    {
        public:
            explicit OrAST( AST* parent = 0 );
            ~OrAST();
            void addScope( ScopeAST* );
            void insertScope( int i, ScopeAST* );
            void removeScope( int i );
            QList<ScopeAST*> scopes() const;
            void setIdentifier( ValueAST* );
            int line() const;
            int column() const;
            AST::Type type() const;
        private:
            QList<ScopeAST*> m_scopes;
    };

    class KDEVQMAKEPARSER_EXPORT ValueAST : public AST
    {
        public:
            explicit ValueAST( AST* parent = 0 );
            void setValue( const QString& );
            QString value() const;
            AST::Type type() const;
        private:
            QString m_value;
    };

}


#endif

