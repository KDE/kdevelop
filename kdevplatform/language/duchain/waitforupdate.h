/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_WAITFORUPDATE_H
#define KDEVPLATFORM_WAITFORUPDATE_H

#include "duchain/topducontext.h"

#include <serialization/indexedstring.h>

#include <QObject>

namespace KDevelop {
struct WaitForUpdate
    : public QObject
{
    Q_OBJECT

public:
    WaitForUpdate();

public Q_SLOTS:
    void updateReady(const KDevelop::IndexedString& url,
                     const KDevelop::ReferencedTopDUContext& topContext);

public:
    bool m_ready;
    ReferencedTopDUContext m_topContext;
};
}

#endif // KDEVPLATFORM_WAITFORUPDATE_H
