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

#include "helpers.h"
#include "network/networkfwd.h"
#include "network/sessioninterface.h"
#include "network/user.h"

Teamwork::UserPointer userFromSession( const Teamwork::SessionPointer& session ) {
  if( !session ) return 0;
  return session.unsafe() ->safeUser();
}


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
