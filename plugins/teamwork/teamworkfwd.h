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
 
#ifndef TEMWORKFWD_H
#define TEMWORKFWD_H

#include "network/networkfwd.h"

class PatchesManager;
class KDevTeamwork;
class BoostSerialization;
class KDevTeamworkTextMessage;
class PatchesManagerMessage;


class KDevTeamworkUser;
typedef SafeSharedPtr< KDevTeamworkUser, BoostSerialization > KDevTeamworkUserPointer;
class KDevTeamworkClient;
typedef SafeSharedPtr< KDevTeamworkClient > KDevTeamworkClientPointer;
typedef SafeSharedPtr< KDevTeamworkClient > TeamworkClientPointer;

class ConnectionRequest;
typedef SafeSharedPtr<ConnectionRequest > ConnectionRequestPointer;

typedef SafeSharedPtr< KDevTeamworkTextMessage > KDevTextMessagePointer;
typedef SafeSharedPtr< ConnectionRequest > ConnectionRequestPointer;
typedef SafeSharedPtr< PatchesManagerMessage > PatchesManagerMessagePointer;

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
