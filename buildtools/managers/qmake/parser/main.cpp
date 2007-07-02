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

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include "qmakedriver.h"
#include "qmakeast.h"

#include <QtGlobal>
#include <QtCore/QString>

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kurl.h>
#include <klocale.h>

int main( int argc, char* argv[] )
{
    KCmdLineArgs::init( argc, argv, "QMake Parser", 0, ki18n("qmake-parser"), "4.0.0", ki18n("Parse QMake project files"));

    KCmdLineOptions options;
    options.add("silent", ki18n("Enable Parser debug output"));
    options.add("!debug", ki18n("Disable output of the generated AST"));
    options.add("!+files", ki18n("QMake project files"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if( args->count() < 1 )
    {
        KCmdLineArgs::usage(0);
    }

    int debug = 0;
    bool silent = false;

    if( args->isSet("silent") )
        silent = true;
    if( args->isSet("debug") )
        debug = 1;
    for( int i = 0 ; i < args->count() ; i++ )
    {
        QMake::ProjectAST* ast = new QMake::ProjectAST();
        if ( QMake::Driver::parseFile( args->url(i).toLocalFile(), ast, debug ) != 0 ) {
            exit( EXIT_FAILURE );
        }else
        {
            QString buf;
            if( !silent )
                ast->writeToString( buf );
            kDebug(9024) << "Project Read: "<< ast->statements().count() << " Top-Level Statements" << endl;
            if( !silent )
                kDebug(9024) << "-------------\n" << buf << "-------------\n";
        }
    }

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
