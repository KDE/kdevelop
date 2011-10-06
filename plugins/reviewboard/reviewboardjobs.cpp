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
#include <KRandom>
#include <KMimeType>
#include <QFile>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace ReviewBoard;



QByteArray ReviewBoard::urlToData(const KUrl& url)
{
    QByteArray ret;
    if (url.isLocalFile()) {
        QFile f(url.toLocalFile());
        Q_ASSERT(f.exists());
        bool corr=f.open(QFile::ReadOnly | QFile::Text);
        Q_ASSERT(corr);

        ret = f.readAll();

    } else {
#if defined(__GNUC__)
#warning TODO: add downloading the data
#endif
    }
    return ret;
}
namespace
{
static const QByteArray m_boundary = "----------" + KRandom::randomString( 42 + 13 ).toLatin1();

QByteArray multipartFormData(const QList<QPair<QString, QVariant> >& values)
{
    typedef QPair<QString, QVariant> StrVar;
    QByteArray form_data;
    foreach(const StrVar& val, values)
    {
        QByteArray hstr("--");
        hstr += m_boundary;
        hstr += "\r\n";
        hstr += "Content-Disposition: form-data; name=\"";
        hstr += val.first.toLatin1();
        hstr += "\"";

        //File
        if (val.second.type()==QVariant::Url) {
            KUrl path=val.second.toUrl();
            hstr += "; filename=\"" + path.fileName().toLatin1() + "\"";
            const KMimeType::Ptr ptr = KMimeType::findByUrl(path);
            if (!ptr->name().isEmpty()) {
                hstr += "\r\nContent-Type: ";
                hstr += ptr->name().toAscii().constData();
            }
        }
        //

        hstr += "\r\n\r\n";

        // append body
        form_data.append(hstr);
        if (val.second.type()==QVariant::Url)
            form_data += urlToData(val.second.toUrl());
        else
            form_data += val.second.toByteArray();
        form_data.append("\r\n");
        //EOFILE
    }

    form_data += QByteArray("--" + m_boundary + "--\r\n");

    return form_data;
}

}

HttpPostCall::HttpPostCall(const KUrl& s, const QString& apiPath, const QList<QPair<QString,QString>>& queryParameters, const QByteArray& post, bool multipart, QObject* parent)
        : KJob(parent), m_post(post), m_multipart(multipart)
{
    m_requrl=s;
    m_requrl.addPath(apiPath);

    for(QList<QPair<QString,QString>>::const_iterator i = queryParameters.begin(); i < queryParameters.end(); i++) {
        m_requrl.addQueryItem(i->first, i->second);
    }
}

void HttpPostCall::start()
{
    QNetworkRequest r(m_requrl);

    if(m_requrl.hasUser()) {
        QByteArray head = "Basic " + m_requrl.userInfo().toAscii().toBase64();
        r.setRawHeader("Authorization", head);
    }
    
    if(m_multipart) {
        r.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
        r.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(m_post.size()));
        r.setRawHeader( "Content-Type", "multipart/form-data; boundary=" + m_boundary );
    }

    if(m_post.isEmpty())
        m_reply=m_manager.get(r);
    else
        m_reply=m_manager.post(r, m_post);

    connect(m_reply, SIGNAL(finished()), SLOT(finished()));

    qDebug() << "starting..." << m_requrl << m_post;
}

QVariant HttpPostCall::result() const
{
    Q_ASSERT(m_reply->isFinished());
    return m_result;
}

void HttpPostCall::finished()
{
    QJson::Parser parser;
    QByteArray receivedData = m_reply->readAll();

    qDebug() << "parsing..." << receivedData;
    bool ok;
    m_result = parser.parse(receivedData, &ok);
    if (!ok) {
        setError(1);
        setErrorText(i18n("JSON error: %1: %2", parser.errorLine(), parser.errorString()));
    }

    if (m_result.toMap().value("stat").toString()!="ok") {
        setError(2);
        setErrorText(i18n("Request Error: %1", m_result.toMap().value("err").toMap().value("msg").toString()));
    }

    emitResult();
}

NewRequest::NewRequest(const KUrl& server, const KUrl& patch, const QString& projectPath, const QString& basedir, QObject* parent)
        : KJob(parent), m_server(server), m_patch(patch), m_basedir(basedir), m_project(projectPath)
{
    m_newreq = new HttpPostCall(m_server, "/api/review-requests/", QList<QPair<QString,QString>>(), "repository="+projectPath.toLatin1(), false, this);
    connect(m_newreq, SIGNAL(finished(KJob*)), SLOT(submitPatch()));
}

void NewRequest::start()
{
    m_newreq->start();
}

void NewRequest::submitPatch()
{
    if (m_newreq->error()) {
        qDebug() << "Could not create the new request" << m_newreq->errorString();
        setError(2);
        setErrorText(i18n("Could not create the new request:\n%1", m_newreq->errorString()));
        emitResult();
        return;
    }
    QVariant res = m_newreq->result();

    m_id = res.toMap()["review_request"].toMap()["id"].toString();
    Q_ASSERT(!m_id.isEmpty());

    QList<QPair<QString, QVariant> > vals;
    vals += QPair<QString, QVariant>("basedir", m_basedir);
    vals += QPair<QString, QVariant>("path", qVariantFromValue<QUrl>(m_patch));

    m_uploadpatch = new HttpPostCall(m_server, "/api/review-requests/"+m_id+"/diffs/", QList<QPair<QString,QString>>(), multipartFormData(vals), true, this);
    connect(m_uploadpatch, SIGNAL(finished(KJob*)), SLOT(done()));
    m_uploadpatch->start();
}

QString NewRequest::requestId() const
{
    return m_id;
}

void NewRequest::done()
{
    if (m_uploadpatch->error()) {
        qDebug() << "Could not upload the patch" << m_uploadpatch->errorString();
        setError(3);
        setErrorText(i18n("Could not upload the patch"));
    }

    emitResult();
}

ProjectsListRequest::ProjectsListRequest(const KUrl& server, QObject* parent)
    : m_server(server), KJob(parent)
{
    QList<QPair<QString,QString>> countParameters;

    countParameters << qMakePair<QString,QString>("counts-only", "1");

    m_countRequest = new HttpPostCall(m_server, "/api/repositories/", countParameters, "", false, this);
    connect(m_countRequest, SIGNAL(finished(KJob*)), SLOT(gotRepositoryCount(KJob*)));
}

void ProjectsListRequest::start()
{
    m_countRequest->start();
}

QVariantList ProjectsListRequest::repositories() const
{
    return m_repositoriesRequest->result().toMap()["repositories"].toList();
}

void ProjectsListRequest::gotRepositoryCount(KJob* )
{
    int numberOfRepositories = m_countRequest->result().toMap()["count"].toInt();
    QList<QPair<QString,QString>> repositoriesParameteres;

    repositoriesParameteres << qMakePair<QString,QString>("max-results", QString("%1").arg(numberOfRepositories));

    m_repositoriesRequest = new HttpPostCall(m_server, "/api/repositories/", repositoriesParameteres, "", false, this);
    connect(m_repositoriesRequest, SIGNAL(finished(KJob*)), SLOT(done(KJob*)));

    m_repositoriesRequest->start();
}

void ProjectsListRequest::done(KJob* job)
{
    emitResult();
}
