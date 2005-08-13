/*
  KDevelop Autotools Support
  Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
*/

#include <qdir.h>
#include <qglobal.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kdebug.h>

#include <autotoolsast.h>
#include <autotoolsdriver.h>

#include "makefilehandler.h"

typedef QValueList<AutoTools::AST*> ASTList;

class MakefileHandler::Private
{
public:
    QMap<QString, AutoTools::ProjectAST*> projects;
    QMap<QString, QString> folderToFileMap;
};

MakefileHandler::MakefileHandler()
{
    d = new MakefileHandler::Private;
}

MakefileHandler::~MakefileHandler()
{
    delete d;
}

void MakefileHandler::parse( const QString& folder, bool recursive )
{
    //look for either Makefile.am.in, Makefile.am, or Makefile.in, in that order
    AutoTools::ProjectAST* ast;
    int ret = -1;
    QString filePath = folder + "/Makefile.am.in";
    if ( QFile::exists( filePath ) )
        ret = AutoTools::Driver::parseFile( filePath, &ast );
    else
    {
        filePath = folder + "/Makefile.am";
        if ( QFile::exists( filePath ) )
            ret = AutoTools::Driver::parseFile( filePath, &ast );
        else
        {
            filePath = folder + "/Makefile.in";
            if ( QFile::exists( filePath ) )
                ret = AutoTools::Driver::parseFile( filePath, &ast );
            else
                kdDebug(9020) << k_funcinfo << "no appropriate file to parse in "
                              << folder << endl;
        }
    }

    if ( ret != 0 )
    {
        return;
    }

    kdDebug(9020) << k_funcinfo << filePath << " was parsed correctly. Adding information" << endl;
    Q_ASSERT( ast != 0 );
    d->projects[filePath] = ast;
    d->folderToFileMap[folder] = filePath;

    if ( recursive && ast && ast->hasChildren() )
    {
        QValueList<AutoTools::AST*> astChildList = ast->children();
        QValueList<AutoTools::AST*>::iterator it(astChildList.begin()), clEnd(astChildList.end());
        for ( ; it != clEnd; ++it )
        {
            if ( (*it)->nodeType() == AutoTools::AST::AssignmentAST )
            {
                AutoTools::AssignmentAST* assignment = static_cast<AutoTools::AssignmentAST*>( (*it) );
                if ( assignment->scopedID == "SUBDIRS"  )
                {
                    QString list = assignment->values.join( QString::null );
                    list.simplifyWhiteSpace();
                    kdDebug(9020) << k_funcinfo << "subdirs is " << list << endl;
                    QStringList subdirList = QStringList::split( " ",  list );
                    QStringList::iterator vit = subdirList.begin();
                    for ( ; vit != subdirList.end(); ++vit )
                    {
                        QString realDir = ( *vit );
                        if ( realDir.startsWith( "\\" ) )
                            realDir.remove( 0, 1 );

                        realDir = realDir.stripWhiteSpace();
                        if ( realDir != "." && realDir != ".." && !realDir.isEmpty() )
                        {
                            if ( isVariable( realDir ) )
                            {
                                kdDebug(9020) << k_funcinfo << "'" << realDir << "' is a variable" << endl;
                                realDir = resolveVariable( realDir, ast );
                            }

                            kdDebug(9020) << k_funcinfo << "Beginning parsing of '" << realDir << "'" << endl;
                            parse( folder + '/' + realDir, recursive );
                        }
                    }
                }
            }
        }
    }
}

AutoTools::ProjectAST* MakefileHandler::astForFolder( const QString& folderPath )
{
    if ( d->folderToFileMap.contains( folderPath ) )
    {
        QString filePath = d->folderToFileMap[folderPath];
        return d->projects[filePath];
    }
    else
        return 0;
}

bool MakefileHandler::isVariable( const QString& item ) const
{
    if ( item.contains( QRegExp( "(\\$\\([a-zA-Z0-9_-]*\\)|@[a-zA-Z0-9_-]*@)" ) ) )
        return true;
    else
        return false;
}

QString MakefileHandler::resolveVariable( const QString& variable, AutoTools::ProjectAST* ast )
{
    if ( !ast )
        return variable;

    kdDebug(9020) << k_funcinfo << "attempting to resolve '" << variable << "'"<< endl;
    ASTList childList = ast->children();
    ASTList::iterator it( childList.begin() ), clEnd( childList.end() );
    for ( ; it != clEnd; ++it )
    {
        if ( ( *it )->nodeType() == AutoTools::AST::AssignmentAST )
        {
            AutoTools::AssignmentAST* assignment = static_cast<AutoTools::AssignmentAST*>( ( *it ) );
            if ( variable.find( assignment->scopedID ) != -1 )
            {
                kdDebug(9020) << k_funcinfo << "Resolving variable '" << variable << "' to '"
                              << assignment->values.join( QString::null ).stripWhiteSpace() << "'" << endl;
                return assignment->values.join( QString::null ).stripWhiteSpace();
            }
        }
    }

    return variable;
}
//kate: space-indent on; indent-width 4;
