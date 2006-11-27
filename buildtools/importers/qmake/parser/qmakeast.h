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
            virtual ~AST();

            AST* parent() const;

            virtual void writeToString( QString& ) const = 0 ;
            void setDepth( unsigned int );
            unsigned int depth() const;
        protected:
            void setParent( AST* );
        private:
            AST* m_parent;
            unsigned int m_depth;
        friend class ProjectAST;
    };

    class ProjectAST : public AST
    {
        public:
            ProjectAST( AST* parent = 0 );
            ~ProjectAST();

            QList<AST*> childs() const;

            QString filename() const;

            void writeToString( QString& ) const;

            void addChild( AST* );

        protected:
            void setFilename( const QString& );

        private:
            QList<AST*> m_childs;
            QString m_filename;

            friend class QMake::Parser;
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
}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

