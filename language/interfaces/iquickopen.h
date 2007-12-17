/* This file is part of the KDE project
   Copyright 2006 David Nolden <david.nolden.kde@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef IQUICKOPEN_H
#define IQUICKOPEN_H

#include <KUrl>
#include <QSet>
#include <iextension.h>
#include "../languageexport.h"

class QStringList;

namespace KDevelop
{

class QuickOpenDataProviderBase;
class HashedString;

/**
 * Interface to quickopen
*/
class KDEVPLATFORMLANGUAGE_EXPORT IQuickOpen
{
public:
    virtual ~IQuickOpen();

    /**
     * Registers a new provider under a specified name.
     * There may be multiple providers with the same type/scope, they will be used simultaneously in that case.
     * type and scope will be shown in the GUI, so they should be translated.
     * @param scope Different scopes supported by this data-provider, Examples: "Project", "Imports", etc.
     * @param type Types of the provided data, Examples: "Files", "Functiosn", "Classes", etc.
     * @param provider The provider. It does not need to be explicitly removed before its destruction.
     * */
    virtual void registerProvider( const QStringList& scopes, const QStringList& type, QuickOpenDataProviderBase* provider ) = 0;

    /**
     * Remove provider.
     * @param provider The provider to remove
     * @return Whether a provider was removed. If false, the provider was not attached.
     * */
    virtual bool removeProvider( QuickOpenDataProviderBase* provider ) = 0;

    /**
     * Queries a set of files merged from all active data-providers that implement QuickOpenFileSetInterface.
     * This should not be queried by data-providers that implement QuickOpenFileSetInterface during their initialization(set() and enableData())
    * */
    virtual QSet<KDevelop::HashedString> fileSet() const = 0;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS(KDevelop, IQuickOpen, "org.kdevelop.IQuickOpen")
Q_DECLARE_INTERFACE(KDevelop::IQuickOpen, "org.kdevelop.IQuickOpen")

#endif // IQuickOpen_H
