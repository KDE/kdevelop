/***************************************************************************
   copyright            : (C) 2006 by David Nolden
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

#ifndef INTERFACES_H
#define INTERFACES_H

#include "common.h"
#include "pointer.h"

class SafeShared;
class MutexInterface;
struct MessageSerialization;

namespace Teamwork {
	class MessageInterface;
	typedef SafeSharedPtr<MessageInterface, MessageSerialization> MessagePointer;
	}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
