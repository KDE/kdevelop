/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "includeitem.h"

#include <QDebug>

#include <util/path.h>

using namespace KDevelop;

IncludeItem::IncludeItem()
{
}

///Constructs the url from basePath and name.
// TODO: port this to Path
QUrl IncludeItem::url() const
{
    QUrl u;
    if (!basePath.isEmpty()) {
        u = Path(Path(basePath), name).toUrl();
    } else {
        u = QUrl::fromLocalFile(name);
    }
    return u;
}

QDebug operator<<(const QDebug& dbg, const IncludeItem& item)
{
    return dbg << item.url();
}
