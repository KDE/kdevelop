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

#include "debug.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <KLocalizedString>
#include <KIO/Job>
#include <KRandom>
#include <QMimeType>
#include <QMimeDatabase>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

using namespace ReviewBoard;

QByteArray ReviewBoard::urlToData(const QUrl& url)
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
    QByteArray form_data;
    foreach(const auto& val, values)
    {
        QByteArray hstr("--");
        hstr += m_boundary;
        hstr += "\r\n";
        hstr += "Content-Disposition: form-data; name=\"";
        hstr += val.first.toLatin1();
        hstr += "\"";

        //File
        if (val.second.type()==QVariant::Url) {
            QUrl path=val.second.toUrl();
            hstr += "; filename=\"" + path.fileName().toLatin1() + "\"";
            const QMimeType mime = QMimeDatabase().mimeTypeForUrl(path);
            if (!mime.name().isEmpty()) {
                hstr += "\r\nContent-Type: ";
                hstr += mime.name().toLatin1().constData();
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

QByteArray multipartFormData(const QVariantMap& values)
{
    QList<QPair<QString, QVariant> > vals;
    for(QVariantMap::const_iterator it = values.constBegin(), itEnd = values.constEnd(); it!=itEnd; ++it) {
        vals += qMakePair<QString, QVariant>(it.key(), it.value());
    }
    return multipartFormData(vals);
}

}

HttpCall::HttpCall(const QUrl& s, const QString& apiPath, const QList<QPair<QString,QString> >& queryParameters, Method method, const QByteArray& post, bool multipart, QObject* parent)
    : KJob(parent)
    , m_reply(nullptr)
    , m_post(post)
    , m_multipart(multipart)
    , m_method(method)
{
    m_requrl=s;
    m_requrl.setPath(m_requrl.path() + '/' + apiPath);
    QUrlQuery query;
    for(QList<QPair<QString,QString> >::const_iterator i = queryParameters.begin(); i < queryParameters.end(); i++) {
        query.addQueryItem(i->first, i->second);
    }
    m_requrl.setQuery(query);
}

void HttpCall::start()
{
    QNetworkRequest r(m_requrl);

    if(!m_requrl.userName().isEmpty()) {
        QByteArray head = "Basic " + m_requrl.userInfo().toLatin1().toBase64();
        r.setRawHeader("Authorization", head);
    }

    if(m_multipart) {
        r.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
        r.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(m_post.size()));
        r.setRawHeader( "Content-Type", "multipart/form-data; boundary=" + m_boundary );
    }

    switch(m_method) {
        case Get:
            m_reply=m_manager.get(r);
            break;
        case Post:
            m_reply=m_manager.post(r, m_post);
            break;
        case Put:
            m_reply=m_manager.put(r, m_post);
            break;
    }
    connect(m_reply, &QNetworkReply::finished, this, &HttpCall::onFinished);

//     qCDebug(PLUGIN_REVIEWBOARD) << "starting... requrl=" << m_requrl << "post=" << m_post;
}

QVariant HttpCall::result() const
{
    Q_ASSERT(m_reply->isFinished());
    return m_result;
}

void HttpCall::onFinished()
{
    if (m_reply->error()) {
        setError(55);
        setErrorText(i18n("Error %1 while accessing %2", m_reply->error(), m_reply->request().url().toDisplayString()));
        emitResult();
        return;
    }
    QByteArray receivedData = m_reply->readAll();
    QJsonParseError error;
    QJsonDocument parser = QJsonDocument::fromJson(receivedData, &error);
    const QVariant output = parser.toVariant();

    if (error.error == 0) {
        m_result = output;
    } else {
        setError(1);
        setErrorText(i18n("JSON error: %1", error.errorString()));
    }

    if (output.toMap().value("stat").toString()!="ok") {
        setError(2);
        setErrorText(i18n("Request Error: %1", output.toMap().value("err").toMap().value("msg").toString()));
    }

    qCDebug(PLUGIN_REVIEWBOARD) << "parsing..." << receivedData;
    emitResult();
}

NewRequest::NewRequest(const QUrl& server, const QString& projectPath, QObject* parent)
    : ReviewRequest(server, 0, parent), m_project(projectPath)
{
    m_newreq = new HttpCall(this->server(), "/api/review-requests/", {}, HttpCall::Post, "repository="+projectPath.toLatin1(), false, this);
    connect(m_newreq, &HttpCall::finished, this, &NewRequest::done);
}

void NewRequest::start()
{
    m_newreq->start();
}

void NewRequest::done()
{
    if (m_newreq->error()) {
        qCDebug(PLUGIN_REVIEWBOARD) << "Could not create the new request" << m_newreq->errorString();
        setError(2);
        setErrorText(i18n("Could not create the new request:\n%1", m_newreq->errorString()));
    } else {
        QVariant res = m_newreq->result();
        setRequestId(res.toMap()["review_request"].toMap()["id"].toString());
        Q_ASSERT(!requestId().isEmpty());
    }

    emitResult();
}


SubmitPatchRequest::SubmitPatchRequest(const QUrl& server, const QUrl& patch, const QString& basedir, const QString& id, QObject* parent)
    : ReviewRequest(server, id, parent), m_patch(patch), m_basedir(basedir)
{
    QList<QPair<QString, QVariant> > vals;
    vals += QPair<QString, QVariant>("basedir", m_basedir);
    vals += QPair<QString, QVariant>("path", qVariantFromValue<QUrl>(m_patch));

    m_uploadpatch = new HttpCall(this->server(), "/api/review-requests/"+requestId()+"/diffs/", {}, HttpCall::Post, multipartFormData(vals), true, this);
    connect(m_uploadpatch, &HttpCall::finished, this, &SubmitPatchRequest::done);
}

void SubmitPatchRequest::start()
{
    m_uploadpatch->start();
}

void SubmitPatchRequest::done()
{
    if (m_uploadpatch->error()) {
        qCDebug(PLUGIN_REVIEWBOARD) << "Could not upload the patch" << m_uploadpatch->errorString();
        setError(3);
        setErrorText(i18n("Could not upload the patch"));
    }

    emitResult();
}

ProjectsListRequest::ProjectsListRequest(const QUrl& server, QObject* parent)
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
    repositoriesParameters << qMakePair<QString,QString>(QStringLiteral("max-results"), QStringLiteral("200"));
    repositoriesParameters << qMakePair<QString,QString>(QStringLiteral("start"), QString::number(startIndex));

    HttpCall* repositoriesCall = new HttpCall(m_server, "/api/repositories/", repositoriesParameters, HttpCall::Get, "", false, this);
    connect(repositoriesCall, &HttpCall::finished, this, &ProjectsListRequest::done);

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

ReviewListRequest::ReviewListRequest(const QUrl& server, const QString& user, const QString& reviewStatus, QObject* parent)
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
    reviewParameters << qMakePair<QString,QString>(QStringLiteral("max-results"), QStringLiteral("200"));
    reviewParameters << qMakePair<QString,QString>(QStringLiteral("start"), QString::number(startIndex));
    reviewParameters << qMakePair<QString,QString>(QStringLiteral("from-user"), m_user);
    reviewParameters << qMakePair<QString,QString>(QStringLiteral("status"), m_reviewStatus);

    HttpCall* reviewsCall = new HttpCall(m_server, "/api/review-requests/", reviewParameters, HttpCall::Get, "", false, this);
    connect(reviewsCall, &HttpCall::finished, this, &ReviewListRequest::done);

    reviewsCall->start();
}

void ReviewListRequest::done(KJob* job)
{
    // TODO error
    // TODO max iterations
    if (job->error()) {
        qCDebug(PLUGIN_REVIEWBOARD) << "Could not get reviews list" << job->errorString();
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

UpdateRequest::UpdateRequest(const QUrl& server, const QString& id, const QVariantMap& newValues, QObject* parent)
    : ReviewRequest(server, id, parent)
{
    m_req = new HttpCall(this->server(), "/api/review-requests/"+id+"/draft/", {}, HttpCall::Put, multipartFormData(newValues), true, this);
    connect(m_req, &HttpCall::finished, this, &UpdateRequest::done);
}

void UpdateRequest::start()
{
    m_req->start();
}

void UpdateRequest::done()
{
    if (m_req->error()) {
        qCWarning(PLUGIN_REVIEWBOARD) << "Could not set all metadata to the review" << m_req->errorString() << m_req->property("result");
        setError(3);
        setErrorText(i18n("Could not set metadata"));
    }

    emitResult();
}

#include "reviewboardjobs.moc"
