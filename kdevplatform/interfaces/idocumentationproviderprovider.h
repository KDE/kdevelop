/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IDOCUMENTATIONPROVIDERPROVIDER_H
#define KDEVPLATFORM_IDOCUMENTATIONPROVIDERPROVIDER_H

#include <QObject>
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
    virtual void changedProvidersList() = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IDocumentationProviderProvider, "org.kdevelop.IDocumentationProviderProvider")

#endif

