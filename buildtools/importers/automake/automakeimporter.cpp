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

#include "automakeimporter.h"

#include <QList>

#include <kgenericfactory.h>
#include "kdevproject.h"
#include "kdevprojectmodel.h"

#include "automakeprojectmodel.h"
#include "autotoolsast.h"
#include "autotoolsdriver.h"

K_EXPORT_COMPONENT_FACTORY( libkdevautomakeimporter,
                            KGenericFactory<AutoMakeImporter>( "kdevautomakeimporter" ) )

AutoMakeImporter::AutoMakeImporter( QObject* parent, const char* name,
                                    const QStringList& )
: KDevProjectEditor( parent )
, m_rootItem(0L)
{
	setObjectName( QString::fromUtf8( name ) );
	m_project = qobject_cast<KDevProject*>( parent );
	Q_ASSERT( m_project );
}

AutoMakeImporter::~AutoMakeImporter()
{
	//delete m_rootItem;
}

KDevProject* AutoMakeImporter::project() const
{
	return m_project;
}

KDevProjectEditor* AutoMakeImporter::editor() const
{
	return const_cast<AutoMakeImporter*>( this );
}

QList<KDevProjectFolderItem*> AutoMakeImporter::parse( KDevProjectFolderItem* dom )
{
	Q_UNUSED( dom );
	return QList<KDevProjectFolderItem*>();
}

KDevProjectItem* AutoMakeImporter::import( KDevProjectModel* model,
                                           const QString& fileName )
{
	Q_UNUSED( model );
	m_rootItem = new AutoMakeDirItem( fileName, 0 );
	AutoTools::ProjectAST* ast;
	int ret = -1;
	QString filePath = fileName + "/Makefile.am.in";
	if ( QFile::exists( filePath ) )
		ret = AutoTools::Driver::parseFile( filePath, &ast );
	else
	{
		filePath = fileName + "/Makefile.am";
		if ( QFile::exists( filePath ) )
			ret = AutoTools::Driver::parseFile( filePath, &ast );
		else
		{
			filePath = fileName + "/Makefile.in";
			if ( QFile::exists( filePath ) )
				ret = AutoTools::Driver::parseFile( filePath, &ast );
			else
				kdDebug(9020) << k_funcinfo << "no appropriate file to parse in "
				              << fileName << endl;
		}
	}
	
    if ( ret != 0 )
	    return m_rootItem;

	
    kdDebug(9020) << k_funcinfo << filePath << " was parsed correctly. Adding information" << endl;
    Q_ASSERT( ast != 0 );
	
	if ( ast && ast->hasChildren() )
	{
		QList<AutoTools::AST*> astChildList = ast->children();
		QList<AutoTools::AST*>::const_iterator it, itEnd = astChildList.constEnd();
		for ( it = astChildList.constBegin(); it != itEnd; ++it )
		{
			if ( (*it)->nodeType() == AutoTools::AST::AssignmentAST )
			{
				AutoTools::AssignmentAST* assignment = static_cast<AutoTools::AssignmentAST*>( (*it) );
				if ( assignment->scopedID == "SUBDIRS"  )
				{
					kdDebug(9020) << k_funcinfo << "subdirs is " << assignment->values << endl;
					foreach( const QString& s, assignment->values )
					{
						QString dir = fileName + "/" + s;
						new AutoMakeDirItem( dir, m_rootItem );
					}
				}
			}
		}
	}
		
	/* old code
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
	*/
	return m_rootItem;
}

QString AutoMakeImporter::findMakefile( KDevProjectFolderItem* dom ) const
{
	Q_UNUSED( dom );
	return QString();
}

QStringList AutoMakeImporter::findMakefiles( KDevProjectFolderItem* dom ) const
{
	Q_UNUSED( dom );
	return QStringList();
}

#include "automakeimporter.h"
// kate: indent-mode csands; space-indent off; tab-width 4; auto-insert-doxygen on;


