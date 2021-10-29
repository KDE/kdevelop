/*
    SPDX-FileCopyrightText: 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ILANGUAGECHECKPROVIDER_H
#define KDEVPLATFORM_ILANGUAGECHECKPROVIDER_H

#include "interfacesexport.h"
#include <QList>
#include <QObject>

namespace KDevelop
{
class ILanguageCheck;

class KDEVPLATFORMINTERFACES_EXPORT ILanguageCheckProvider
{
    public:
        virtual ~ILanguageCheckProvider();
        
        virtual QList<ILanguageCheck*> providedChecks()=0;
};

}

Q_DECLARE_INTERFACE( KDevelop::ILanguageCheckProvider, "org.kdevelop.ILanguageCheckProvider")

#endif

