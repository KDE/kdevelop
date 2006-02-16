/* KDevelop Automake Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "makefileinterface.h"

#include <QtAlgorithms>
#include <QFileInfo>
#include <QDir>
#include <QList>
#include <QHash>

#include <kdebug.h>

#include "autotoolsast.h"
#include "autotoolsdriver.h"


uint qHash( const QFileInfo& key )
{
	return qHash( key.absolutePath() );
}

using namespace AutoTools;

class MakefileInterface::Private
{
public:
    ASTHash projects;
    QStringList filesToParse;
    QDir topLevelParseDir;
};

MakefileInterface::MakefileInterface( QObject* parent )
: QObject( parent )
{
    d = new MakefileInterface::Private;
    d->filesToParse.append("Makefile.am.in");
    d->filesToParse.append("Makefile.am");
    d->filesToParse.append("Makefile.in");
}

MakefileInterface::~MakefileInterface()
{
    qDeleteAll( d->projects );
    delete d;
}

bool MakefileInterface::parse( const QDir& dir, ParserRecursion recursive )
{
    kDebug(9020) << k_funcinfo << "directory to parse is: " << dir.absolutePath() << endl; 
    int ret = -1;
    AutoTools::ProjectAST* ast;

    QFileInfo parsingFile;
    QStringList::const_iterator it, itEnd = d->filesToParse.constEnd();
    for ( it = d->filesToParse.constBegin(); it != itEnd; ++it )
    {
        parsingFile.setFile( dir, (*it) );
        if ( parsingFile.exists() )
        {
            using namespace AutoTools;
            ret = Driver::parseFile( parsingFile.absoluteFilePath(), &ast );
            if ( ret == 0 )
                break;
        }
    }

    if ( !ast || ret != 0 )
    {
        kWarning(9020) << k_funcinfo << "parsing " << dir.absolutePath()
                << "not successful! Fix your Makefile!" << endl;
        return false;
    }

    d->projects[parsingFile] = ast;

    if ( recursive == NonRecursive || ( ast && !ast->hasChildren() ) )
        return ( ret != -1 );


    QList<AST*> childList = ast->children();
    QList<AST*>::const_iterator cit, citEnd = childList.constEnd();
    for ( cit = childList.constBegin(); cit != citEnd; ++cit )
    {
        if ( (*cit)->nodeType() == AST::AssignmentAST )
        {
            AssignmentAST* assignment = static_cast<AssignmentAST*>( (*cit) );
            if ( assignment->scopedID == "SUBDIRS"  )
            {
                QStringList subdirList = assignment->values;
                subdirList.remove(".");
                subdirList.remove("..");
                subdirList.removeAll("\\");
                subdirList.removeAll("#");
                kDebug(9020) << k_funcinfo << "subdirs is '" 
                              << assignment->values << "'" << endl;
                QStringList::const_iterator vit = subdirList.constBegin();
                for ( ; vit != subdirList.constEnd(); ++vit )
                {
                    QString realDir = ( *vit );
                    if ( realDir.startsWith( "\\" ) )
                    realDir.remove( 0, 1 );

                    realDir = realDir.trimmed();
                    if ( !realDir.isEmpty() )
                    {
                        if ( isVariable( realDir ) )
                        {
                            kDebug(9020) << k_funcinfo << "'" << realDir << "' is a variable" << endl;
                            realDir = resolveVariable( realDir, ast );
                        }

                    kDebug(9020) << k_funcinfo << "Beginning parsing of '" << realDir << "'" << endl;
                    parse( dir.absolutePath() + '/' + realDir, recursive );
                    }
                }
            }
        }
    }

    return (ret != -1);
}

void MakefileInterface::setProjectRoot( const QDir& dir )
{
    d->topLevelParseDir = dir;
}

QStringList MakefileInterface::topSubDirs() const
{
    return subdirsFor( d->topLevelParseDir );
}

bool MakefileInterface::isVariable( const QString& item ) const
{
    if ( item.contains( QRegExp( "(\\$\\([a-zA-Z0-9_-]*\\)|@[a-zA-Z0-9_-]*@)" ) ) )
        return true;
    else
        return false;
}

QString MakefileInterface::resolveVariable( const QString& variable, AutoTools::ProjectAST* ast )
{
    if ( !ast )
        return variable;

    kDebug(9020) << k_funcinfo << "attempting to resolve '" << variable << "'"<< endl;
    QList<AST*> childList = ast->children();
    QList<AST*>::iterator it( childList.begin() ), clEnd( childList.end() );

    for ( ; it != clEnd; ++it )
    {
        if ( ( *it )->nodeType() == AST::AssignmentAST )
        {
            AssignmentAST* assignment = static_cast<AssignmentAST*>( ( *it ) );
            if ( variable.find( assignment->scopedID ) != -1 )
            {
                kDebug(9020) << k_funcinfo << "Resolving variable '" << variable << "' to '"
                              << assignment->values.join( QString::null ).trimmed() << "'" << endl;
                return assignment->values.join( QString::null ).trimmed();
            }
        }
    }

    return variable;
}

QStringList MakefileInterface::subdirsFor( const QDir& folder ) const
{
    ProjectAST* ast = 0;
    QList<QFileInfo> projectFileInfoList = d->projects.keys();
    QList<QFileInfo>::const_iterator it, itEnd = projectFileInfoList.constEnd();
    for ( it = projectFileInfoList.constBegin(); it != itEnd; ++it )
    {
        if ( (*it).dir() == folder )
        {
            ast = d->projects.value( (*it), 0 );
            break;
        }
    }

    if ( !ast )
    {
        kDebug(9020) << k_funcinfo << "Couldn't find AST for " 
                      << folder.absolutePath() << endl;
        return QStringList();
    }

    QList<AST*> childList = ast->children();
    QList<AST*>::const_iterator cit, citEnd = childList.constEnd();
    for ( cit = childList.constBegin(); cit != citEnd; ++cit )
    {
        if ( (*cit)->nodeType() == AST::AssignmentAST )
        {
            AssignmentAST* assignment = static_cast<AssignmentAST*>( (*cit) );
            if ( assignment->scopedID == "SUBDIRS"  )
            {
                QStringList subdirList = assignment->values;
                subdirList.remove(".");
                subdirList.remove("..");
                subdirList.removeAll("\\");
                subdirList.removeAll("#");
                kDebug(9020) << k_funcinfo << "subdirs is '" 
                              << assignment->values << "'" << endl;
                return subdirList;
            }
        }
    }

    return QStringList();
}

#include "makefileinterface.moc"
// kate: space-indent on; indent-width 4; auto-insert-doxygen on; replace-tabs on; indent-mode cstyle;

