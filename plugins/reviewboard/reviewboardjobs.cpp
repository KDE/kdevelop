/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "reviewboardjobs.h"
#include <qjson/parser.h>
#include <KLocalizedString>
#include <KIO/Job>

using namespace ReviewBoard;

HttpPostCall::HttpPostCall(const KUrl& s, const QString& apiPath, const QByteArray& post, QObject* parent)
    : KJob(parent)
{
    KUrl url=s;
    url.addPath(apiPath);
    requestJob = KIO::http_post(url, post);
    
    connect(requestJob, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(data(KIO::Job*, const QByteArray&)));
    connect(requestJob, SIGNAL(suspended(KJob*)), SIGNAL(suspended(KJob*)));
    connect(requestJob, SIGNAL(resumed(KJob*)), SIGNAL(resumed(KJob*)));
    connect(requestJob, SIGNAL(finished(KJob*)), SLOT(processData(KJob*)));
}

void HttpPostCall::data(KIO::Job*, const QByteArray& data)
{
    receivedData+=data;
}

void HttpPostCall::start()
{
    requestJob->start();
}

QVariant HttpPostCall::result() const
{
    return m_result;
}

void HttpPostCall::processData(KJob* )
{
    QJson::Parser parser;

    bool ok;
    m_result = parser.parse(receivedData, &ok);
    if(!ok) {
        setError(1);
        setErrorText(i18n("JSON error: %1: %2", parser.errorLine(), parser.errorString()));
    } else
        emitResult();
}

NewRequest::NewRequest(const KUrl& server, QObject* parent)
    : KJob(parent), m_server(server)
{
    m_repositories = new HttpPostCall(server, "/api/json/repositories/", "", this);
}

void NewRequest::second()
{
    if(m_repositories->error()) {
        qDebug() << "Could not check the repository" << m_repositories->errorString();
        setError(1);
        setErrorText(i18n("Could not find the repositories"));
    }
    QString repo = m_repositories->result().toMap()["repositories"].toList().first().toMap()["path"].toString();
    
    m_newreq = new HttpPostCall(m_server, "/api/json/reviewrequests/new/", "submit_as="+m_server.userName().toLatin1()+"&repository_path="+repo.toLatin1(), this);
    m_newreq->start();
}

void NewRequest::third()
{
    if(m_newreq->error()) {
        qDebug() << "Could not create the new request" << m_newreq->errorString();
        setError(1);
        setErrorText(i18n("Could not create the new request")); 
    }
    QString id = m_newreq->result().toMap()["id"].toString();
    
    m_newreq = new HttpPostCall(m_server, "/api/json/reviewrequests/"+id+"/diff/new", s, this);
    m_newreq->start();
}
