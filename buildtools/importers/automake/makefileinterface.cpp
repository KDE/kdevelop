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

QString MakefileInterface::canonicalize( const QString& target )
{
    QString result;
    for ( int i = 0; i < target.length();  ++i )
    {
        QChar addition = target.at( i );
        if ( !addition.isLetterOrNumber() && addition != '@' && addition != '_' )
            addition = QChar( '_' );

        result += addition;
    }

    kDebug(9020) << k_funcinfo << "'" << target << "' has been normalized to '"
                   << result << "'" << endl;

    return result;
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
            if ( ret != -1 )
                break;
        }
    }

    if ( !ast || ret == -1 )
    {
        kWarning(9020) << k_funcinfo << "parsing " << dir.absolutePath()
                << " not successful! Fix your Makefile!" << endl;
        return false;
    }

    d->projects[parsingFile] = ast;

    if ( recursive == NonRecursive || ( ast && !ast->hasChildren() ) )
        return ( ret != -1 );


    QStringList subdirs = subdirsFor( dir );
    foreach( QString sd, subdirs )
    {
        kDebug(9020) << k_funcinfo << "Beginning parsing of '" << sd << "'" << endl;
        parse( dir.absolutePath() + '/' + sd, recursive );
    }

    return (ret != -1);
}

void MakefileInterface::setProjectRoot( const QDir& dir )
{
    d->topLevelParseDir = dir;
}

QString MakefileInterface::projectRoot() const
{
    return d->topLevelParseDir.absolutePath();
}

QStringList MakefileInterface::topSubDirs() const
{
    return subdirsFor( d->topLevelParseDir );
}

AutoTools::ProjectAST* MakefileInterface::astForFolder( const QDir& folder ) const
{
    ProjectAST* ast = 0;
    QFileInfo parsingFile;
    QStringList::const_iterator it, itEnd = d->filesToParse.constEnd();
    for ( it = d->filesToParse.constBegin(); it != itEnd; ++it )
    {
        parsingFile.setFile( folder, (*it) );
        if ( parsingFile.exists() )
            ast = d->projects[parsingFile];

        if ( ast != 0 )
            break;

    }

    return ast;
}

bool MakefileInterface::isVariable( const QString& item ) const
{
    if ( item.contains( QRegExp( "(\\$\\([a-zA-Z0-9_-]*\\)|@[a-zA-Z0-9_-]*@)" ) ) )
        return true;
    else
        return false;
}

QString MakefileInterface::resolveVariable( const QString& variable, AutoTools::ProjectAST* ast ) const
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
                QString resolution = assignment->values.join( " " ).trimmed();
                kDebug(9020) << k_funcinfo << "Resolving variable '" << variable << "' to '"
                              << resolution << "'" << endl;
                return resolution;
            }
        }
    }

    return variable;
}

QStringList MakefileInterface::subdirsFor( const QDir& folder ) const
{
    AutoTools::ProjectAST* ast = astForFolder( folder );
    if ( !ast )
    {
        kWarning(9020) << k_funcinfo << "Couldn't find AST for " 
                << folder.absolutePath() << endl;
        return QStringList();
    }

    return subdirsFor( ast );
}

QList<TargetInfo> MakefileInterface::targetsForFolder( const QDir& folder ) const
{
    QList<TargetInfo> targetList;
    AutoTools::ProjectAST* ast = astForFolder( folder );

    if ( !ast )
    {
        kWarning(9020) << k_funcinfo << "Unable to get AST for "
                << folder.absolutePath() << endl;
        return targetList;
    }

    return targetList;
}

QStringList MakefileInterface::subdirsFor( AutoTools::ProjectAST* ast ) const
{
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

                QStringList::const_iterator vit = subdirList.constBegin();
                for ( ; vit != subdirList.constEnd(); ++vit )
                {
                    QString dirFromList = ( *vit );
                    QString realDir = dirFromList;
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

                        subdirList.replace( subdirList.indexOf( dirFromList ),
                                            realDir );
                    }
                }
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

