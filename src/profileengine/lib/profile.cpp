/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo <adymo@kdevelop.org>             *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "profile.h"

#include <qdir.h>
#include <qfileinfo.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kio/netaccess.h>

Profile::Profile(Profile *parent, const QString &name)
    :m_parent(parent), m_name(name)
{
    if (m_parent)
        m_parent->addChildProfile(this);
    
    QString profileConfig = locate("data", "kdevelop/profiles" + dirName() + "/profile.config");
    KConfig config(profileConfig);
    
    config.setGroup("Information");
    m_genericName = config.readEntry("GenericName");
    m_description = config.readEntry("Description");
    
    config.setGroup("Properties");
    m_properties = config.readListEntry("List");
    
    config.setGroup("Enable");
    m_explicitEnable = config.readListEntry("List");
    
    config.setGroup("Disable");
    m_explicitDisable = config.readListEntry("List");
}

Profile::Profile(Profile *parent, const QString &name, const QString &genericName, const QString &description)
    :m_parent(parent), m_name(name), m_genericName(genericName), m_description(description)
{
    if (m_parent)
        m_parent->addChildProfile(this);
    save();
}

Profile::~Profile()
{
    for (QValueList<Profile*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        delete *it;
}

void Profile::addChildProfile(Profile *profile)
{
    m_children.append(profile);
}

void Profile::removeChildProfile(Profile *profile)
{
    m_children.remove(profile);
}

QString Profile::dirName() const
{
    if (m_parent)
        return m_parent->dirName() + "/" + m_name;
    else
        return "/"/* + m_name*/;
}

void Profile::save()
{
    QString profileConfig = locateLocal("data", "kdevelop/profiles" + dirName() + "/profile.config");
    KConfig config(profileConfig);
    
    config.setGroup("Information");
    config.writeEntry("GenericName", m_genericName);
    config.writeEntry("Description", m_description);
    
    config.setGroup("Properties");
    config.writeEntry("List", m_properties);
    
    config.setGroup("Enable");
    config.writeEntry("List", m_explicitEnable);
    
    config.setGroup("Disable");
    config.writeEntry("List", m_explicitDisable);
    
    config.sync();
}

Profile::EntryList Profile::list(List type)
{
    EntryList parentList;
    if (m_parent)
        parentList = m_parent->list(type);
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
    list.remove(value);
}

QStringList &Profile::listByType(List type)
{
    switch (type) {
        case Properties:
            return m_properties;
        case ExplicitEnable:
            return m_explicitEnable;
        case ExplicitDisable:
            return m_explicitDisable;
    }
    return QStringList();
}

bool Profile::hasInEntryList(EntryList &list, QString value)
{
    for (EntryList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
        if ((*it).name == value)
            return true;
    return false;
}

bool Profile::remove()
{
    QStringList dirs = KGlobal::dirs()->findDirs("data", "kdevelop/profiles" + dirName());
    if ((dirs.count() == 1) && dirs[0].startsWith(QDir::homeDirPath()))
        return KIO::NetAccess::del(KURL::fromPathOrURL(dirs[0]), 0);
    return false;
}

void Profile::detachFromParent()
{
    if (m_parent)
        m_parent->removeChildProfile(this);
}

KURL::List Profile::resources(const QString &nameFilter)
{
    QStringList resources;
    QStringList resourceDirs = KGlobal::dirs()->resourceDirs("data");
    for (QStringList::const_iterator it = resourceDirs.begin(); it != resourceDirs.end(); ++it)
    {
        QString dir = *it;
        dir = dir + "kdevelop/profiles" + dirName();
        
        QDir d(dir);
        const QFileInfoList *infoList = d.entryInfoList(nameFilter, QDir::Files);
        if (!infoList)
            continue;
        for (QFileInfoList::const_iterator infoIt = infoList->constBegin();
             infoIt != infoList->constEnd(); ++ infoIt)
            resources.append((*infoIt)->absFilePath());
    }
    
    return KURL::List(resources);
}

void Profile::addResource(const KURL &url)
{
    QString saveLocation = KGlobal::dirs()->saveLocation("data", "kdevelop/profiles"+dirName(), true);
    KIO::NetAccess::file_copy(url, KURL::fromPathOrURL(saveLocation), -1, true);
}
