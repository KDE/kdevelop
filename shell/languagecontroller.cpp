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
#include <QMimeDatabase>
#include <QMutexLocker>
#include <QThread>

#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <language/assistant/staticassistantsmanager.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>

#include "problemmodelset.h"

#include "core.h"
#include "settings/languagepreferences.h"
#include "completionsettings.h"
#include "debug.h"

namespace {
QString KEY_SupportedMimeTypes() { return QStringLiteral("X-KDevelop-SupportedMimeTypes"); }
QString KEY_ILanguageSupport() { return QStringLiteral("ILanguageSupport"); }
}

#if QT_VERSION < 0x050600
inline uint qHash(const QMimeType& mime, uint seed = 0)
{
    return qHash(mime.name(), seed);
}
#endif

namespace KDevelop {


typedef QHash<QString, ILanguageSupport*> LanguageHash;
typedef QHash<QString, QList<ILanguageSupport*> > LanguageCache;

struct LanguageControllerPrivate
{
    explicit LanguageControllerPrivate(LanguageController *controller)
        : dataMutex(QMutex::Recursive)
        , backgroundParser(new BackgroundParser(controller))
        , staticAssistantsManager(nullptr)
        , m_cleanedUp(false)
        , problemModelSet(new ProblemModelSet(controller))
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
        foreach (const auto lang, languages) {
            activeLanguages << lang;
        }
    }

    QList<ILanguageSupport*> activeLanguages;

    mutable QMutex dataMutex;

    LanguageHash languages; //Maps language-names to languages
    LanguageCache languageCache; //Maps mimetype-names to languages
    typedef QMultiHash<QMimeType, ILanguageSupport*> MimeTypeCache;
    MimeTypeCache mimeTypeCache; //Maps mimetypes to languages

    BackgroundParser *backgroundParser;
    StaticAssistantsManager* staticAssistantsManager;
    bool m_cleanedUp;

    void addLanguageSupport(ILanguageSupport* support, const QStringList& mimetypes);
    void addLanguageSupport(ILanguageSupport* support);

    ProblemModelSet *problemModelSet;

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

    KPluginMetaData info = Core::self()->pluginController()->pluginInfo(dynamic_cast<IPlugin*>(languageSupport));
    QStringList mimetypes = KPluginMetaData::readStringList(info.rawData(), KEY_SupportedMimeTypes());
    addLanguageSupport(languageSupport, mimetypes);
}

LanguageController::LanguageController(QObject *parent)
    : ILanguageController(parent)
{
    setObjectName(QStringLiteral("LanguageController"));
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

ICompletionSettings *LanguageController::completionSettings() const
{
    return &CompletionSettings::self();
}

ProblemModelSet* LanguageController::problemModelSet() const
{
    return d->problemModelSet;
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
        return nullptr;

    if(d->languages.contains(name))
        return d->languages[name];

    // temporary support for deprecated-in-5.1 "X-KDevelop-Language" as fallback
    // remove in later version
    const QString keys[2] = {
        QStringLiteral("X-KDevelop-Languages"),
        QStringLiteral("X-KDevelop-Language")
    };
    QList<IPlugin*> supports;
    for (const auto& key : keys) {
        QVariantMap constraints;
        constraints.insert(key, name);
        supports = Core::self()->pluginController()->allPluginsForExtension(KEY_ILanguageSupport(), constraints);
        if (key == keys[1]) {
            for (auto support : supports) {
                qWarning() << "Plugin" << Core::self()->pluginController()->pluginInfo(support).name() << " has deprecated (since 5.1) metadata key \"X-KDevelop-Language\", needs porting to: \"X-KDevelop-Languages\": ["<<name<<"]'";
            }
        }
        if (!supports.isEmpty()) {
            break;
        }
    }

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

    //Never use findByUrl from within a background thread, and never load a language support
    //from within the backgruond thread. Both is unsafe, and can lead to crashes
    if(!languages.isEmpty() || QThread::currentThread() != thread())
        return languages;

    QMimeType mimeType;

    if (url.isLocalFile()) {
        mimeType = QMimeDatabase().mimeTypeForFile(url.toLocalFile());
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
        constraints.insert(KEY_SupportedMimeTypes(), mimetype);
        QList<IPlugin*> supports = Core::self()->pluginController()->allPluginsForExtension(KEY_ILanguageSupport(), constraints);

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
