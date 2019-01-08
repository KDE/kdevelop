/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#ifndef MESONIMPORTJOB_H
#define MESONIMPORTJOB_H

#include <KJob>
#include <QJsonObject>
#include <QFutureWatcher>
#include <interfaces/iproject.h>

class MesonManager;

class MesonImportJob : public KJob
{
    Q_OBJECT

public:
    MesonImportJob(MesonManager* manager, KDevelop::IProject* project, QObject* parent);

    void start() override;
    bool doKill() override;

private:
    void importFinished();

    KDevelop::IProject* m_project;
    QFutureWatcher<QJsonObject> m_futureWatcher;
    MesonManager* m_manager;
};

#endif // MESONIMPORTJOB_H
