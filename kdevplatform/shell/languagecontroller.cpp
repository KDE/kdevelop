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

namespace KDevelop {


using LanguageHash = QHash<QString, ILanguageSupport*>;
using LanguageCache = QHash<QString, QList<ILanguageSupport*>>;

class LanguageControllerPrivate
{
public:
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

        const QList<ILanguageSupport*> languages = m_controller->languagesForUrl(url);
        activeLanguages.reserve(languages.size());
        for (const auto lang : languages) {
            activeLanguages << lang;
        }
    }

    QList<ILanguageSupport*> activeLanguages;

    mutable QMutex dataMutex;

    LanguageHash languages; //Maps language-names to languages
    LanguageCache languageCache; //Maps mimetype-names to languages
    using MimeTypeCache = QMultiHash<QMimeType, ILanguageSupport*>;
    MimeTypeCache mimeTypeCache; //Maps mimetypes to languages

    BackgroundParser* const backgroundParser;
    StaticAssistantsManager* staticAssistantsManager;
    bool m_cleanedUp;

    void addLanguageSupport(ILanguageSupport* support, const QStringList& mimetypes);
    void addLanguageSupport(ILanguageSupport* support);

    ProblemModelSet* const problemModelSet;

private:
    LanguageController* const m_controller;
};

void LanguageControllerPrivate::addLanguageSupport(ILanguageSupport* languageSupport,
                                                            const QStringList& mimetypes)
{
    Q_ASSERT(!languages.contains(languageSupport->name()));
    languages.insert(languageSupport->name(), languageSupport);

    for (const QString& mimeTypeName : mimetypes) {
        qCDebug(SHELL) << "adding supported mimetype:" << mimeTypeName << "language:" << languageSupport->name();
        languageCache[mimeTypeName] << languageSupport;
        QMimeType mime = QMimeDatabase().mimeTypeForName(mimeTypeName);
        if (mime.isValid()) {
            mimeTypeCache.insert(mime, languageSupport);
        } else {
            qCWarning(SHELL) << "could not create mime-type" << mimeTypeName;
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
    , d_ptr(new LanguageControllerPrivate(this))
{
    setObjectName(QStringLiteral("LanguageController"));
}

LanguageController::~LanguageController() = default;

void LanguageController::initialize()
{
    Q_D(LanguageController);

    d->backgroundParser->loadSettings();
    d->staticAssistantsManager = new StaticAssistantsManager(this);

    // make sure the DUChain is setup before we try to access it from different threads at the same time
    DUChain::self();

    connect(Core::self()->documentController(), &IDocumentController::documentActivated,
            this, [this] (IDocument* document) { Q_D(LanguageController); d->documentActivated(document); });
}

void LanguageController::cleanup()
{
    Q_D(LanguageController);

    QMutexLocker lock(&d->dataMutex);
    d->m_cleanedUp = true;
}

QList<ILanguageSupport*> LanguageController::activeLanguages()
{
    Q_D(LanguageController);

    QMutexLocker lock(&d->dataMutex);

    return d->activeLanguages;
}

StaticAssistantsManager* LanguageController::staticAssistantsManager() const
{
    Q_D(const LanguageController);

    return d->staticAssistantsManager;
}

ICompletionSettings *LanguageController::completionSettings() const
{
    return &CompletionSettings::self();
}

ProblemModelSet* LanguageController::problemModelSet() const
{
    Q_D(const LanguageController);

    return d->problemModelSet;
}

QList<ILanguageSupport*> LanguageController::loadedLanguages() const
{
    Q_D(const LanguageController);

    QMutexLocker lock(&d->dataMutex);
    QList<ILanguageSupport*> ret;

    if(d->m_cleanedUp)
        return ret;

    ret.reserve(d->languages.size());
    for (ILanguageSupport* lang : qAsConst(d->languages)) {
        ret << lang;
    }
    return ret;
}

ILanguageSupport* LanguageController::language(const QString &name) const
{
    Q_D(const LanguageController);

    QMutexLocker lock(&d->dataMutex);

    if(d->m_cleanedUp)
        return nullptr;

    const auto languageIt = d->languages.constFind(name);
    if (languageIt != d->languages.constEnd())
        return *languageIt;

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
            for (auto support : qAsConst(supports)) {
                qCWarning(SHELL) << "Plugin" << Core::self()->pluginController()->pluginInfo(support).name() << " has deprecated (since 5.1) metadata key \"X-KDevelop-Language\", needs porting to: \"X-KDevelop-Languages\": ["<<name<<"]'";
            }
        }
        if (!supports.isEmpty()) {
            break;
        }
    }

    if(!supports.isEmpty()) {
        auto *languageSupport = supports[0]->extension<ILanguageSupport>();
        if(languageSupport) {
            const_cast<LanguageControllerPrivate*>(d)->addLanguageSupport(languageSupport);
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
    Q_D(LanguageController);

    QMutexLocker lock(&d->dataMutex);

    QList<ILanguageSupport*> languages;

    if(d->m_cleanedUp)
        return languages;

    const QString fileName = url.fileName();

    ///TODO: cache regexp or simple string pattern for endsWith matching
    QRegExp exp(QString(), Qt::CaseInsensitive, QRegExp::Wildcard);
    ///non-crashy part: Use the mime-types of known languages
    for(LanguageControllerPrivate::MimeTypeCache::const_iterator it = d->mimeTypeCache.constBegin();
        it != d->mimeTypeCache.constEnd(); ++it)
    {
        const auto globPatterns = it.key().globPatterns();
        for (const QString& pattern : globPatterns) {
            if(pattern.startsWith(QLatin1Char('*'))) {
                const QStringRef subPattern = pattern.midRef(1);
                if (!subPattern.contains(QLatin1Char('*'))) {
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
    Q_D(LanguageController);

    QMutexLocker lock(&d->dataMutex);

    QList<ILanguageSupport*> languages;
    LanguageCache::ConstIterator it = d->languageCache.constFind(mimetype);
    if (it != d->languageCache.constEnd()) {
        languages = it.value();
    } else {
        QVariantMap constraints;
        constraints.insert(KEY_SupportedMimeTypes(), mimetype);
        const QList<IPlugin*> supports = Core::self()->pluginController()->allPluginsForExtension(KEY_ILanguageSupport(), constraints);

        if (supports.isEmpty()) {
            qCDebug(SHELL) << "no languages for mimetype:" << mimetype;
            d->languageCache.insert(mimetype, QList<ILanguageSupport*>());
        } else {
            for (IPlugin *support : supports) {
                auto* languageSupport = support->extension<ILanguageSupport>();
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
    Q_D(LanguageController);

    QMutexLocker lock(&d->dataMutex);

    QList<QString> mimetypes;
    for (LanguageCache::ConstIterator iter = d->languageCache.constBegin(); iter != d->languageCache.constEnd(); ++iter) {
        bool isFromLanguage = std::any_of(iter.value().begin(), iter.value().end(), [&] (ILanguageSupport* language ) {
            return (language->name() == languageName);
        });
        if (isFromLanguage) {
            mimetypes << iter.key();
        }
    }
    return mimetypes;
}

BackgroundParser *LanguageController::backgroundParser() const
{
    Q_D(const LanguageController);

    return d->backgroundParser;
}

void LanguageController::addLanguageSupport(ILanguageSupport* languageSupport, const QStringList& mimetypes)
{
    Q_D(LanguageController);

    d->addLanguageSupport(languageSupport, mimetypes);
}

}

#include "moc_languagecontroller.cpp"
