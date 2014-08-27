/*  This file is part of KDevelop
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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

#ifndef KDEVPLATFORM_IDOCUMENTATIONPROVIDERPROVIDER_H
#define KDEVPLATFORM_IDOCUMENTATIONPROVIDERPROVIDER_H

#include <QExplicitlySharedDataPointer>
#include <QtCore/QObject>
#include "interfacesexport.h"

namespace KDevelop
{
class IDocumentationProvider;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentationProviderProvider
{
    public:
        virtual ~IDocumentationProviderProvider();

        /** @returns a list of providers provided by this class. */
        virtual QList<IDocumentationProvider*> providers() = 0;

Q_SIGNALS:
    virtual void changedProvidersList() const=0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IDocumentationProviderProvider, "org.kdevelop.IDocumentationProviderProvider")

#endif

