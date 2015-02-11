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

#ifndef KDEVELOPSESSIONSSERVICE_H
#define KDEVELOPSESSIONSSERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>

class KDevelopSessionsService : public Plasma::Service
{
    Q_OBJECT

public:
    KDevelopSessionsService(QObject* parent, const QString& sessionName);

protected:
    Plasma::ServiceJob* createJob(const QString& operation, QMap<QString,QVariant>& parameters) override;
};

class SessionJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    SessionJob(KDevelopSessionsService *service, const QString& operation, const QMap<QString, QVariant> &parameters);
    void start() override;
};

#endif
