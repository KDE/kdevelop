/*
  KDevelop Autotools Support
  Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

***************************************************************************
*									  *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or	  *
*   (at your option) any later version.					  *
*									  *
***************************************************************************
*/

#include <qdir.h>
#include <qglobal.h>
#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kdebug.h>

#include <autotoolsast.h>
#include <autotoolsdriver.h>

#include "makefilehandler.h"

class MakefileHandler::Private
{
public:
	QMap<QString, AutoTools::ProjectAST*> projects;
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
		kdDebug(9020) << k_funcinfo << filePath << " is valid, but could not be parsed" << endl;
		return;
	}

	kdDebug(9020) << k_funcinfo << filePath << " was parsed correctly. Adding information" << endl;
	kdDebug(9020) << k_funcinfo << "pointer to AST is " << ast << endl;
	Q_ASSERT( ast != 0 );
	d->projects[filePath] = ast;

	if ( recursive && ast && ast->hasChildren() )
	{
		QValueList<AutoTools::AST*> astChildList = ast->children();
		QValueList<AutoTools::AST*>::iterator it(astChildList.begin()), clEnd(astChildList.end());
		for ( ; it != clEnd; ++it )
		{
			if ( (*it)->nodeType() == AutoTools::AST::AssignmentAST )
			{
				AutoTools::AssignmentAST* assignment = static_cast<AutoTools::AssignmentAST*>( (*it) );
				if ( assignment->scopedID == "SUBDIRS"	)
				{
					kdDebug(9020) << k_funcinfo << "found SUBDIRS assignment '"
							<< assignment->scopedID << "'" << endl;
					kdDebug(9020) << k_funcinfo << "subdirs is " << assignment->values << endl;
					QStringList::iterator vit = assignment->values.begin();
					for ( ; vit != assignment->values.end(); ++vit )
					{
					    parse( folder + '/' + ( *vit ), recursive );
					}
				}
			}
		}
	}
}

void MakefileHandler::astForFolder( const QString& folderPath )
{
	Q_UNUSED( folderPath );
}




//kate: space-indent off; tab-width 4; indent-mode cstyle;
