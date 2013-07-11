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


#include <KConfig>
#include <KGlobal>

#include <ghaccount.h>
#include <ghresource.h>
#include <ghproviderplugin.h>


namespace gh
{

Account::Account(Resource *resource)
{
    m_group = KConfigGroup(KGlobal::config(), "ghprovider");
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
    QString res = orgs.join(",");
    m_group.writeEntry("orgs", orgs);
}

const QStringList Account::orgs() const
{
    const QString orgs = m_group.readEntry("orgs", QString());
    if (orgs.isEmpty())
        return QStringList();
    return orgs.split(",");
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
