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
            explicit AST( AST* parent = 0 );
            virtual ~AST() = 0;

            AST* parent() const;
        protected:
            void setParent( AST* );
        private:
            AST* m_parent;
    };


    class StatementAST : public AST
    {
        public:
            explicit StatementAST( AST* parent = 0 );
            QString identifier() const;
            void setIdentifier( const QString& );
        private:
            QString m_identifier;
    };

    /**
     * Represents a QMake Project file
     */
    class ProjectAST : public AST
    {
        public:
            explicit ProjectAST( AST* parent = 0 );
            ~ProjectAST();

            /**
            * Returns the filename of the project file, or an empty string if the project was parser from a string
            */
            QString filename() const;
            void insertStatement( int i, StatementAST* );
            void addStatement( StatementAST* );
            QList<StatementAST*> statements() const;
            void removeStatement( int i );

            void setFilename( const QString& );

        private:
            QString m_filename;
            QList<StatementAST*> m_statements;


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
        private:
            QString m_op;
            QList<ValueAST*> m_values;
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
        private:
            QList<StatementAST*> m_statements;
    };

    class ScopeAST : public StatementAST
    {
        public:
            explicit ScopeAST( AST* parent = 0 );
            ~ScopeAST();
            void setScopeBody( ScopeBodyAST* );
            ScopeBodyAST* scopeBody() const;
        private:
            ScopeBodyAST* m_body;
    };

    class FunctionCallAST : public ScopeAST
    {
        public:
            explicit FunctionCallAST( AST* parent = 0 );
            ~FunctionCallAST();
            QList<ValueAST*> arguments() const;
            void insertArgument( int i, ValueAST* );
            void removeArgument( int i );
            QString functionName() const;
            void setFunctionName( const QString& );
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
        private:
    };

    class OrAST : public ScopeAST
    {
        public:
            explicit OrAST( AST* parent = 0 );
            ~OrAST();
            void writeToString( QString& ) const;
            ScopeAST* leftScope() const;
            ScopeAST* rightScope() const;
            void setLeftScope( ScopeAST* );
            void setRightScope( ScopeAST* );
        private:
            ScopeAST* m_lScope;
            ScopeAST* m_rScope;
    };

    class ValueAST : public AST
    {
        public:
            explicit ValueAST( AST* parent = 0 );
            void setValue( const QString& );
            QString value() const;
        private:
            QString m_value;
    };

}


#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
