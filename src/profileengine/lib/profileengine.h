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

    Profile *rootProfile() const { return m_rootProfile; }
    
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

private:
    Profile *m_rootProfile;
};

#endif
