/***************************************************************************
 *   Copyright 2004 by Alexander Dymo <adymo@kdevelop.org>             *
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
#ifndef KDEVPROFILE_H
#define KDEVPROFILE_H

#include <QtCore/QStringList>
#include <kurl.h>

#include <shellexport.h>

namespace KDevelop
{

/**
@short KDevelop profile

A class which represents a profile for KDevelop platform stored on disk.
*/
class KDEVPLATFORMSHELL_EXPORT Profile {
public:
    /**An entry in the lists that store profile information*/
    struct Entry {
        Entry() {}
        Entry(const QString &_name, bool _derived): name(_name), derived(_derived) {}
        QString name;
        bool derived;
    };
    typedef QList<Entry> EntryList;

    /**Lists which are held by a profile.*/
    enum List {
        Properties        /**<X-KDevelop-Properties defined for this profile.*/,
        ExplicitEnable    /**<A list of explicitly enabled plugins (names).*/,
        ExplicitDisable   /**<A list of explicitly disabled plugins (names).*/
    };

    Profile(Profile *parent, const QString &name);
    Profile(Profile *parent, const QString &name, const QString &genericName, const QString &description);
    ~Profile();

    QList<Profile*> children() const;
    Profile *parent() const;

    void save();
    bool remove();

    QString name() const;
    QString genericName() const;
    QString description() const;

    EntryList list(List type);

    void addEntry(List type, const QString &value);
    void removeEntry(List type, const QString &value);

    bool hasInEntryList(EntryList &list, const QString& value);

    KUrl::List resources(const QString &nameFilter);
    void addResource(const KUrl &url);

    void detachFromParent();

protected:
    void addChildProfile(Profile *profile);
    void removeChildProfile(Profile *profile);
    QString dirName() const;

    QStringList &listByType(List type);

private:
    class ProfilePrivate* const d;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
