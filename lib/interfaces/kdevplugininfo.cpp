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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kdevplugininfo.h"

#include <qvariant.h>
//Added by qt3to4:
#include <QList>

#include <kservice.h>
#include <kdebug.h>

#include "kdevplugincontroller.h"

/** @todo remove this include when kdelibs development settled down.
   #include <Q3ValueList> is needed to use KTrader::OfferList as of current kdelibs4_snapshot
*/
#include <Q3ValueList>

struct KDevPluginInfo::Private {
    QString m_pluginName;
    QString m_rawGenericName;

    QString m_genericName;
    QString m_description;
    QString m_icon;

    QString m_version;
    int m_licenseType;
    QString m_copyrightStatement;
    QString m_homePageAddress;
    QString m_bugsEmailAddress;

    QList<KAboutPerson> m_authors;
    QList<KAboutPerson> m_credits;

    KAboutData *m_data;
};


KDevPluginInfo::KDevPluginInfo(const QString &pluginName)
    :d(new Private())
{
    d->m_pluginName = pluginName;

    KService::Ptr offer = KService::serviceByDesktopName(pluginName);
    if (offer)
    {
        d->m_genericName = offer->genericName();
        d->m_icon = offer->icon();
        d->m_description = offer->comment();

        d->m_rawGenericName = offer->untranslatedGenericName();

        d->m_version = offer->property("X-KDevelop-Plugin-Version").toString();
        d->m_homePageAddress = offer->property("X-KDevelop-Plugin-Homepage").toString();
        d->m_bugsEmailAddress = offer->property("X-KDevelop-Plugin-BugsEmailAddress").toString();
        d->m_copyrightStatement = offer->property("X-KDevelop-Plugin-Copyright").toString();

        QString lic = offer->property("X-KDevelop-Plugin-License").toString();
        if (lic == "GPL")
            d->m_licenseType = KAboutData::License_GPL;
        else if (lic == "LGPL")
            d->m_licenseType = KAboutData::License_LGPL;
        else if (lic == "BSD")
            d->m_licenseType = KAboutData::License_BSD;
        else if (lic == "QPL")
            d->m_licenseType = KAboutData::License_QPL;
        else if (lic == "Artistic")
            d->m_licenseType = KAboutData::License_Artistic;
        else if (lic == "Custom")
            d->m_licenseType = KAboutData::License_Custom;
        else
            d->m_licenseType = KAboutData::License_Unknown;

        d->m_data = new KAboutData(d->m_pluginName.ascii(), d->m_rawGenericName.ascii(), "1", 0, d->m_licenseType);
    }
    else
	kdDebug() << "Unable to load information for plugin: " << pluginName
	    << ". Check if " << pluginName << ".desktop exists." << endl;
}

KDevPluginInfo::~KDevPluginInfo()
{
    delete d;
}

KDevPluginInfo::operator KAboutData *() const
{
    return d->m_data;
}

QString KDevPluginInfo::pluginName() const
{
    return d->m_pluginName;
}

QString KDevPluginInfo::genericName() const
{
    return d->m_genericName;
}

QString KDevPluginInfo::icon() const
{
    return d->m_icon;
}

QString KDevPluginInfo::description() const
{
    return d->m_description;
}

QString KDevPluginInfo::version() const
{
    return d->m_version;
}

int KDevPluginInfo::licenseType() const
{
    return d->m_licenseType;
}

QString KDevPluginInfo::license() const
{
    KDevPluginInfo &info = *const_cast<KDevPluginInfo*>(this);
//    return KAboutData(info).license();
    KAboutData *data = info;
    return data->license();
}

QString KDevPluginInfo::copyrightStatement() const
{
    return d->m_copyrightStatement;
}

QString KDevPluginInfo::homePageAddress() const
{
    return d->m_homePageAddress;
}

QString KDevPluginInfo::bugsEmailAddress() const
{
    return d->m_bugsEmailAddress;
}

QVariant KDevPluginInfo::property(const QString &name) const
{
    KTrader::OfferList offers = KDevPluginController::queryPlugins(QString("Name='%1'").arg(d->m_pluginName));
    if (offers.count() == 1)
        return offers.first()->property(name);
    return QVariant();
}

QVariant KDevPluginInfo::operator [](const QString &name) const
{
    return property(name);
}

QStringList KDevPluginInfo::propertyNames( ) const
{
    KTrader::OfferList offers = KDevPluginController::queryPlugins(QString("Name='%1'").arg(d->m_pluginName));
    if (offers.count() == 1)
        return offers.first()->propertyNames();
    return QStringList();
}

void KDevPluginInfo::addAuthor(const char *name, const char *task,
    const char *emailAddress, const char *webAddress)
{
    d->m_authors.append(KAboutPerson(name, task, emailAddress, webAddress));
}

void KDevPluginInfo::addCredit(const char *name, const char *task,
    const char *emailAddress, const char *webAddress)
{
    d->m_credits.append(KAboutPerson(name, task, emailAddress, webAddress));
}
