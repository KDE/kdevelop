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

static const KCmdLineOptions options[] =
{
    {"silent", I18N_NOOP("Enable Parser debug output"), 0},
    {"!debug", I18N_NOOP("Disable output of the generated AST"), 0},
    {"!+files", I18N_NOOP("QMake project files"), 0}
};

int main( int argc, char* argv[] )
{
    KCmdLineArgs::init( argc, argv, "QMake Parser", "qmake-parser", I18N_NOOP("Parse QMake project files"), "4.0.0");
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
        QMake::ProjectAST* ast;
        if ( QMake::Driver::parseFile( args->url(i).toLocalFile(), &ast, debug ) != 0 ) {
            exit( EXIT_FAILURE );
        }else
        {
            QString buf;
            if( !silent )
                ast->writeToString( buf );
            kDebug(9024) << "Project Read: "<< ast->statements().count() << " Top-Level Statements" << endl;
            if( !silent )
                kDebug(9024) << "-------------\n" << buf << "\n-------------\n";
        }
    }

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

