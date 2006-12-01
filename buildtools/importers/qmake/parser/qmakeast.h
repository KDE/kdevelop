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
            AST( AST* parent = 0 );
            virtual ~AST() = 0;

            AST* parent() const;

            virtual void writeToString( QString& ) const = 0 ;
            void setDepth( unsigned int );
            unsigned int depth() const;
        protected:
            void setParent( AST* );
        private:
            AST* m_parent;
            unsigned int m_depth;
            friend class ChildAST;
    };


    class StatementAST : public AST
    {
        public:
            StatementAST( AST* parent ) : AST( parent ) {}
//             virtual ~StatementAST( ) {}
//             virtual void writeToString( QString& ) const {}
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



    class OpAST : public AST
    {
        public:
            OpAST( const QString& , const QString&, const QString&, AST* parent = 0 );
            ~OpAST();

            QString rightWhitespace() const;
            QString leftWhitespace() const;
            QString op() const;

            void setRightWhitespace( const QString& );
            void setLeftWhitespace( const QString& );
            void setOp( const QString& );
            void writeToString( QString& ) const;
        private:
            QString m_op;
            QString m_lWs;
            QString m_rWs;
    };

    class AssignmentAST : public StatementAST
    {
        public:
            AssignmentAST( const QString& variable, OpAST* op, const QStringList& values, AST* parent = 0 );
            ~AssignmentAST();

            void addValue( const QString& );
            QStringList values() const;
            void removeValue( const QString& );
            QString variable() const;
            void setVariable( const QString& );
            OpAST* op() const;
            void setOp( OpAST* );
            void writeToString( QString& ) const;
        private:
            QString m_variable;
            OpAST* m_op;
            QStringList m_values;

    };

    class NewlineAST : public StatementAST
    {
        public:
            NewlineAST( AST* parent = 0 ) : StatementAST( parent ) {}
            void writeToString( QString& buf ) const { buf += "\n"; }
    };

    class CommentAST : public StatementAST
    {
        public:
            CommentAST( const QString& comment, AST* parent = 0 );
            QString comment() const;
            void setComment( const QString& );
            void writeToString( QString& ) const;
        private:
            QString m_comment;
    };

    class WhitespaceAST : public StatementAST
    {
        public:
            WhitespaceAST( const QString& comment, AST* parent = 0 );
            QString whitespace() const;
            void setWhitespace( const QString& );
            void writeToString( QString& ) const;
        private:
            QString m_ws;
    };

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

