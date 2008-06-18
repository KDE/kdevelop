/* This file is part of KDevelop
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//If a file is defined here, the context-, type-, and declaration-builder will show additional
//information while processing that file, but not while processing other files. This is perfect
//for debugging specific reproducable problems in a runtime environment.
//#define DEBUG_FILE "/home/nolden/kdedev/install-mini/include/kdevplatform/interfaces/icore.h"

#ifdef DEBUG_FILE
#include <hashedstring.h>
static HashedString globalDebuggedFileName(DEBUG_FILE);
#define ifDebugCurrentFile(action) if(currentContext()->url() == globalDebuggedFileName) { action }

#define ifDebugFile(fileName, action) if(fileName == globalDebuggedFileName) { action }
#else
#define ifDebugCurrentFile(x)
#define ifDebugFile(fileName, action)
#endif


