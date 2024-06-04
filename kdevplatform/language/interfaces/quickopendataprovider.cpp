/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "quickopendataprovider.h"
#include <QVariant>

#include <QIcon>

namespace KDevelop {
QuickOpenFileSetInterface::~QuickOpenFileSetInterface()
{
}

QuickOpenEmbeddedWidgetInterface::~QuickOpenEmbeddedWidgetInterface()
{
}

QuickOpenDataBase::~QuickOpenDataBase()
{
}
QIcon QuickOpenDataBase::icon() const
{
    return QIcon();
}

bool QuickOpenDataBase::isExpandable() const
{
    return false;
}

QWidget* QuickOpenDataBase::expandingWidget() const
{
    return nullptr;
}

QList<QVariant> QuickOpenDataBase::highlighting() const
{
    return QList<QVariant>();
}

QuickOpenDataProviderBase::~QuickOpenDataProviderBase()
{
}

void QuickOpenDataProviderBase::enableData(const QStringList&, const QStringList&)
{
}
}

#include "moc_quickopendataprovider.cpp"
