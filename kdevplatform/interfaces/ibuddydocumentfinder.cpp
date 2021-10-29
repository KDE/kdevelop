/*
    SPDX-FileCopyrightText: 2011 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2011 Martin Heide <martin.heide@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ibuddydocumentfinder.h"

#include <QMap>

namespace KDevelop {

//Our private data is entirely static, so don't need to create an
//instance of the private data class
class IBuddyDocumentFinderPrivate
{
public:
    static QMap<QString, IBuddyDocumentFinder*>& finders()
    {
        static QMap<QString, IBuddyDocumentFinder*> finders;
        return finders;
    }
};

// ---------------- "Registry" interface --------------------------------------------
void IBuddyDocumentFinder::addFinder(const QString& mimeType, IBuddyDocumentFinder *finder)
{
    IBuddyDocumentFinderPrivate::finders()[mimeType] = finder;
}

void IBuddyDocumentFinder::removeFinder(const QString& mimeType)
{
    IBuddyDocumentFinderPrivate::finders().remove(mimeType);
}

IBuddyDocumentFinder* IBuddyDocumentFinder::finderForMimeType(const QString& mimeType)
{
    return IBuddyDocumentFinderPrivate::finders().value(mimeType, nullptr);
}

}
