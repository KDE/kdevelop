/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <QUrl>
#include <kio/scheduler.h>
#include <kio/transferjob.h>
#include <kio/storedtransferjob.h>
#include <QJsonDocument>
#include <QHostInfo>
#include <QDateTime>

#include <debug.h>
#include <ghresource.h>
#include <ghprovidermodel.h>


namespace gh
{
/// Base url for the Github API v3.
const static QUrl baseUrl(QStringLiteral("https://api.github.com"));

KIO::StoredTransferJob* createHttpAuthJob(const QString &httpHeader)
{
    QUrl url = baseUrl;
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1String("/authorizations"));

    // generate a unique token, see bug 372144
    const QString tokenName = "KDevelop Github Provider : "
        + QHostInfo::localHostName() + " - "
        + QDateTime::currentDateTimeUtc().toString();
    const QByteArray data = QByteArrayLiteral("{ \"scopes\": [\"repo\"], \"note\": \"") + tokenName.toUtf8() + QByteArrayLiteral("\" }");

    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
    job->setProperty("requestedTokenName", tokenName);
    job->addMetaData(QStringLiteral("customHTTPHeader"), httpHeader);

    return job;
}

Resource::Resource(QObject *parent, ProviderModel *model)
    : QObject(parent), m_model(model)
{
    /* There's nothing to do here */
}

void Resource::searchRepos(const QString &uri, const QString &token)
{
    KIO::TransferJob *job = getTransferJob(uri, token);
    connect(job, &KIO::TransferJob::data,
            this, &Resource::slotRepos);
}

void Resource::getOrgs(const QString &token)
{
    KIO::TransferJob *job = getTransferJob(QStringLiteral("/user/orgs"), token);
    connect(job, &KIO::TransferJob::data,
            this, &Resource::slotOrgs);
}

void Resource::authenticate(const QString &name, const QString &password)
{
    auto job = createHttpAuthJob(QLatin1String("Authorization: Basic ") + QString::fromUtf8(QByteArray(name.toUtf8() + ':' + password.toUtf8()).toBase64()));
    job->addMetaData("PropagateHttpHeader","true");
    connect(job, &KIO::StoredTransferJob::result,
            this, &Resource::slotAuthenticate);
    job->start();
}

void Resource::twoFactorAuthenticate(const QString &transferHeader, const QString &code)
{
    auto job = createHttpAuthJob(transferHeader + QLatin1String("\nX-GitHub-OTP: ") + code);
    connect(job, &KIO::StoredTransferJob::result,
            this, &Resource::slotAuthenticate);
    job->start();
}

void Resource::revokeAccess(const QString &id, const QString &name, const QString &password)
{
    QUrl url = baseUrl;
    url.setPath(url.path() + "/authorizations/" + id);
    KIO::TransferJob *job = KIO::http_delete(url, KIO::HideProgressInfo);
    job->addMetaData(QStringLiteral("customHTTPHeader"), QLatin1String("Authorization: Basic ") + QString (name + ':' + password).toUtf8().toBase64());
    /* And we don't care if it's successful ;) */
    job->start();
}

KIO::TransferJob * Resource::getTransferJob(const QString &path, const QString &token) const
{
    QUrl url = baseUrl;
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + path);
    KIO::TransferJob *job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    if (!token.isEmpty())
        job->addMetaData(QStringLiteral("customHTTPHeader"), "Authorization: token " + token);
    return job;
}

void Resource::retrieveRepos(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error == 0) {
        QVariantList map = doc.toVariant().toList();
        m_model->clear();
        foreach (const QVariant &it, map) {
            const QVariantMap &map = it.toMap();
            Response res;
            res.name = map.value(QStringLiteral("name")).toString();
            res.url = map.value(QStringLiteral("clone_url")).toUrl();
            if (map.value(QStringLiteral("fork")).toBool())
                res.kind = Fork;
            else if (map.value(QStringLiteral("private")).toBool())
                res.kind = Private;
            else
                res.kind = Public;
            ProviderItem *item = new ProviderItem(res);
            m_model->appendRow(item);
        }
    }
    emit reposUpdated();
}

void Resource::retrieveOrgs(const QByteArray &data)
{
    QStringList res;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error == 0) {
        QVariantList json = doc.toVariant().toList();
        foreach (QVariant it, json) {
            QVariantMap map = it.toMap();
            res << map.value(QStringLiteral("login")).toString();
        }
    }
    emit orgsUpdated(res);
}

void Resource::slotAuthenticate(KJob *job)
{
    const QString tokenName = job->property("requestedTokenName").toString();
    Q_ASSERT(!tokenName.isEmpty());

    if (job->error()) {
        emit authenticated("", "", tokenName);
        return;
    }

    const auto metaData = qobject_cast<KIO::StoredTransferJob*>(job)->metaData();
    if (metaData[QStringLiteral("responsecode")] == QStringLiteral("401")) {
        const auto& header = metaData[QStringLiteral("HTTP-Headers")];
        if (header.contains(QStringLiteral("X-GitHub-OTP: required;"), Qt::CaseInsensitive)) {
          emit twoFactorAuthRequested(qobject_cast<KIO::StoredTransferJob*>(job)->outgoingMetaData()[QStringLiteral("customHTTPHeader")]);
          return;
        }
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(qobject_cast<KIO::StoredTransferJob *>(job)->data(), &error);

    qCDebug(GHPROVIDER) << "Response:" << doc;

    if (error.error == 0) {
        QVariantMap map = doc.toVariant().toMap();
        emit authenticated(map.value(QStringLiteral("id")).toByteArray(),
                           map.value(QStringLiteral("token")).toByteArray(), tokenName);
    } else
        emit authenticated("", "", tokenName);
}

void Resource::slotRepos(KIO::Job *job, const QByteArray &data)
{
    if (!job) {
        qCWarning(GHPROVIDER) << "NULL job returned!";
        return;
    }
    if (job->error()) {
        qCWarning(GHPROVIDER) << "Job error: " << job->errorString();
        return;
    }

    m_temp.append(data);
    if (data.isEmpty()) {
        retrieveRepos(m_temp);
        m_temp = "";
    }
}

void Resource::slotOrgs(KIO::Job *job, const QByteArray &data)
{
    QList<QString> res;

    if (!job) {
        qCWarning(GHPROVIDER) << "NULL job returned!";
        emit orgsUpdated(res);
        return;
    }
    if (job->error()) {
        qCWarning(GHPROVIDER) << "Job error: " << job->errorString();
        emit orgsUpdated(res);
        return;
    }

    m_orgTemp.append(data);
    if (data.isEmpty()) {
        retrieveOrgs(m_orgTemp);
        m_orgTemp = "";
    }
}

} // End of namespace gh
