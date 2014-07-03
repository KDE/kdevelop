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


#include <KDebug>
#include <kio/scheduler.h>
#include <qjson/parser.h>

#include <ghresource.h>
#include <ghprovidermodel.h>


namespace gh
{
/// Base url for the Github API v3.
const static KUrl baseUrl("https://api.github.com");

Resource::Resource(QObject *parent, ProviderModel *model)
    : QObject(parent), m_model(model)
{
    /* There's nothing to do here */
}

void Resource::searchRepos(const QString &uri, const QString &token)
{
    KIO::TransferJob *job = getTransferJob(uri, token);
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotRepos(KIO::Job *, const QByteArray &)));
}

void Resource::getOrgs(const QString &token)
{
    KIO::TransferJob *job = getTransferJob("/user/orgs", token);
    connect(job, SIGNAL(data(KIO::Job *, QByteArray)),
            this, SLOT(slotOrgs(KIO::Job *, const QByteArray &)));
}

void Resource::authenticate(const QString &name, const QString &password)
{
    KUrl url = baseUrl;
    url.addPath("/authorizations");
    QByteArray data = "{ \"scopes\": [\"repo\"], \"note\": \"KDevelop Github Provider\" }";
    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
    job->addMetaData("customHTTPHeader", "Authorization: Basic " + QString (name + ':' + password).toUtf8().toBase64());
    connect(job, SIGNAL(result(KJob *)), SLOT(slotAuthenticate(KJob *)));
    job->start();
}

void Resource::revokeAccess(const QString &id, const QString &name, const QString &password)
{
    KUrl url = baseUrl;
    url.addPath("/authorizations/" + id);
    KIO::TransferJob *job = KIO::http_delete(url, KIO::HideProgressInfo);
    job->addMetaData("customHTTPHeader", "Authorization: Basic " + QString (name + ':' + password).toUtf8().toBase64());
    /* And we don't care if it's successful ;) */
    job->start();
}

KIO::TransferJob * Resource::getTransferJob(const QString &uri, const QString &token) const
{
    KUrl url = baseUrl;
    url.addPath(uri);
    KIO::TransferJob *job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    if (!token.isEmpty())
        job->addMetaData("customHTTPHeader", "Authorization: token " + token);
    return job;
}

void Resource::retrieveRepos(const QByteArray &data)
{
    bool ok;
    QJson::Parser parser;
    QVariantList map = parser.parse(data, &ok).toList();

    if (ok) {
        m_model->clear();
        foreach (const QVariant &it, map) {
            const QVariantMap &map = it.toMap();
            Response res;
            res.name = map.value("name").toString();
            res.url = map.value("clone_url").toUrl();
            if (map.value("fork").toBool())
                res.kind = Fork;
            else if (map.value("private").toBool())
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
    bool ok;
    QStringList res;
    QJson::Parser parser;
    QVariantList json = parser.parse(data, &ok).toList();

    if (ok) {
        foreach (const QVariant &it, json) {
            const QVariantMap &map = it.toMap();
            res << map.value("login").toString();
        }
    }
    emit orgsUpdated(res);
}

void Resource::slotAuthenticate(KJob *job)
{
    if (job->error()) {
        emit authenticated("", "");
        return;
    }

    bool ok;
    QJson::Parser p;
    const QVariant &res = p.parse(qobject_cast<KIO::StoredTransferJob *>(job)->data(), &ok);
    if (ok) {
        const QVariantMap &map = res.toMap();
        emit authenticated(map.value("id").toByteArray(),
                           map.value("token").toByteArray());
    } else
        emit authenticated("", "");
}

void Resource::slotRepos(KIO::Job *job, const QByteArray &data)
{
    if (!job) {
        kWarning() << "NULL job returned!";
        return;
    }
    if (job->error()) {
        kWarning() << "Job error: " << job->errorString();
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
        kWarning() << "NULL job returned!";
        emit orgsUpdated(res);
        return;
    }
    if (job->error()) {
        kWarning() << "Job error: " << job->errorString();
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
