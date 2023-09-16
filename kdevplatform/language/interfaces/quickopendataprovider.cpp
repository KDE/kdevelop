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

bool extractLineNumber(const QString& from, QString& path, uint& lineNumber)
{
    int colonIndex = from.indexOf(QLatin1Char(':'));
    if (colonIndex != -1) {
        if (colonIndex == from.count() - 1) {
            path = from.mid(0, colonIndex);
            lineNumber = 0;
        } else {
            bool ok;
            uint number = QStringView(from).mid(colonIndex + 1).toUInt(&ok);
            if (ok) {
                path = from.mid(0, colonIndex);
                lineNumber = number;
            } else {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}
}

#include "moc_quickopendataprovider.cpp"
