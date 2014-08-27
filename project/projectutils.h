/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  David Nolden <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef KDEVPLATFORM_PROJECTUTILS_H
#define KDEVPLATFORM_PROJECTUTILS_H

#include "projectexport.h"

#include <QList>

class QMenu;

namespace KDevelop {

class ProjectBaseItem;
class ProjectFileItem;

/**
 * Adds menu entries for all parent folders of the given item, each containing all the project
 * items for the folder, at the end of the given menu.
 * */
KDEVPLATFORMPROJECT_EXPORT void populateParentItemsMenu( ProjectBaseItem* item, QMenu* menu );

/**
 * Returns all the files that have @p projectItem as ancestor
 */
KDEVPLATFORMPROJECT_EXPORT QList<ProjectFileItem*> allFiles(ProjectBaseItem* projectItem);

}

#endif // KDEVPLATFORM_PROJECTUTILS_H
