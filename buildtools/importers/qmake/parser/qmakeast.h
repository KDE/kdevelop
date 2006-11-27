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

    class ChildAST : public AST
    {
        public:
            ChildAST( AST* parent = 0 );
            virtual ~ChildAST();
            QList<AST*> childs() const;
            void addChild( AST* );
            virtual void writeToString( QString& ) const;
        private:
            QList<AST*> m_childs;

    };

    class ProjectAST : public ChildAST
    {
        public:
            ProjectAST( AST* parent = 0 );
            ~ProjectAST();

            QString filename() const;

        protected:
            void setFilename( const QString& );

        private:
            QString m_filename;

            friend class QMake::Parser;
    };

    class AssignmentAST : public ChildAST
    {
        public:
            AssignmentAST( const QString& variable, AST* parent = 0 );
            ~AssignmentAST();

            void addValues( QList<AST*> );
            QString variable() const;
            void setVariable( const QString& );
        void writeToString( QString& ) const;
        private:
            QString m_variable;

    };

    class CommentAST : public AST
    {
        public:
            CommentAST( const QString& comment, AST* parent = 0 );
            ~CommentAST();

            QString comment() const;
            void setComment( const QString& comment ) ;

            void writeToString( QString& ) const;

        private:
            QString m_comment;
    };

    class NewlineAST : public AST
    {
        public:
            NewlineAST( AST* parent = 0 );
            ~NewlineAST();

            void writeToString( QString& ) const;
    };

    class LiteralValueAST : public AST
    {
        public:
            LiteralValueAST( const QString& value, AST* parent = 0 );
            virtual ~LiteralValueAST();

            QString value() const;
            void setValue( const QString& );
            virtual void writeToString( QString& ) const;
        private:
            QString m_value;
    };

    class EnvironmentVariableAST : public LiteralValueAST
    {
        public:
            EnvironmentVariableAST( const QString& value, AST* parent = 0 );
            ~EnvironmentVariableAST();

            void writeToString( QString& ) const;
    };

    class QMakeVariableAST : public LiteralValueAST
    {
        public:
            QMakeVariableAST( const QString& value, bool useBrackets, AST* parent = 0 );
            ~QMakeVariableAST();

            bool usesBrackets() const;

            void writeToString( QString& ) const;

        private:
            bool m_useBrackets;

    };
    class WhitespaceAST : public LiteralValueAST
    {
        public:
            WhitespaceAST( const QString& value, AST* parent = 0 );
            ~WhitespaceAST();
    };

class OpAST : public LiteralValueAST
{
public:
    OpAST( const QString& value, AST* parent = 0 );
    ~OpAST();
};
}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

