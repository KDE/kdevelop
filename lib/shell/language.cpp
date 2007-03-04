/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#include "language.h"

#include <kdebug.h>
#include <kmimetype.h>

#include "iplugin.h"
#include "iplugincontroller.h"
#include "ilanguagesupport.h"

#include "core.h"

namespace KDevelop {

struct LanguagePrivate {
    ILanguageSupport *support;

    static QMap<KMimeType::Ptr, QList<Language*> > languageCache;
};

QMap<KMimeType::Ptr, QList<Language*> > LanguagePrivate::languageCache;

Language::Language(ILanguageSupport *support, QObject *parent)
    :ILanguage(support->name(), parent)
{
    d = new LanguagePrivate();
    d->support = support;
}

Language::~Language()
{
    delete d;
}

void Language::deactivate()
{
    kDebug(9000) << "deactivating language " << name() << endl;
}

void Language::activate()
{
    kDebug(9000) << "activating language " << name() << endl;
}

ILanguageSupport *Language::languageSupport()
{
    return d->support;
}

QList<Language*> Language::findByUrl(const KUrl &url, QObject *parent)
{
    KMimeType::Ptr mimeType = KMimeType::findByUrl(url);

    QList<Language*> languages;
    if (LanguagePrivate::languageCache.contains(mimeType))
        languages = LanguagePrivate::languageCache[mimeType];
    else
    {
        QStringList constraints;
        constraints << QString("'%1' in [X-KDevelop-SupportedMimeTypes]").arg(mimeType->name());
        QList<IPlugin*> supports = Core::self()->pluginController()->
            allPluginsForExtension("ILanguageSupport", constraints);

        foreach (IPlugin *support, supports)
        {
            ILanguageSupport *languageSupport = support->extension<ILanguageSupport>();
            languages << new Language(languageSupport, parent);
        }
        LanguagePrivate::languageCache[mimeType] = languages;
    }

    return languages;
}

}

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on;
