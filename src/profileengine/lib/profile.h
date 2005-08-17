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
#ifndef PROFILE_H
#define PROFILE_H

#include <kurl.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>

/**
@short KDevelop profile

A class which represents a profile for KDevelop platform stored on disk.
*/
class Profile {
public:
    /**An entry in the lists that store profile information*/
    struct Entry {
        Entry() {}
        Entry(const QString &_name, bool _derived): name(_name), derived(_derived) {}
        QString name;
        bool derived;
    };
    typedef Q3ValueList<Entry> EntryList;
    
    /**Lists which are held by a profile.*/
    enum List { 
        Properties        /**<X-KDevelop-Properties defined for this profile.*/,
        ExplicitEnable    /**<A list of explicitly enabled plugins (names).*/,
        ExplicitDisable   /**<A list of explicitly disabled plugins (names).*/
    };

    Profile(Profile *parent, const QString &name);
    Profile(Profile *parent, const QString &name, const QString &genericName, const QString &description);
    ~Profile();
    
    Q3ValueList<Profile*> children() const { return m_children; }
    Profile *parent() const { return m_parent; }
    
    void save();
    bool remove();
    
    QString name() const { return m_name; }
    QString genericName() const { return m_genericName; }
    QString description() const { return m_description; }
    
    EntryList list(List type);
    
    void addEntry(List type, const QString &value);
    void removeEntry(List type, const QString &value);
    
    bool hasInEntryList(EntryList &list, QString value);
    
    KURL::List resources(const QString &nameFilter);
    void addResource(const KURL &url);
    
    void detachFromParent();
    
protected:
    void addChildProfile(Profile *profile);
    void removeChildProfile(Profile *profile);
    QString dirName() const;
    
    QStringList &listByType(List type);

private:
    Profile *m_parent;
    Q3ValueList<Profile*> m_children;
    
    QString m_name;
    
    QString m_genericName;
    QString m_description;
    
    QStringList m_properties;
    QStringList m_explicitEnable;
    QStringList m_explicitDisable;
};

#endif
