/***************************************************************************
 copyright            : (C) 2007 by David Nolden
 email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HELPERS_H
#define HELPERS_H

#include "teamworkfwd.h"

///This header contains little helper-functions that have no other dependencies than teamworkfwd.h

/**Returns the user the session is pointing to, and returns zero if the session is zero.
 * */
Teamwork::UserPointer userFromSession( const Teamwork::SessionPointer& session );

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
