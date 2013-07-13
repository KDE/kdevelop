/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_TYPEREPOSITORY_H
#define KDEVPLATFORM_TYPEREPOSITORY_H

#include <language/duchain/types/abstracttype.h>

namespace KDevelop {

class ReferenceCountManager;
class AbstractRepositoryManager;

class TypeRepository
{
public:
    static uint indexForType(AbstractType::Ptr input);
    static AbstractType::Ptr typeForIndex(uint index);
    static void increaseReferenceCount(uint index);
    static void decreaseReferenceCount(uint index);
    static void increaseReferenceCount(uint index, ReferenceCountManager* manager);
    static void decreaseReferenceCount(uint index, ReferenceCountManager* manager);
};

AbstractRepositoryManager* typeRepositoryManager();

}

#endif
