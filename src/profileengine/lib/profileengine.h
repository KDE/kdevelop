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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef PROFILEENGINE_H
#define PROFILEENGINE_H

#include <QMap>
#include <Q3ValueList>
#include <kservicetypetrader.h>
#include "profile.h"

/**
Profile listing operation.
Used to get a plain list of profiles
and store it in the QMap<QString, Profile*>.
*/
class KDEVPROFILES_EXPORT ProfileListing{
public:
    void operator() (Profile *profile)
    {
        profiles[profile->name()] = profile;
    }

    QMap<QString, Profile*> profiles;
};

/**Profile resource listing operation.
Used to get a list of urls to the profile resources.

Resource urls can be filtered by an @p filter parameter
passed to the constructor. Filter can have values
as described in @ref QDir::setNameFilter function documentation.*/
class KDEVPROFILES_EXPORT ProfileListingEx {
public:
    ProfileListingEx(const QString &filter): m_filter(filter) {}
    void operator() (Profile *profile)
    {
        resourceList += profile->resources(m_filter);
    }

    KUrl::List resourceList;
    QString m_filter;
};

/**
Profile engine.

- Uses KDevelop profiles to form lists of plugin offers;
- Provides means of managing profiles;
- Provides means to access the resources provided by a profile.

KDevelop profiles form a tree with a root profile named "KDevelop".
For example, such profiles tree can look as:
@code
KDevelop
- IDE
  - CompiledLanguageIDE
    - AdaIDE
    - CandCppIDE
      - CIDE
      - CppIDE
        - KDECppIDE
    - FortranIDE
    ...
  - DatabaseIDE
  - ScriptingLanguageIDE
  ..
- KDevAssistant
@endcode
To manage a tree of profiles, use @ref ProfileEngine::walkProfiles methods.
*/
class KDEVPROFILES_EXPORT ProfileEngine {
public:
    ProfileEngine();
    ~ProfileEngine();

    /**Type of the plugin offer. Engine will usually find profiles and return offers
    of selected type.
    @sa KDevPlugin class documentation for more information of plugin types.*/
    enum OfferType {
        Global    /**<Global plugins.*/,
        Project   /**<Project plugins.*/,
        Core      /**<Core plugins.*/
    };

    /** @return The list of plugin offers for given profile and type.*/
    KService::List offers(const QString &profileName, OfferType offerType);

    /** @return The list of all plugin offers for given type.*/
    KService::List allOffers(OfferType offerType);

    /**@return The list of URLs to the resources (files) with given @p extension.
    @param profileName A name of a profile to find resources in.
    @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
    for name filters syntax.*/
    KUrl::List resources(const QString &profileName, const QString &nameFilter);

    /**@return The list of URLs to the resources (files) with given @p extension.
    This list is obtained by a recursive search that process given profile
    and all it's subprofiles.
    @param profileName A name of a profile to find resources in.
    @param nameFilter Name filter for files. @see QDir::setNameFilter documentation
    for name filters syntax.*/
    KUrl::List resourcesRecursive(const QString &profileName, const QString &nameFilter);

    /**Adds a resource for the profile. Resource will be copied to the user profile directory
    (like $HOME/.kde/share/apps/kdevelop/profiles/...).
    @param profileName A name of the profile.
    @param url The url to a file to copy as a profile resource.*/
    void addResource(const QString &profileName, const KUrl &url);

    /**Gets the difference between @p profile1 and @p profile2.
    Difference is calculated as two lists of plugins to be unloaded and loaded
    in order to switch from @p profile1 to @p profile2.
    @param offerType A type of plugin offers to list.
    @param profile1 A name of the first profile.
    @param profile2 A name of the second profile.
    @param unload Will be filled with a list of plugins to unload.
    @param load Will be filled with a list of plugins to load.
    @note Resulting lists are not cleared. Pass only clean lists in the
    common case.*/
    void diffProfiles(OfferType offerType, const QString &profile1, const QString &profile2,
        QStringList &unload, KService::List &load);

    /**@return The root profile. Root profile is always named "KDevelop" and it
    defines an empty list of plugins. Applications built on KDevelop platform
    will define nested profiles.*/
    Profile *rootProfile() const { return m_rootProfile; }
    /**Finds a profile with given name.
    @return The profile found or 0 if it does not exist.*/
    Profile *findProfile(const QString &profileName);

    /**Walks profiles tree and applies operation @p op to each profile found
    in the tree below @p root (@p root profile itself is not processed).

    Operation is a class that have operator(Profile *).
    Example of operation class which is used to build a plain list of profiles:
    @code
    class ProfileListing{
    public:
        void operator() (Profile *profile)
        {
            profiles[profile->name()] = profile;
        }

        QMap<QString, Profile*> profiles;
    };
    @endcode
    Use case for such operation - building a list of all profiles:
    @code
    ProfileEngine engine;
    ProfileListing listing;
    engine.walkProfiles<ProfileListing>(listing, engine.rootProfile());
    @endcode

    @note @ref ProfileListing and @ref ProfileListingEx operations are already defined in
    profileengine.h header file.

    @param op An operation to apply.
    @param root A profile to start walking from. Complete subtree of the @p root is traversed.
    */
    template<class Operation>
    void walkProfiles(Operation &op, Profile *root)
    {
        Q3ValueList<Profile*> children = root->children();
        for (Q3ValueList<Profile*>::iterator it = children.begin(); it != children.end(); ++it)
        {
            op(*it);
            walkProfiles<Operation>(op, *it);
        }
    }
    /**Walks profiles tree and applies operation @p op to each profile
    found in the tree below @p root (@p root profile itself is not processed)
    but the operation in this case returns a result of type defined by
    "Result" template parameter.

    When iterating the tree, the result of operation applied to the parent profile
    is passed as @p result parameter to the recursive call for child profiles.

    For example, this function can be used to build another hierarcy of profiles
    or other objects connected to profiles.
    Example of operation class which is used to build a listview with items
    where each item represents a profile:
    @code
    class ProfileListBuilding {
    public:
        ProfileItem * operator() (ProfileItem *parent, Profile *profile)
        {
            parent->setOpen(true);
            return new ProfileItem(parent, profile);
        }
    };

    class ProfileItem: public K3ListViewItem {
    public:
        ProfileItem(K3ListView *parent, Profile *profile)
            :K3ListViewItem(parent), m_profile(profile)
        {
            setText(0, profile->genericName());
            setText(1, profile->description());
        }

        ProfileItem(K3ListViewItem *parent, Profile *profile)
            : K3ListViewItem(parent), m_profile(profile)
        {
            setText(0, profile->genericName());
            setText(1, profile->description());
        }

        Profile *profile() const { return m_profile; }

    private:
        Profile *m_profile;
    };

    @endcode
    Use case for such operation - building a listview:
    @code
    ProfileEngine engine;
    ProfileItem *item = new ProfileItem(profilesList, engine.rootProfile());
    ProfileListBuilding op;
    engine.walkProfiles<ProfileListBuilding, ProfileItem>(op, item, engine.rootProfile());
    @endcode

    @param op An operation to apply.
    @param result A result of the operation as it would have been applied to the @p root.
    @param root A profile to start walking from. Complete subtree of the @p root is traversed.
    */
    template<class Operation, class Result>
    void walkProfiles(Operation &op, Result *result, Profile *root)
    {
        Q3ValueList<Profile*> children = root->children();
        for (Q3ValueList<Profile*>::iterator it = children.begin(); it != children.end(); ++it)
        {
            Result *newResult = op(result, *it);
            walkProfiles<Operation>(op, newResult, *it);
        }
    }

protected:
    void processDir(const QString &dir, const QString &currPath, QMap<QString, Profile*> &passedPaths, Profile *root);

    KUrl::List resources(Profile *profile, const QString &nameFilter);

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
