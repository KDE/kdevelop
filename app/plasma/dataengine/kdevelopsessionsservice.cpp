/*****************************************************************************
*   Copyright (C) 2012 by Eike Hein <hein@kde.org>                           *
*   Copyright (C) 2011 by Shaun Reich <shaun.reich@kdemail.net>              *
*   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                    *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

#include "kdevelopsessionsservice.h"

#include <KToolInvocation>

KDevelopSessionsService::KDevelopSessionsService(QObject* parent, const QString& sessionName)
    : Plasma::Service(parent)
{
    setName("org.kde.plasma.dataengine.kdevelopsessions");
    setDestination(sessionName);
}

Plasma::ServiceJob* KDevelopSessionsService::createJob(const QString& operation, QMap<QString,QVariant>& parameters)
{
    return new SessionJob(this, operation, parameters);
}

SessionJob::SessionJob(KDevelopSessionsService *service, const QString &operation, const QMap<QString, QVariant> &parameters)
    : Plasma::ServiceJob(service->destination(), operation, parameters, service)
{
}

void SessionJob::start()
{
    if (operationName() == "open")
    {
        QStringList args;
        args << "--open-session" << destination();
        KToolInvocation::kdeinitExec("kdevelop", args);

        setResult(true);
    }
}

