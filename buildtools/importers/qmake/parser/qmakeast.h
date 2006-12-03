/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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
#include <QtCore/QStringList>

namespace QMake
{
    class Parser;

    class AST
    {
        public:
            AST( const QString ws = "", AST* parent = 0 );
            virtual ~AST() = 0;

            AST* parent() const;
            QString whitespace() const;
            void setWhitespace( const QString& );
            virtual void writeToString( QString& ) const = 0 ;
        protected:
            void setParent( AST* );
        private:
            AST* m_parent;
            QString m_ws;
            friend class ChildAST;
    };


    class StatementAST : public AST
    {
        public:
            StatementAST( const QString& ws = "", AST* parent = 0 ) : AST( ws, parent ) {}
    };

    class ProjectAST : public AST
    {
        public:
            ProjectAST( AST* parent = 0 );
            ~ProjectAST();

            QString filename() const;
            void addStatement( StatementAST* );
            QList<StatementAST*> statements() const;
            void removeStatement( StatementAST* );

            void writeToString( QString& ) const;
        protected:
            void setFilename( const QString& );

        private:
            QString m_filename;
            QList<StatementAST*> m_statements;

            friend class QMake::Parser;
    };

    class AssignmentAST : public StatementAST
    {
        public:
            AssignmentAST( const QString& variable, const QString& op, const QStringList& values, const QString& = "", AST* parent = 0 );
            ~AssignmentAST();

            void addValue( const QString& );
            QStringList values() const;
            void removeValue( const QString& );
            QString variable() const;
            void setVariable( const QString& );
            QString op() const;
            void setOp( const QString& );
            void writeToString( QString& ) const;
        private:
            QString m_variable;
            QString m_op;
            QStringList m_values;

    };

    class NewlineAST : public StatementAST
    {
        public:
            NewlineAST( const QString& ws = "", AST* parent = 0 ) : StatementAST( ws, parent ) {}
            void writeToString( QString& buf ) const { buf += whitespace()+"\n"; }
    };

    class CommentAST : public StatementAST
    {
        public:
            CommentAST( const QString& comment, const QString& ws = "", AST* parent = 0 );
            QString comment() const;
            void setComment( const QString& );
            void writeToString( QString& ) const;
        private:
            QString m_comment;
    };

    class FunctionArgAST : public AST
    {
        public:
            FunctionArgAST( const QString& ws = "", AST* parent = 0 );
            virtual ~FunctionArgAST();
            virtual void writeToString( QString& ) const;
        private:
    };

    class FunctionCallAST : public FunctionArgAST
    {
        public:
            FunctionCallAST( const QString& name, const QString& begin, QList<FunctionArgAST*> args,
                             const QString& end = "", const QString& ws = "", AST* parent = 0 );
            ~FunctionCallAST();
            void setAsFunctionArg( bool );
            bool asFunctionArg() const;
            void writeToString( QString& ) const;
        private:
            QList<FunctionArgAST*> m_args;
            bool m_asFunctionArg;
            QString m_functionName;
            QString m_begin;
            QString m_end;
    };

    class SimpleFunctionArgAST : public FunctionArgAST
    {
        public:
            SimpleFunctionArgAST( const QString& arg, const QString& ws = "", AST* parent = 0 );
            ~SimpleFunctionArgAST();
            void writeToString( QString& ) const;
        private:
            QString m_value;
    };


    class FunctionAST : public StatementAST
    {
        public:
            FunctionAST( FunctionCallAST* call, const QString& begin, QList<StatementAST*> stmts,
                         const QString& end = "", const QString& ws = "", AST* parent = 0 );
        FunctionAST( FunctionCallAST* call, const QString& begin, StatementAST* stmt,
                         const QString& ws = "", AST* parent = 0 );
            FunctionAST( FunctionCallAST* call, const QString& ws = "", AST* parent = 0 );
            ~FunctionAST();
            void writeToString( QString& ) const;
        private:
            FunctionCallAST* m_call;
            QList<StatementAST*> m_statements;
            QString m_begin;
            QString m_end;
    };

    class ScopeAST : public StatementAST
    {
        public:
            ScopeAST( const QString& name, const QString& begin, QList<StatementAST*> stmts,
                      const QString& end = "", const QString& ws = "", AST* parent = 0 );
            ScopeAST( const QString& name, const QString& begin, StatementAST* stmt,
                      const QString& ws = "", AST* parent = 0 );
            ~ScopeAST();
            void writeToString( QString& ) const;
        private:
            QString m_scopeName;
            QList<StatementAST*> m_statements;
            QString m_begin;
            QString m_end;
    };

    class OrAST : public StatementAST
    {
        public:
            OrAST( FunctionCallAST* lcall, const QString& orop, FunctionCallAST* rcall,
                   const QString& begin, QList<StatementAST*> stmts, const QString& end = "",
                   const QString& ws = "", AST* parent = 0 );
            OrAST( FunctionCallAST* lcall, const QString& orop, FunctionCallAST* rcall,
                   const QString& begin, StatementAST* stmt,
                   const QString& ws = "", AST* parent = 0 );
            ~OrAST();
            void writeToString( QString& ) const;
        private:
            FunctionCallAST* m_lCall;
            FunctionCallAST* m_rCall;
            QList<StatementAST*> m_statements;
            QString m_begin;
            QString m_end;
            QString m_orop;
    };

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

