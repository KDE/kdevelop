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
#ifndef PROFILEENGINE_H
#define PROFILEENGINE_H

#include <qmap.h>

#include <ktrader.h>

#include "profile.h"

class ProfileListing{
public:
    void operator() (Profile *profile)
    {
        profiles[profile->name()] = profile;
    }

    QMap<QString, Profile*> profiles;
};

class ProfileListingEx {
public:
    ProfileListingEx(const QString &filter): m_filter(filter) {}
    void operator() (Profile *profile)
    {
        resourceList += profile->resources(m_filter);
    }
    
    KURL::List resourceList;
    QString m_filter;
};

/**
Profile engine.

Uses KDevelop profiles to form lists of plugin offers.
*/
class ProfileEngine {
public:
    ProfileEngine();
    ~ProfileEngine();

    enum OfferType { Global, Project, Core };
    
    /**@return The list of plugin offers for given profile and type.*/
    KTrader::OfferList offers(const QString &profileName, OfferType offerType);
    /**@return The list of all plugin offers for given type.*/
    KTrader::OfferList allOffers(OfferType offerType);
    
    /**@return The list of URLs to the resources (files) with given @p extension.
    @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
    for name filters syntax.*/
    KURL::List resources(const QString &profileName, const QString &nameFilter);
    
    /**@return The list of URLs to the resources (files) with given @p extension.
    This list is obtained by a recursive search that process given profile
    and all it's subprofiles.
    @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
    for name filters syntax.*/
    KURL::List resourcesRecursive(const QString &profileName, const QString &nameFilter);
    
    /**Gets the difference between @p profile1 and @p profile2.
    Difference is calculated as two lists of plugins to be unloaded and loaded
    in order to switch from @p profile1 to @p profile2.
    @param unload Will be filled with a list of plugins to unload.
    @param load Will be filled with a list of plugins to load.
    @note Resulting lists are not cleared. Pass only clean lists in the
    common case.*/
    void diffProfiles(OfferType offerType, const QString &profile1, const QString &profile2, 
        QStringList &unload, KTrader::OfferList &load);

    Profile *rootProfile() const { return m_rootProfile; }
    Profile *findProfile(const QString &profileName);
    
    template<class Operation>
    void walkProfiles(Operation &op, Profile *root)
    {
        QValueList<Profile*> children = root->children();
        for (QValueList<Profile*>::iterator it = children.begin(); it != children.end(); ++it)
        {
            op(*it);
            walkProfiles<Operation>(op, *it);
        }
    }
    template<class Operation, class Result>
    void walkProfiles(Operation &op, Result *result, Profile *root)
    {
        QValueList<Profile*> children = root->children();
        for (QValueList<Profile*>::iterator it = children.begin(); it != children.end(); ++it)
        {
            Result *newResult = op(result, *it);
            walkProfiles<Operation>(op, newResult, *it);
        }
    }

protected:
    void processDir(const QString &dir, const QString &currPath, QMap<QString, Profile*> &passedPaths, Profile *root);

    KURL::List resources(Profile *profile, const QString &nameFilter);
    
    /**Gets a complete listing of available profiles and looks for a profile.
    @param listing Profiles listing will be saved here.
    @param profile Will be a pointer to a profile with the name @p profileName or 0
    if no profile with that name is found.
    @param profileName The name of a profile to find.*/
    void getProfileWithListing(ProfileListing &listing, Profile **profile, 
        const QString &profileName);
    
private:
    Profile *m_rootProfile;
};

#endif
