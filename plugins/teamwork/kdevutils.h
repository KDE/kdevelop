/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVUTILS_H
#define KDEVUTILS_H
#include <QString>

/** Returns the path of the current document,
 * relative to the workspace-directory.
 * 
 * Throws a QString on error */
QString currentDocumentPath();

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
