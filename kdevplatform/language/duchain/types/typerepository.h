/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_TYPEREPOSITORY_H
#define KDEVPLATFORM_TYPEREPOSITORY_H

#include <language/duchain/types/abstracttype.h>

class QRecursiveMutex;

namespace KDevelop {
struct ReferenceCountManager;

class TypeRepository
{
public:
    static uint indexForType(const AbstractType::Ptr& input);
    static AbstractType::Ptr typeForIndex(uint index);
    static void increaseReferenceCount(uint index);
    static void decreaseReferenceCount(uint index);
    static void increaseReferenceCount(uint index, ReferenceCountManager* manager);
    static void decreaseReferenceCount(uint index, ReferenceCountManager* manager);
};

QRecursiveMutex* typeRepositoryMutex();
}

#endif
