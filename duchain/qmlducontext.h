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
#ifndef __QMLDUCONTEXT_H__
#define __QMLDUCONTEXT_H__

#include "duchainexport.h"

#include <language/duchain/ducontext.h>
#include <language/duchain/ducontextdata.h>

class KDEVQMLJSDUCHAIN_EXPORT QmlDUContextData : public KDevelop::DUContextData
{
    public:
        QmlDUContextData(const QmlDUContextData& rhs);
        QmlDUContextData();

        bool m_searchInParentsEnabled;
};

/**
 * @brief DUContext with some QML-related features
 */
class KDEVQMLJSDUCHAIN_EXPORT QmlDUContext : public KDevelop::DUContext
{
    public:
        QmlDUContext(QmlDUContextData& dd, const KDevelop::RangeInRevision& range, DUContext* parent = 0, bool anonymous = false);
        explicit QmlDUContext(const KDevelop::RangeInRevision& range, DUContext* parent = 0, bool anonymous = false);
        QmlDUContext(DUContext& useDataFrom);
        explicit QmlDUContext(QmlDUContextData& dd);

        /**
         * @brief Prevent this context from finding declarations in its parents
         * @note The top-level context and imported parent contexts are still
         *       explored
         */
        void disableSearchInParents();

        /**
         * @brief Find the declarations visible from this context
         *
         * This method simply overloads the one of DUContext and ensures that
         * searchInParents is set to false when searching in parents is disabled
         * for this context.
         */
        virtual void mergeDeclarationsInternal(QList<QPair<KDevelop::Declaration*, int>>& definitions,
                                               const KDevelop::CursorInRevision& position,
                                               QHash<const DUContext*, bool>& hadContexts,
                                               const KDevelop::TopDUContext* source,
                                               bool searchInParents = true,
                                               int currentDepth = 0) const;

        /**
         * @brief Return whether the parent of this context should be explored or
         *        not when looking for a declaration
         *
         * This method returns false when searching in parents is disabled for this
         * context. Otherwise, it behaves exactly like DUContext::shouldSearchInParent.
         */
        virtual bool shouldSearchInParent(SearchFlags flags) const;

        enum {
            Identity = 110
        };

    private:
        DUCHAIN_DECLARE_DATA(QmlDUContext)
};

#endif