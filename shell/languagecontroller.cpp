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
#include <QMimeDatabase>

#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <language/assistant/staticassistantsmanager.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>

#include "core.h"
#include "settings/ccpreferences.h"
#include "completionsettings.h"
#include "debug.h"
#include <QThread>

namespace {
// Maximum length of a string to still consider it as a file extension which we cache
// This has to be a slow value, so that we don't fill our file extension cache with crap
const int maximumCacheExtensionLength = 3;

// TODO: kf5, use QStringLiteral
const QString KEY_SupportedMimeTypes = "X-KDevelop-SupportedMimeTypes";
const QString KEY_ILanguageSupport = "ILanguageSupport";
}

inline uint qHash(const QMimeType& mime, uint seed = 0)
{
    return qHash(mime.name(), seed);
}

namespace KDevelop {


typedef QHash<QString, ILanguageSupport*> LanguageHash;
typedef QHash<QString, QList<ILanguageSupport*> > LanguageCache;

struct LanguageControllerPrivate
{
    LanguageControllerPrivate(LanguageController *controller)
        : dataMutex(QMutex::Recursive)
        , backgroundParser(new BackgroundParser(controller))
        , staticAssistantsManager(nullptr)
        , m_cleanedUp(false)
        , m_controller(controller)
    {}

    void documentActivated(KDevelop::IDocument *document)
    {
        QUrl url = document->url();
        if (!url.isValid()) {
            return;
        }

        activeLanguages.clear();

        QList<ILanguageSupport*> languages = m_controller->languagesForUrl(url);
        foreach (auto lang, languages) {
            activeLanguages << lang;
        }
    }

    QList<ILanguageSupport*> activeLanguages;

    mutable QMutex dataMutex;

    LanguageHash languages; //Maps language-names to languages
    LanguageCache languageCache; //Maps mimetype-names to languages
    typedef QMultiHash<QMimeType, ILanguageSupport*> MimeTypeCache;
    MimeTypeCache mimeTypeCache; //Maps mimetypes to languages
    // fallback cache for file extensions not handled by any pattern
    typedef QMap<QString, QList<ILanguageSupport*> > FileExtensionCache;
    FileExtensionCache fileExtensionCache;

    BackgroundParser *backgroundParser;
    StaticAssistantsManager* staticAssistantsManager;
    bool m_cleanedUp;

    void addLanguageSupport(ILanguageSupport* support, const QStringList& mimetypes);
    void addLanguageSupport(ILanguageSupport* support);

private:
    LanguageController *m_controller;
};

void LanguageControllerPrivate::addLanguageSupport(ILanguageSupport* languageSupport,
                                                            const QStringList& mimetypes)
{
    Q_ASSERT(!languages.contains(languageSupport->name()));
    languages.insert(languageSupport->name(), languageSupport);

    foreach(const QString& mimeTypeName, mimetypes) {
        qCDebug(SHELL) << "adding supported mimetype:" << mimeTypeName << "language:" << languageSupport->name();
        languageCache[mimeTypeName] << languageSupport;
        QMimeType mime = QMimeDatabase().mimeTypeForName(mimeTypeName);
        if (mime.isValid()) {
            mimeTypeCache.insert(mime, languageSupport);
        } else {
            qWarning() << "could not create mime-type" << mimeTypeName;
        }
    }
}

void LanguageControllerPrivate::addLanguageSupport(KDevelop::ILanguageSupport* languageSupport)
{
    if (languages.contains(languageSupport->name()))
        return;

    Q_ASSERT(dynamic_cast<IPlugin*>(languageSupport));

    const QVariant mimetypes = Core::self()->pluginController()->pluginInfo(dynamic_cast<IPlugin*>(languageSupport)).property(KEY_SupportedMimeTypes);
    addLanguageSupport(languageSupport, mimetypes.toStringList());
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
    d->staticAssistantsManager = new StaticAssistantsManager(this);

    // make sure the DUChain is setup before we try to access it from different threads at the same time
    DUChain::self();

    connect(Core::self()->documentController(), &IDocumentController::documentActivated,
            this, [&] (IDocument* document) { d->documentActivated(document); });
}

void LanguageController::cleanup()
{
    QMutexLocker lock(&d->dataMutex);
    d->m_cleanedUp = true;
}

QList<ILanguageSupport*> LanguageController::activeLanguages()
{
    QMutexLocker lock(&d->dataMutex);

    return d->activeLanguages;
}

StaticAssistantsManager* LanguageController::staticAssistantsManager() const
{
    return d->staticAssistantsManager;
}

ICompletionSettings *LanguageController::completionSettings() const {
    return &CompletionSettings::self();
}

QList<ILanguageSupport*> LanguageController::loadedLanguages() const
{
    QMutexLocker lock(&d->dataMutex);
    QList<ILanguageSupport*> ret;

    if(d->m_cleanedUp)
        return ret;

    foreach(ILanguageSupport* lang, d->languages)
        ret << lang;
    return ret;
}

ILanguageSupport* LanguageController::language(const QString &name) const
{
    QMutexLocker lock(&d->dataMutex);

    if(d->m_cleanedUp)
        return 0;

    if(d->languages.contains(name))
        return d->languages[name];

    QVariantMap constraints;
    constraints.insert("X-KDevelop-Language", name);
    QList<IPlugin*> supports = Core::self()->pluginController()->allPluginsForExtension(KEY_ILanguageSupport, constraints);
    if(!supports.isEmpty()) {
        ILanguageSupport *languageSupport = supports[0]->extension<ILanguageSupport>();
        if(languageSupport) {
            d->addLanguageSupport(languageSupport);
            return languageSupport;
        }
    }
    return nullptr;
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

QList<ILanguageSupport*> LanguageController::languagesForUrl(const QUrl &url)
{
    QMutexLocker lock(&d->dataMutex);

    QList<ILanguageSupport*> languages;

    if(d->m_cleanedUp)
        return languages;

    const QString fileName = url.fileName();

    ///TODO: cache regexp or simple string pattern for endsWith matching
    QRegExp exp("", Qt::CaseInsensitive, QRegExp::Wildcard);
    ///non-crashy part: Use the mime-types of known languages
    for(LanguageControllerPrivate::MimeTypeCache::const_iterator it = d->mimeTypeCache.constBegin();
        it != d->mimeTypeCache.constEnd(); ++it)
    {
        foreach(const QString& pattern, it.key().globPatterns()) {
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

    QMimeType mimeType;

    if (url.isLocalFile()) {
        // If we have recognized a file extension, allow using the file-contents
        // to look up the type. We will cache it after all.
        // If we have not recognized a file extension, do not allow using the file-contents
        // to look up the type. We cannot cache the result, and thus we might end up reading
        // the contents of every single file, which can make the application very unresponsive.
        if (!extension.isEmpty()) {
            mimeType = QMimeDatabase().mimeTypeForFile(url.toLocalFile());
        } else {
            // this will not be cached -> don't bother reading the contents
            mimeType = QMimeDatabase().mimeTypeForFile(url.toLocalFile(), QMimeDatabase::MatchExtension);
        }
    } else {
        // remote file, only look at the extension
        mimeType = QMimeDatabase().mimeTypeForUrl(url);
    }
    if (mimeType.isDefault()) {
        // ask the document controller about a more concrete mimetype
        IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
        if (doc) {
            mimeType = doc->mimeType();
        }
    }

    languages = languagesForMimetype(mimeType.name());

    if(!extension.isEmpty()) {
        d->fileExtensionCache.insert(extension, languages);
    }

    return languages;
}

QList<ILanguageSupport*> LanguageController::languagesForMimetype(const QString& mimetype)
{
    QMutexLocker lock(&d->dataMutex);

    QList<ILanguageSupport*> languages;
    LanguageCache::ConstIterator it = d->languageCache.constFind(mimetype);
    if (it != d->languageCache.constEnd()) {
        languages = it.value();
    } else {
        QVariantMap constraints;
        constraints.insert(KEY_SupportedMimeTypes, mimetype);
        QList<IPlugin*> supports = Core::self()->pluginController()->allPluginsForExtension(KEY_ILanguageSupport, constraints);

        if (supports.isEmpty()) {
            qCDebug(SHELL) << "no languages for mimetype:" << mimetype;
            d->languageCache.insert(mimetype, QList<ILanguageSupport*>());
        } else {
            foreach (IPlugin *support, supports) {
                ILanguageSupport* languageSupport = support->extension<ILanguageSupport>();
                qCDebug(SHELL) << "language-support:" << languageSupport;
                if(languageSupport) {
                    d->addLanguageSupport(languageSupport);
                    languages << languageSupport;
                }
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
        foreach (ILanguageSupport* language, iter.value()) {
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
    d->addLanguageSupport(languageSupport, mimetypes);
}

}

#include "moc_languagecontroller.cpp"
