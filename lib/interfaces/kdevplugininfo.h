/* This file is part of the KDE project
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDEVPLUGININFO_H
#define KDEVPLUGININFO_H

#include <qvariant.h>
//Added by qt3to4:
#include <QList>

#include <kaboutdata.h>

/**
@file kdevplugininfo.h
KDevelop plugin information container class.
*/

/**
Information about KDevelop plugin.
It uses plugin name to query plugin .desktop files and
retrieve most information. Names of authors and credits to other people
must be added manually using KDevPluginInfo::addAuthor and KDevPluginInfo::addCredit
methods.
@sa KDevPlugin class documentation for more information about .desktop files.
*/
class KDevPluginInfo
{
public:
    /**Constructor.
    @param pluginName A name of a plugin. Must be the same as the name of a .desktop file
    and the same as the location of plugin resource files.*/
    KDevPluginInfo(const QString &pluginName);
    
    /**Casts KDevPluginInfo to KAboutData. Uses KDevPluginInfo::pluginName as KAboutData::appName,
    KDevPluginInfo::rawGenericName as KAboutData::programName KDevPluginInfo::licenseType as KAboutData::licenseType. Other parameters are ignored.*/
    operator KAboutData*() const;
    
    /**@return A name of a plugin (always untranslated).*/
    QString pluginName() const;
    /**@return A generic name of a plugin (translated). Use this in GUI. Information is taken from .desktop file.*/
    QString genericName() const;
    /**@return An icon name of a plugin. Information is taken from .desktop file.*/
    QString icon() const;
    /**@return A description of a plugin. Information is taken from .desktop file.*/
    QString description() const;
    /**@return A version of a plugin. Information is taken from .desktop file.*/
    QString version() const;
    /**@return A license type of a plugin. Information is taken from .desktop file.*/
    int licenseType() const;
    /**@return A license text for known license types (GPL, LGPL, BSD, Artistic, QPL).*/
    QString license() const;
    /**@return A copyright statement of a plugin. Information is taken from .desktop file.*/
    QString copyrightStatement() const;
    /**@return A home page of a plugin. Information is taken from .desktop file.*/
    QString homePageAddress() const;
    /**@return A email address for bugs of a plugin. Information is taken from .desktop file.*/
    QString bugsEmailAddress() const;
    
    /**@param name The name of a property.
    @return Any property value which exists in .desktop file.*/
    QVariant property(const QString &name) const;
    /**@param name The name of a property.
    @return Any property value which exists in .desktop file.*/
    QVariant operator [] (const QString &name) const;
    /**@return A list of property names set in .desktop file.*/
    QStringList propertyNames() const;

    /**@return A list of plugin authors.*/
    const QList<KAboutPerson> authors() const;
    /**@return A list of other plugin contributors.*/
    const QList<KAboutPerson> credits() const;

    /**Adds an author to the list of authors.
    You can call this function as many times you need. Each entry is appended to a list.
    @param name The developer's name in UTF-8 encoding.
    @param task What the person is responsible for. This text can contain newlines. It should be marked for translation like this: I18N_NOOP("Task description..."). Can be 0.
    @param emailAddress An Email address where the person can be reached. Can be 0.
    @param webAddress The person's homepage or a relevant link. Start the address with "http://". "http://some.domain" is correct, "some.domain" is not. Can be 0.*/
    void addAuthor(const char *name, const char *task,
        const char *emailAddress = 0, const char *webAddress = 0);
    /**Adds a contributor to the list of contributors.
    You can call this function as many times you need. Each entry is appended to a list.
    @param name The developer's name in UTF-8 encoding.
    @param task What the person is responsible for. This text can contain newlines. It should be marked for translation like this: I18N_NOOP("Task description..."). Can be 0.
    @param emailAddress An Email address where the person can be reached. Can be 0.
    @param webAddress The person's homepage or a relevant link. Start the address with "http://". "http://some.domain" is correct, "some.domain" is not. Can be 0.*/
    void addCredit(const char *name, const char *task = 0,
        const char *emailAddress = 0, const char *webAddress = 0);
    
private:
    /**Returns the untranslated generic name of a plugin as set in .desktop file.*/
    const char *rawGenericName() const;
    
    class Private;
    Private *d;
};

#endif
