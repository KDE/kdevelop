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

#ifndef QMAKEPARSER_H
#define QMAKEPARSER_H

#include <QtCore/QString>

#include "qmakeexport.h"

namespace QMake
{
    class ProjectAST;
    /**
     * Class to parse a QMake project file or a string containing a QMake project structure
     */
    class QMAKEPARSER_EXPORT Driver
    {
        public:
            /**
             * Parses the given filename and returns success or failure
             * @param filename QMake project filename to be parsed
             * @param ast pointer to an AST structure that will contain the AST after parsing
             * @param debug if set to 1 the parser prints debug information
             * @return 0 if parsing succeeds
             */
            static int parseFile( char const* filename, QMake::ProjectAST* ast, int debug = 0 );

            /**
             * Parses the given filename and returns success or failure
             * @param filename QMake project filename to be parsed
             * @param ast pointer to an AST structure that will contain the AST after parsing
             * @param debug if set to 1 the parser prints debug information
             * @return 0 if parsing succeeds
             */
            static int parseFile( const QString& filename, QMake::ProjectAST* ast, int debug = 0 );

            /**
             * Parses the given string and returns success or failure
             * @param content QMake project file content to be parsed
             * @param ast pointer to an AST structure that will contain the AST after parsing
             * @param debug if set to 1 the parser prints debug information
             * @return 0 if parsing succeeds
             */
            static int parseString( char const* content, QMake::ProjectAST* ast, int debug = 0 );

            /**
             * Parses the given string and returns success or failure
             * @param content QMake project file content to be parsed
             * @param ast pointer to an AST structure that will contain the AST after parsing
             * @param debug if set to 1 the parser prints debug information
             * @return 0 if parsing succeeds
             */
            static int parseString( const QString& content, QMake::ProjectAST* ast, int debug = 0 );
    };
}

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
