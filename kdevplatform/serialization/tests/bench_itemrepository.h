/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BENCHITEMREPOSITORY_H
#define BENCHITEMREPOSITORY_H

#include "itemrepositorytestbase.h"

class BenchItemRepository
    : public ItemRepositoryTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void insert();
    void remove();
    void removeDisk();
    void lookupKey();
    void lookupValue();

    void shouldDoReferenceCounting_data();
    void shouldDoReferenceCounting();
};

#endif // BENCHITEMREPOSITORY_H
