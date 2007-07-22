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

namespace QMake
{
    class ValueAST;

    class AST
    {
        public:

            enum Type
            {
                Project = 1,
                Assignment = 2,
                FunctionCall = 3,
                SimpleScope = 4,
                Or = 5,
                Value = 6,
                ScopeBody = 7,
            };

            virtual ~AST() = 0;
            virtual AST::Type type() const = 0;
            AST* parent() const;
        protected:
            explicit AST( AST* parent = 0 );
            void setParent( AST* );
        private:
            AST* m_parent;
    };


    class StatementAST : public AST
    {
        public:
            explicit StatementAST( AST* parent = 0 );
            QString identifier() const;
            virtual void setIdentifier( const QString& );
        private:
            QString m_identifier;
    };


    class ScopeBodyAST: public AST
    {
        public:
            explicit ScopeBodyAST( AST* parent = 0 );
            ~ScopeBodyAST();
            void insertStatement( int i, StatementAST* );
            void addStatement( StatementAST* );
            QList<StatementAST*> statements() const;
            void removeStatement( int i );
            AST::Type type() const;
        private:
            QList<StatementAST*> m_statements;
    };

    /**
     * Represents a QMake Project file
     */
    class ProjectAST : public ScopeBodyAST
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

    class AssignmentAST : public StatementAST
    {
        public:
            explicit AssignmentAST( AST* parent = 0 );
            ~AssignmentAST();

            void addValue( ValueAST* );
            void insertValue( int i, ValueAST* );
            QList<ValueAST*> values() const;
            void removeValue( int i );
            QString variable() const;
            void setVariable( const QString& );
            QString op() const;
            void setOp( const QString& );
            AST::Type type() const;
        private:
            QString m_op;
            QList<ValueAST*> m_values;
    };


    class ScopeAST : public StatementAST
    {
        public:
            ~ScopeAST();
            void setScopeBody( ScopeBodyAST* );
            ScopeBodyAST* scopeBody() const;
        protected:
            explicit ScopeAST( AST* parent = 0 );
        private:
            ScopeBodyAST* m_body;
    };

    class FunctionCallAST : public ScopeAST
    {
        public:
            explicit FunctionCallAST( AST* parent = 0 );
            ~FunctionCallAST();
            QList<ValueAST*> arguments() const;
            void addArgument( ValueAST* );
            void insertArgument( int i, ValueAST* );
            void removeArgument( int i );
            QString functionName() const;
            void setFunctionName( const QString& );
            AST::Type type() const;
        private:
            QList<ValueAST*> m_args;
    };


    class SimpleScopeAST : public ScopeAST
    {
        public:
            explicit SimpleScopeAST( AST* parent = 0);
            ~SimpleScopeAST();
            QString scopeName() const;
            void setScopeName( const QString& );
            AST::Type type() const;
        private:
    };

    class OrAST : public ScopeAST
    {
        public:
            explicit OrAST( AST* parent = 0 );
            ~OrAST();
            void addScope( ScopeAST* );
            void insertScope( int i, ScopeAST* );
            void removeScope( int i );
            QList<ScopeAST*> scopes() const;
            void setIdentifier( const QString& );
            AST::Type type() const;
        private:
            QList<ScopeAST*> m_scopes;
    };

    class ValueAST : public AST
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

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
