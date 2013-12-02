/***************************************************************************
 *   Copyright 2006 Adam Treat <treat@kde.org>                             *
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
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
#include <QMutexLocker>

#include <kmimetype.h>

#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>

#include "core.h"
#include "language.h"
#include "settings/ccpreferences.h"
#include "completionsettings.h"
#include <QThread>

// Maximum length of a string to still consider it as a file extension which we cache
// This has to be a slow value, so that we don't fill our file extension cache with crap
static const int maximumCacheExtensionLength = 3;

namespace KDevelop {


typedef QHash<QString, ILanguage*> LanguageHash;
typedef QHash<QString, QList<ILanguage*> > LanguageCache;

struct LanguageControllerPrivate {
    LanguageControllerPrivate(LanguageController *controller)
        : dataMutex(QMutex::Recursive), backgroundParser(new BackgroundParser(controller)), m_cleanedUp(false), m_controller(controller) {}

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

    mutable QMutex dataMutex;
    
    LanguageHash languages; //Maps language-names to languages
    LanguageCache languageCache; //Maps mimetype-names to languages
    typedef QMultiMap<KMimeType::Ptr, ILanguage*> MimeTypeCache;
    MimeTypeCache mimeTypeCache; //Maps mimetypes to languages
    // fallback cache for file extensions not handled by any pattern
    typedef QMap<QString, QList<ILanguage*> > FileExtensionCache;
    FileExtensionCache fileExtensionCache;

    BackgroundParser *backgroundParser;
    bool m_cleanedUp;
    
    ILanguage* addLanguageForSupport(ILanguageSupport* support, const QStringList& mimetypes);
    ILanguage* addLanguageForSupport(ILanguageSupport* support);

private:
    LanguageController *m_controller;
};

ILanguage* LanguageControllerPrivate::addLanguageForSupport(ILanguageSupport* languageSupport,
                                                            const QStringList& mimetypes)
{
    Q_ASSERT(!languages.contains(languageSupport->name()));

    ILanguage* ret = new Language(languageSupport, m_controller);
    languages.insert(languageSupport->name(), ret);

    foreach(const QString& mimeTypeName, mimetypes) {
        kDebug(9505) << "adding supported mimetype:" << mimeTypeName << "language:" << languageSupport->name();
        languageCache[mimeTypeName] << ret;
        KMimeType::Ptr mime = KMimeType::mimeType(mimeTypeName);
        if(mime) {
            mimeTypeCache.insert(mime, ret);
        } else {
            kWarning() << "could not create mime-type" << mimeTypeName;
        }
    }

    return ret;
}

ILanguage* LanguageControllerPrivate::addLanguageForSupport(KDevelop::ILanguageSupport* languageSupport)
{
    if(languages.contains(languageSupport->name()))
        return languages[languageSupport->name()];

    Q_ASSERT(dynamic_cast<IPlugin*>(languageSupport));

    QVariant mimetypes = Core::self()->pluginController()->pluginInfo(dynamic_cast<IPlugin*>(languageSupport)).property("X-KDevelop-SupportedMimeTypes");

    return addLanguageForSupport(languageSupport, mimetypes.toStringList());
}

LanguageController::LanguageController(QObject *parent)
    : ILanguageController(parent)
{
    setObjectName("LanguageController");
    d = new LanguageControllerPrivate(this);
}

LanguageController::~LanguageController()
{
    delete d;
}

void LanguageController::initialize()
{
    d->backgroundParser->loadSettings();

    // make sure the DUChain is setup before we try to access it from different threads at the same time
    DUChain::self();

    connect(Core::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)),
            SLOT(documentActivated(KDevelop::IDocument*)));
}

void LanguageController::cleanup()
{
    QMutexLocker lock(&d->dataMutex);
    d->m_cleanedUp = true;
}

QList<ILanguage*> LanguageController::activeLanguages()
{
    QMutexLocker lock(&d->dataMutex);
    
    return d->activeLanguages;
}

ICompletionSettings *LanguageController::completionSettings() const {
    return &CompletionSettings::self();
}

QList<ILanguage*> LanguageController::loadedLanguages() const
{
    QMutexLocker lock(&d->dataMutex);
    QList<ILanguage*> ret;
    
    if(d->m_cleanedUp)
        return ret;
    
    foreach(ILanguage* lang, d->languages)
        ret << lang;
    return ret;
}

ILanguage *LanguageController::language(const QString &name) const
{
    QMutexLocker lock(&d->dataMutex);
    
    if(d->m_cleanedUp)
        return 0;
    
    if(d->languages.contains(name))
        return d->languages[name];
    else{
        ILanguage* ret = 0;
        QStringList constraints;
        constraints << QString("'%1' == [X-KDevelop-Language]").arg(name);
        QList<IPlugin*> supports = Core::self()->pluginController()->
            allPluginsForExtension("ILanguageSupport", constraints);
        if(!supports.isEmpty()) {
            ILanguageSupport *languageSupport = supports[0]->extension<ILanguageSupport>();
            if(supports[0])
                ret = d->addLanguageForSupport(languageSupport);
        }
        return ret;
    }
}

bool isNumeric(const QString& str)
{
    int len = str.length();
    if(len == 0)
        return false;
    for(int a = 0; a < len; ++a)
        if(!str[a].isNumber())
            return false;
    return true;
}

QList<ILanguage*> LanguageController::languagesForUrl(const KUrl &url)
{
    QMutexLocker lock(&d->dataMutex);
    
    QList<ILanguage*> languages;
    
    if(d->m_cleanedUp)
        return languages;
    
    const QString fileName = url.fileName();

    ///TODO: cache regexp or simple string pattern for endsWith matching
    QRegExp exp("", Qt::CaseInsensitive, QRegExp::Wildcard);
    ///non-crashy part: Use the mime-types of known languages
    for(LanguageControllerPrivate::MimeTypeCache::const_iterator it = d->mimeTypeCache.constBegin();
        it != d->mimeTypeCache.constEnd(); ++it)
    {
        foreach(const QString& pattern, it.key()->patterns()) {
            if(pattern.startsWith('*')) {
                const QStringRef subPattern = pattern.midRef(1);
                if (!subPattern.contains('*')) {
                    //optimize: we can skip the expensive QRegExp in this case
                    //and do a simple string compare (much faster)
                    if (fileName.endsWith(subPattern)) {
                        languages << *it;
                    }
                    continue;
                }
            }

            exp.setPattern(pattern);
            if(int position = exp.indexIn(fileName)) {
                if(position != -1 && exp.matchedLength() + position == fileName.length())
                    languages << *it;
            }
        }
    }

    if(!languages.isEmpty())
        return languages;

    // no pattern found, try the file extension cache
    int extensionStart = fileName.lastIndexOf(QLatin1Char('.'));
    QString extension;
    if(extensionStart != -1)
    {
        extension = fileName.mid(extensionStart+1);
        if(extension.size() > maximumCacheExtensionLength || isNumeric(extension))
            extension = QString();
    }

    if(!extension.isEmpty())
    {
        languages = d->fileExtensionCache.value(extension);
        if(languages.isEmpty() && d->fileExtensionCache.contains(extension))
            return languages; // Nothing found, but was in the cache
    }

    //Never use findByUrl from within a background thread, and never load a language support
    //from within the backgruond thread. Both is unsafe, and can lead to crashes
    if(!languages.isEmpty() || QThread::currentThread() != thread())
        return languages;

    KMimeType::Ptr mimeType;
    
    if(!extension.isEmpty()) {
        // If we have recognized a file extension, allow using the file-contents
        // to look up the type. We will cache it after all.
        mimeType = KMimeType::findByUrl(url);
    } else {
        // If we have not recognized a file extension, do not allow using the file-contents
        // to look up the type. We cannot cache the result, and thus we might end up reading
        // the contents of every single file, which can make the application very unresponsive.
        mimeType = KMimeType::findByUrl(url, 0, false, true);

        if (mimeType->isDefault()) {
            // ask the document controller about a more concrete mimetype
            IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
            if (doc) {
                mimeType = doc->mimeType();
            }
        }
    }

    languages = languagesForMimetype(mimeType->name());

    if(!extension.isEmpty())
        d->fileExtensionCache.insert(extension, languages);

    return languages;
}

QList<ILanguage*> LanguageController::languagesForMimetype(const QString& mimetype)
{
    QMutexLocker lock(&d->dataMutex);

    QList<ILanguage*> languages;
    LanguageCache::ConstIterator it = d->languageCache.constFind(mimetype);
    if (it != d->languageCache.constEnd()) {
        languages = it.value();
    } else {
        QStringList constraints;
        constraints << QString("'%1' in [X-KDevelop-SupportedMimeTypes]").arg(mimetype);
        QList<IPlugin*> supports = Core::self()->pluginController()->
            allPluginsForExtension("ILanguageSupport", constraints);

        if (supports.isEmpty()) {
            kDebug(9505) << "no languages for mimetype:" << mimetype;
            d->languageCache.insert(mimetype, QList<ILanguage*>());
        } else {
            foreach (IPlugin *support, supports) {
                ILanguageSupport* languageSupport = support->extension<ILanguageSupport>();
                kDebug(9505) << "language-support:" << languageSupport;
                if(languageSupport)
                    languages << d->addLanguageForSupport(languageSupport);
            }
        }
    }
    return languages;
}

QList<QString> LanguageController::mimetypesForLanguageName(const QString& languageName)
{
    QMutexLocker lock(&d->dataMutex);

    QList<QString> mimetypes;
    for (LanguageCache::ConstIterator iter = d->languageCache.constBegin(); iter != d->languageCache.constEnd(); ++iter) {
        foreach (ILanguage* language, iter.value()) {
            if (language->name() == languageName) {
                mimetypes << iter.key();
                break;
            }
        }
    }
    return mimetypes;
}

BackgroundParser *LanguageController::backgroundParser() const
{
    return d->backgroundParser;
}

void LanguageController::addLanguageSupport(ILanguageSupport* languageSupport, const QStringList& mimetypes)
{
    d->addLanguageForSupport(languageSupport, mimetypes);
}

}

#include "languagecontroller.moc"

