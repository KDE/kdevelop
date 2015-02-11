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

#ifndef KDEVELOPSESSIONSENGINE_H
#define KDEVELOPSESSIONSENGINE_H

#include <Plasma/DataEngine>

struct Session
{
    QString hash;
    QString name;
    QString description;
};

class KDirWatch;

class KDevelopSessionsEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    KDevelopSessionsEngine(QObject *parent, const QVariantList &args);
    ~KDevelopSessionsEngine();

    void init();
    Plasma::Service *serviceForSource(const QString &source) override;

    private Q_SLOTS:
        void updateSessions();

private:
    QHash<QString, Session> m_currentSessions;

    KDirWatch *m_dirWatch;
};

#endif
