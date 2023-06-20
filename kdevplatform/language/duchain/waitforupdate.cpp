/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "waitforupdate.h"

using namespace KDevelop;

WaitForUpdate::WaitForUpdate()
    : m_ready(false)
{
}

void WaitForUpdate::updateReady(const KDevelop::IndexedString& /*url*/,
                                const KDevelop::ReferencedTopDUContext& topContext)
{
    m_ready = true;
    m_topContext = topContext;
}

#include "moc_waitforupdate.cpp"
