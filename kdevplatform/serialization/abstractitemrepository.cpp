/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "abstractitemrepository.h"

#include "config-kdevplatform.h"

namespace KDevelop {
uint staticItemRepositoryVersion()
{
    return KDEV_ITEMREPOSITORY_VERSION;
}

AbstractItemRepository::~AbstractItemRepository()
{
}
}
