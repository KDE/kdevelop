/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Röder                                    *
*   victor_roeder@gmx.de                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef AUTOMAKEMANAGER_H
#define AUTOMAKEMANAGER_H

class SubprojectItem;
class TargetItem;

class AutomakeManager
{
 	public:
 		AutomakeManager();
 		virtual ~AutomakeManager();
 		
// 	public:
// 		addFileToTarget ( const QString& file, SubprojectItem* spitem, TargetItem* titem );
// 		removeFileFromTarget ( const QString& file, SubprojectItem* spitem, TargetItem* titem );
// 		
// 		addTargetToSubproject ( const TargetItem& titem, SubprojectItem* spitem );
// 		removeTargetFromSubproject ( const TargetItem& titem, SubprojectItem* spitem );
// 		
// 		addSubprojectToSubproject ( const SubprojectItem& spitemAdd, SubprojectItem* spitemTo );
// 		removeSubprojectFromSubproject ( const SubprojectItem& spitemRemove, SubprojectItem* spitemFrom );
};


#endif
