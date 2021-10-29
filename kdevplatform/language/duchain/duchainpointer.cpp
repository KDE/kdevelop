/*
    SPDX-FileCopyrightText: 2007 Bernd Buschinski <b.buschinski@web.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchainpointer.h"

namespace KDevelop {
DUChainBase* DUChainPointerData::base()
{
    return m_base;
}

DUChainBase* DUChainPointerData::base() const
{
    return m_base;
}

DUChainPointerData::DUChainPointerData()
{
}

DUChainPointerData::~DUChainPointerData()
{
}

DUChainPointerData::DUChainPointerData(DUChainBase* base)
    : m_base(base)
{
}
} //KDevelop
