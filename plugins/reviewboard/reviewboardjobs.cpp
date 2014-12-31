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
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
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
        Q_UNUSED(corr);

        ret = f.readAll();

    } else {
//TODO: add downloading the data
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
                hstr += ptr->name().toLatin1().constData();
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

HttpCall::HttpCall(const KUrl& s, const QString& apiPath, const QList<QPair<QString,QString> >& queryParameters, const QByteArray& post, bool multipart, QObject* parent)
    : KJob(parent), m_post(post), m_multipart(multipart)
{
    m_requrl=s;
    m_requrl.addPath(apiPath);

    for(QList<QPair<QString,QString> >::const_iterator i = queryParameters.begin(); i < queryParameters.end(); i++) {
        m_requrl.addQueryItem(i->first, i->second);
    }
}

void HttpCall::start()
{
    QNetworkRequest r(m_requrl);

    if(m_requrl.hasUser()) {
        QByteArray head = "Basic " + m_requrl.userInfo().toLatin1().toBase64();
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

    qDebug() << "starting... requrl=" << m_requrl << "post=" << m_post;
}

QVariant HttpCall::result() const
{
    Q_ASSERT(m_reply->isFinished());
    return m_result;
}

void HttpCall::finished()
{
    QJson::Parser parser;
    QByteArray receivedData = m_reply->readAll();

//     qDebug() << "parsing..." << receivedData;
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

NewRequest::NewRequest(const KUrl& server, const QString& projectPath, QObject* parent)
    : KJob(parent), m_server(server), m_project(projectPath)
{
    m_newreq = new HttpCall(m_server, "/api/review-requests/", QList<QPair<QString,QString> >(), "repository="+projectPath.toLatin1(), false, this);
    connect(m_newreq, SIGNAL(finished(KJob*)), SLOT(done()));
}

void NewRequest::start()
{
    m_newreq->start();
}

QString NewRequest::requestId() const
{
    return m_id;
}

void NewRequest::done()
{
    if (m_newreq->error()) {
        qDebug() << "Could not create the new request" << m_newreq->errorString();
        setError(2);
        setErrorText(i18n("Could not create the new request:\n%1", m_newreq->errorString()));
    } else {
        QVariant res = m_newreq->result();

        m_id = res.toMap()["review_request"].toMap()["id"].toString();
        Q_ASSERT(!m_id.isEmpty());
    }

    emitResult();
}


SubmitPatchRequest::SubmitPatchRequest(const KUrl& server, const KUrl& patch, const QString& basedir, const QString& id, QObject* parent)
    : KJob(parent), m_server(server), m_patch(patch), m_basedir(basedir), m_id(id)
{
    QList<QPair<QString, QVariant> > vals;
    vals += QPair<QString, QVariant>("basedir", m_basedir);
    vals += QPair<QString, QVariant>("path", qVariantFromValue<QUrl>(m_patch));

    m_uploadpatch = new HttpCall(m_server, "/api/review-requests/"+m_id+"/diffs/", QList<QPair<QString,QString> >(), multipartFormData(vals), true, this);
    connect(m_uploadpatch, SIGNAL(finished(KJob*)), SLOT(done()));
}

void SubmitPatchRequest::start()
{
    m_uploadpatch->start();
}

QString SubmitPatchRequest::requestId() const
{
    return m_id;
}

void SubmitPatchRequest::done()
{
    if (m_uploadpatch->error()) {
        qDebug() << "Could not upload the patch" << m_uploadpatch->errorString();
        setError(3);
        setErrorText(i18n("Could not upload the patch"));
    }

    emitResult();
}

ProjectsListRequest::ProjectsListRequest(const KUrl& server, QObject* parent)
    : KJob(parent), m_server(server)
{
}

void ProjectsListRequest::start()
{
    requestRepositoryList(0);
}

QVariantList ProjectsListRequest::repositories() const
{
    return m_repositories;
}

void ProjectsListRequest::requestRepositoryList(int startIndex)
{
    QList<QPair<QString,QString> > repositoriesParameters;

    // In practice, the web API will return at most 200 repos per call, so just hardcode that value here
    repositoriesParameters << qMakePair<QString,QString>("max-results", QLatin1String("200"));
    repositoriesParameters << qMakePair<QString,QString>("start", QString("%1").arg(startIndex));

    HttpCall* repositoriesCall = new HttpCall(m_server, "/api/repositories/", repositoriesParameters, "", false, this);
    connect(repositoriesCall, SIGNAL(finished(KJob*)), SLOT(done(KJob*)));

    repositoriesCall->start();
}

void ProjectsListRequest::done(KJob* job)
{
    // TODO error
    // TODO max iterations
    HttpCall* repositoriesCall = qobject_cast<HttpCall*>(job);
    QMap<QString, QVariant> resultMap = repositoriesCall->result().toMap();
    const int totalResults = repositoriesCall->result().toMap()["total_results"].toInt();
    m_repositories << repositoriesCall->result().toMap()["repositories"].toList();

    if (m_repositories.count() < totalResults) {
        requestRepositoryList(m_repositories.count());
    } else {
        emitResult();
    }
}

ReviewListRequest::ReviewListRequest(const KUrl& server, const QString& user, const QString& reviewStatus, QObject* parent)
    : KJob(parent), m_server(server), m_user(user), m_reviewStatus(reviewStatus)
{
}

void ReviewListRequest::start()
{
    requestReviewList(0);
}

QVariantList ReviewListRequest::reviews() const
{
    return m_reviews;
}

void ReviewListRequest::requestReviewList(int startIndex)
{
    QList<QPair<QString,QString> > reviewParameters;

    // In practice, the web API will return at most 200 repos per call, so just hardcode that value here
    reviewParameters << qMakePair<QString,QString>("max-results", QLatin1String("200"));
    reviewParameters << qMakePair<QString,QString>("start", QString("%1").arg(startIndex));
    reviewParameters << qMakePair<QString,QString>("from-user", m_user);
    reviewParameters << qMakePair<QString,QString>("status", m_reviewStatus);

    HttpCall* reviewsCall = new HttpCall(m_server, "/api/review-requests/", reviewParameters, "", false, this);
    connect(reviewsCall, SIGNAL(finished(KJob*)), SLOT(done(KJob*)));

    reviewsCall->start();
}

void ReviewListRequest::done(KJob* job)
{
    // TODO error
    // TODO max iterations
    if (job->error()) {
        qDebug() << "Could not get reviews list" << job->errorString();
        setError(3);
        setErrorText(i18n("Could not get reviews list"));
        emitResult();
    }

    HttpCall* reviewsCall = qobject_cast<HttpCall*>(job);
    QMap<QString, QVariant> resultMap = reviewsCall->result().toMap();
    const int totalResults = resultMap["total_results"].toInt();

    m_reviews << resultMap["review_requests"].toList();

    if (m_reviews.count() < totalResults) {
        requestReviewList(m_reviews.count());
    } else {
        emitResult();
    }
}
