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

class MessageManager;
class CollaborationManager;
class MessageSendManager;
class PatchesManager;
class KUrl;
class KDevTeamworkPart;
class KDevTeamwork;
class QStandardItemModel;
class QTimer;
class QListView;
class QWidget;
class QModeIndex;
class MessageSerialization;
class BoostSerialization;
class Ui_List;
class KDevTeamworkLogger;
class MessageUserTab;
class QPersistentModelIndex;
class TeamworkFolderManager;
class IconCache;
class PatchesListMessage;
class KDevSystemMessage;
class KDevTeamworkTextMessage;
class PatchesManagerMessage;

namespace KDevelop {
  class ICore;
  class IDocumentController;
  class IDocument;
};

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
