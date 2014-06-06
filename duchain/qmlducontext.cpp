/*************************************************************************************
*  Copyright (C) 2014 by Denis Steckelmacher <steckdenis@yahoo.fr>                  *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/
#include "qmlducontext.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/duchainregister.h>

REGISTER_DUCHAIN_ITEM_WITH_DATA(QmlDUContext, QmlDUContextData);

QmlDUContextData::QmlDUContextData(const QmlDUContextData& rhs)
: DUContextData(rhs),
  m_searchInParentsEnabled(true)
{
}

QmlDUContextData::QmlDUContextData()
: DUContextData(),
  m_searchInParentsEnabled(true)
{
}

QmlDUContext::QmlDUContext(QmlDUContextData& dd, const KDevelop::RangeInRevision& range, KDevelop::DUContext* parent, bool anonymous)
: DUContext(dd, range, parent, anonymous)
{
    // Inform KDevPlatform that QmlDUContext must be stored in the item repository
    // with the QmlDUContext::Identity identifier
    d_func_dynamic()->setClassId(this);
}

QmlDUContext::QmlDUContext(const KDevelop::RangeInRevision& range, KDevelop::DUContext* parent, bool anonymous)
: DUContext(*new QmlDUContextData(), range, parent, anonymous)
{
    d_func_dynamic()->setClassId(this);
}

QmlDUContext::QmlDUContext(KDevelop::DUContext& useDataFrom): DUContext(useDataFrom)
{
    d_func_dynamic()->setClassId(this);
}

QmlDUContext::QmlDUContext(QmlDUContextData& dd): DUContext(dd)
{
    d_func_dynamic()->setClassId(this);
}

void QmlDUContext::disableSearchInParents()
{
    d_func_dynamic()->m_searchInParentsEnabled = false;

    // The top-level context should still be explored
    if (topContext()) {
        addImportedParentContext(topContext());
    }
}

void QmlDUContext::mergeDeclarationsInternal(QList<QPair<KDevelop::Declaration*, int>>& definitions,
                                             const KDevelop::CursorInRevision& position,
                                             QHash<const DUContext*, bool>& hadContexts,
                                             const KDevelop::TopDUContext* source,
                                             bool searchInParents,
                                             int currentDepth) const
{
    KDevelop::DUContext::mergeDeclarationsInternal(definitions,
                                                   position,
                                                   hadContexts,
                                                   source,
                                                   searchInParents && d_func()->m_searchInParentsEnabled,
                                                   currentDepth);
}

bool QmlDUContext::shouldSearchInParent(KDevelop::DUContext::SearchFlags flags) const
{
    return d_func()->m_searchInParentsEnabled && KDevelop::DUContext::shouldSearchInParent(flags);
}
