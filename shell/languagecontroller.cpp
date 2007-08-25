/***************************************************************************
 *   Copyright 2006 Adam Treat <treat@kde.org>                         *
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>             *
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
#include "languagecontroller.h"

#include <QHash>

#include <kmimetype.h>

#include "idocument.h"
#include "idocumentcontroller.h"
#include "iplugin.h"
#include "iplugincontroller.h"
#include "ilanguagesupport.h"

#include "core.h"
#include "language.h"
#include "backgroundparser.h"

namespace KDevelop {

typedef QHash<QString, Language*> LanguageHash;
typedef QHash<QString, QList<ILanguage*> > LanguageCache;

struct LanguageControllerPrivate {
    LanguageControllerPrivate(LanguageController *controller)
        : backgroundParser(new BackgroundParser(controller)), m_controller(controller) {}

    void documentActivated(KDevelop::IDocument *document)
    {
        KUrl url = document->url();
        if (!url.isValid()) {
            return;
        }

        foreach (ILanguage *lang, activeLanguages) {
            lang->deactivate();
        }

        activeLanguages.clear();

        QList<ILanguage*> languages = m_controller->languagesForUrl(url);
        foreach (ILanguage *lang, languages) {
            lang->activate();
            activeLanguages << lang;
        }
    }

    QList<ILanguage*> activeLanguages;

    LanguageHash languages;
    LanguageCache languageCache;

    BackgroundParser *backgroundParser;

private:
    LanguageController *m_controller;
};

LanguageController::LanguageController(QObject *parent)
    : ILanguageController(parent)
{
    d = new LanguageControllerPrivate(this);
}

LanguageController::~LanguageController()
{
    delete d;
}

void LanguageController::initialize()
{
    connect(Core::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)),
            SLOT(documentActivated(KDevelop::IDocument*)));
}

QList<ILanguage*> LanguageController::activeLanguages()
{
    return d->activeLanguages;
}

ILanguage *LanguageController::language(const QString &name) const
{
    return d->languages[name];
}

QList<ILanguage*> LanguageController::languagesForUrl(const KUrl &url)
{
    KMimeType::Ptr mimeType = KMimeType::findByUrl(url);

    QList<ILanguage*> languages;
    LanguageCache::iterator it = d->languageCache.find(mimeType->name());
    if (it != d->languageCache.constEnd()) {
        languages = it.value();
    } else {
        QStringList constraints;
        constraints << QString("'%1' in [X-KDevelop-SupportedMimeTypes]").arg(mimeType->name());
        QList<IPlugin*> supports = Core::self()->pluginController()->
            allPluginsForExtension("ILanguageSupport", constraints);

        foreach (IPlugin *support, supports) {
            ILanguageSupport *languageSupport = support->extension<ILanguageSupport>();
            if (ILanguage *lang = language(languageSupport->name())) {
                languages << lang;
            } else {
                Language *_lang = new Language(languageSupport, this);
                d->languages.insert(languageSupport->name(), _lang);
                languages << _lang;
            }
        }
        d->languageCache.insert(mimeType->name(), languages);
    }

    return languages;
}

BackgroundParser *LanguageController::backgroundParser() const
{
    return d->backgroundParser;
}

}

#include "languagecontroller.moc"

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on;
