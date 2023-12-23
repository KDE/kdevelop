/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <ghaccount.h>

#include <ghresource.h>
#include <ghproviderplugin.h>

#include <KSharedConfig>

namespace gh
{

Account::Account(Resource *resource)
{
    m_group = KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("ghprovider"));
    m_resource = resource;
}

void Account::invalidate(const QString &password)
{
    const QString &id = m_group.readEntry("id", QString());
    if (!id.isEmpty())
        m_resource->revokeAccess(id, name(), password);
    m_group.writeEntry("name", "");
    m_group.writeEntry("id", "");
    m_group.writeEntry("token", "");
    m_group.writeEntry("created_at", "");
    m_group.writeEntry("orgs", "");
}

bool Account::validAccount() const
{
    return !m_group.readEntry("id", QString()).isEmpty();
}

void Account::setName(const QString &name)
{
    m_group.writeEntry("name", name);
}

const QString Account::name() const
{
    return m_group.readEntry("name", QString());
}

void Account::setOrgs(const QStringList &orgs)
{
    m_group.writeEntry("orgs", orgs);
}

const QStringList Account::orgs() const
{
    const QString orgs = m_group.readEntry("orgs", QString());
    if (orgs.isEmpty())
        return QStringList();
    return orgs.split(QLatin1Char(','));
}

void Account::saveToken(const QByteArray &id, const QByteArray &token)
{
    m_group.writeEntry("id", id);
    m_group.writeEntry("token", token);
}

const QString Account::token() const
{
    return m_group.readEntry("token", QString());
}

} // End of namespace gh
