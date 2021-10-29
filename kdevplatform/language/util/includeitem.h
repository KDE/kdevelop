/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_INCLUDEITEM_H
#define KDEVPLATFORM_INCLUDEITEM_H

#include <QUrl>

#include <language/languageexport.h>

class QDebug;

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT IncludeItem
{
public:
    IncludeItem();

    ///Constructs the url from basePath and name.
    QUrl url() const;

    ///The name of this include-item, starting behind basePath.
    QString name;
    ///basePath + name = Absolute path of file
    QUrl basePath;
    ///Which path in the include-path was used to find this item?
    int pathNumber = 0;
    ///If this is true, this item represents a sub-directory. Else it represents a file.
    bool isDirectory = false;
};
}

KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(const QDebug& dbg, const KDevelop::IncludeItem& item);
Q_DECLARE_TYPEINFO(KDevelop::IncludeItem, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_INCLUDEITEM_H
