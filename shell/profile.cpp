/***************************************************************************
 *   Copyright 2004 Alexander Dymo <adymo@kdevelop.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "profile.h"

#include <QDir>
#include <QFileInfo>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kio/job.h>
#include <kconfiggroup.h>
#include <kio/netaccess.h>
namespace KDevelop
{

class ProfilePrivate
{
public:
    Profile *m_parent;
    QList<Profile*> m_children;

    QString m_name;

    QString m_genericName;
    QString m_description;

    QStringList m_properties;
    QStringList m_explicitEnable;
    QStringList m_explicitDisable;
};

Profile::Profile(Profile *parent, const QString &name)
    :d(new ProfilePrivate)
{
    d->m_parent = parent;
    d->m_name = name;
    if (d->m_parent)
        d->m_parent->addChildProfile(this);

    QString profileConfig = KStandardDirs::locate("data", "kdevplatform/profiles" + dirName() + "/profile.config");
    KConfig config(profileConfig);

    KConfigGroup group = config.group("Information");
    d->m_genericName = group.readEntry("GenericName");
    d->m_description = group.readEntry("Description");

    group = config.group("Properties");
    d->m_properties = group.readEntry("List", QStringList());

    group = config.group("Enable");
    d->m_explicitEnable = group.readEntry("List", QStringList());

    group = config.group("Disable");
    d->m_explicitDisable = group.readEntry("List", QStringList());
}

Profile::Profile(Profile *parent, const QString &name, const QString &genericName, const QString &description)
    :d(new ProfilePrivate)
{
    d->m_parent = parent;
    d->m_name = name;
    d->m_genericName = genericName;
    d->m_description = description;
    if (d->m_parent)
        d->m_parent->addChildProfile(this);
    save();
}

Profile::~Profile()
{
    for (QList<Profile*>::iterator it = d->m_children.begin(); it != d->m_children.end(); ++it)
        delete *it;
}

void Profile::addChildProfile(Profile *profile)
{
    d->m_children.append(profile);
}

void Profile::removeChildProfile(Profile *profile)
{
    d->m_children.removeAll(profile);
}

QString Profile::dirName() const
{
    if (d->m_parent)
        return d->m_parent->dirName() + '/' + d->m_name;
    else
        return "/"/* + d->m_name*/;
}

void Profile::save()
{
    QString profileConfig = KStandardDirs::locateLocal("data", "kdevplatform/profiles" + dirName() + "/profile.config");
    KConfig config(profileConfig);

    KConfigGroup group = config.group("Information");
    group.writeEntry("GenericName", d->m_genericName);
    group.writeEntry("Description", d->m_description);

    group = config.group("Properties");
    group.writeEntry("List", d->m_properties);

    group = config.group("Enable");
    group.writeEntry("List", d->m_explicitEnable);

    group = config.group("Disable");
    group.writeEntry("List", d->m_explicitDisable);

    config.sync();
}

Profile::EntryList Profile::list(List type)
{
    EntryList parentList;
    if (d->m_parent)
        parentList = d->m_parent->list(type);
    EntryList list = parentList;
    for (EntryList::iterator it = list.begin(); it != list.end(); ++it)
        (*it).derived = true;
    QStringList &personalList = listByType(type);
    for (QStringList::const_iterator it = personalList.begin(); it != personalList.end(); ++it)
        list.append(Entry(*it, false));
    return list;
}

void Profile::addEntry(List type, const QString &value)
{
    QStringList &list = listByType(type);
    if (!list.contains(value))
        list.append(value);
}

void Profile::removeEntry(List type, const QString &value)
{
    QStringList &list = listByType(type);
    list.removeAll(value);
}

QStringList &Profile::listByType(List type)
{
    static QStringList dummy;

    switch (type) {
        case Properties:
            return d->m_properties;
        case ExplicitEnable:
            return d->m_explicitEnable;
        case ExplicitDisable:
            return d->m_explicitDisable;
    }
    return dummy;
}

bool Profile::hasInEntryList(EntryList &list, const QString& value)
{
    for (EntryList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
        if ((*it).name == value)
            return true;
    return false;
}

bool Profile::remove()
{
    QStringList dirs = KGlobal::dirs()->findDirs("data", "kdevplatform/profiles" + dirName());
    if ((dirs.count() == 1) && dirs[0].startsWith(QDir::homePath()))
        return KIO::NetAccess::del(KUrl(dirs[0]), 0);
    return false;
}

void Profile::detachFromParent()
{
    if (d->m_parent)
        d->m_parent->removeChildProfile(this);
}

KUrl::List Profile::resources(const QString &nameFilter)
{
    QStringList resources;
    QStringList resourceDirs = KGlobal::dirs()->resourceDirs("data");
    for (QStringList::const_iterator it = resourceDirs.begin(); it != resourceDirs.end(); ++it)
    {
        QString dir = *it;
        dir = dir + "kdevplatform/profiles" + dirName();

        QDir d(dir);
        const QFileInfoList infoList = d.entryInfoList(QStringList(nameFilter), QDir::Files);
        for (int i = 0; i < infoList.count(); ++i)
            resources.append(infoList.at(i).absoluteFilePath());
    }

    return KUrl::List(resources);
}

void Profile::addResource(const KUrl &url)
{
    QString saveLocation = KGlobal::dirs()->saveLocation("data", "kdevplatform/profiles"+dirName(), true);
    KIO::FileCopyJob * job = KIO::file_copy(url, KUrl(saveLocation), -1, KIO::Overwrite);
    job->exec();
}

QList<Profile*> Profile::children() const
{
    return d->m_children;
}

Profile *Profile::parent() const
{
    return d->m_parent;
}

QString Profile::name() const
{
    return d->m_name;
}
QString Profile::genericName() const
{
    return d->m_genericName;
}
QString Profile::description() const
{
    return d->m_description;
}

}

