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
#include "profileengine.h"

#include <qdir.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <kdevplugin.h>

ProfileEngine::ProfileEngine()
{
    QStringList dirs = KGlobal::dirs()->findDirs("data", "kdevelop/profiles");
    
    m_rootProfile = new Profile(0, "KDevelop");
    
    QString currPath = "/";
    QMap<QString, Profile*> passedPaths;
    
    for (QStringList::const_iterator it = dirs.constBegin(); it != dirs.constEnd(); ++it)
        processDir(*it, currPath, passedPaths, m_rootProfile);
}

ProfileEngine::~ProfileEngine()
{
    delete m_rootProfile;
}

void ProfileEngine::processDir(const QString &dir, const QString &currPath, QMap<QString, Profile*> &passedPaths, Profile *root)
{
//     kdDebug() << "processDir: " << dir << " " << currPath << endl;
    
    QDir qDir(dir);
    QStringList entryList = qDir.entryList(QDir::Dirs);
    for (QStringList::const_iterator eit = entryList.constBegin(); eit != entryList.constEnd(); ++eit)
    {
        if ((*eit != "..") && (*eit != "."))
        {
            QString dirName = *eit;
            Profile *profile = 0;
            if (passedPaths.contains(currPath + dirName))
                profile = passedPaths[currPath + dirName];
            else
            {
                profile = new Profile(root, dirName);
                passedPaths[currPath + dirName] = profile;
            }
            processDir(dir + *eit + "/", currPath + dirName, passedPaths, profile);
        }
    }
}

KTrader::OfferList ProfileEngine::offers(const QString &profileName, OfferType offerType)
{
    ProfileListing listing;
    Profile *profile;
    if (profileName == "KDevelop")
        profile = m_rootProfile;
    else
    {
        walkProfiles<ProfileListing>(listing, m_rootProfile);
        profile = listing.profiles[profileName];
    }

    if (!profile)
        return KTrader::OfferList();
    
    QString constraint = QString::fromLatin1("[X-KDevelop-Version] == %1").arg(KDEVELOP_PLUGIN_VERSION);
    switch (offerType) {
        case Global:
            constraint += QString::fromLatin1(" and [X-KDevelop-Scope] == 'Global'");
            break;
        case Project:
            constraint += QString::fromLatin1(" and [X-KDevelop-Scope] == 'Project'");
            break;
        case Core:
            constraint += QString::fromLatin1(" and [X-KDevelop-Scope] == 'Core'");
            break;
    }
    QString constraint_add = "";
    Profile::EntryList properties = profile->list(Profile::Properties);
    int i = 0;
    for (Profile::EntryList::const_iterator it = properties.begin(); it != properties.end(); ++it)
        constraint_add += QString::fromLatin1(" %1 '%2' in [X-KDevelop-Properties]").
            arg((i++)==0?"":"or").arg((*it).name);
    if (!constraint_add.isEmpty())
        constraint += " and ( " + constraint_add + " ) ";
    
//BEGIN debug
    kdDebug() << "=============" << endl
        << "    =============" << endl
        << "        =============   Query for Profile:" << endl
        << "        " << constraint << endl << endl << endl;
//END debug
    
    KTrader::OfferList list = KTrader::self()->query(QString::fromLatin1("KDevelop/Plugin"), constraint);
    QStringList names;

    Profile::EntryList disableList = profile->list(Profile::ExplicitDisable);
//     for (KTrader::OfferList::iterator it = list.begin(); it != list.end(); ++it)
    KTrader::OfferList::iterator it = list.begin();
    while (it != list.end())
    {
        QString name = (*it)->name();
        names.append(name);
        if (profile->hasInEntryList(disableList, name))
        {
            it = list.remove(it);
            continue;
        }
        ++it;
    }
    
    Profile::EntryList enableList = profile->list(Profile::ExplicitEnable);
    for (Profile::EntryList::const_iterator it = enableList.begin(); it != enableList.end(); ++it)
    {
        if (names.contains((*it).name))
            continue;
        QString constraint = QString::fromLatin1("[X-KDevelop-Version] == %1").arg(KDEVELOP_PLUGIN_VERSION);
        constraint += QString::fromLatin1("and [Name] == '%1'").arg((*it).name);
        KTrader::OfferList enable = KTrader::self()->query(QString::fromLatin1("KDevelop/Plugin"), constraint);
        list += enable;
    }

/*//BEGIN debug
    kdDebug() << "=============" << endl
        << "    =============" << endl
        << "        =============   Plugins for Profile:" << endl;
    for (KTrader::OfferList::const_iterator it = list.begin(); it != list.end(); ++it)
        kdDebug() << "        " << (*it)->name() << endl;
    kdDebug() << endl << endl;
//END debug*/ 

    return list;
}

KTrader::OfferList ProfileEngine::allOffers(OfferType offerType)
{
    QString constraint = QString::fromLatin1("[X-KDevelop-Version] == %1").arg(KDEVELOP_PLUGIN_VERSION);
    switch (offerType) {
        case Global:
            constraint += QString::fromLatin1(" and [X-KDevelop-Scope] == 'Global'");
            break;
        case Project:
            constraint += QString::fromLatin1(" and [X-KDevelop-Scope] == 'Project'");
            break;
        case Core:
            constraint += QString::fromLatin1(" and [X-KDevelop-Scope] == 'Core'");
            break;
    }
    return KTrader::self()->query(QString::fromLatin1("KDevelop/Plugin"), constraint);
}
