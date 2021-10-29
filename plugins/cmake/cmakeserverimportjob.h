/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKESERVERIMPORTJOB_H
#define CMAKESERVERIMPORTJOB_H

#include <QPointer>
#include <KJob>
#include "cmakeprojectdata.h"

namespace KDevelop
{
class IProject;
}

class CMakeServerImportJob : public KJob
{
    Q_OBJECT
public:
    CMakeServerImportJob(KDevelop::IProject* project, const QSharedPointer<CMakeServer> &server, QObject* parent);

    enum Error { NoError, UnexpectedDisconnect, ErrorResponse };

    void start() override;

    KDevelop::IProject* project() const { return m_project; }

    CMakeProjectData projectData() const { return m_data; }

    static void processCodeModel(const QJsonObject &response, CMakeProjectData &data);

private:
    void doStart();
    void processResponse(const QJsonObject &response);

    QSharedPointer<CMakeServer> m_server;
    KDevelop::IProject* m_project;

    CMakeProjectData m_data;
};

#endif // CMAKESERVERIMPORTJOB_H
