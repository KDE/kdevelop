/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "languagecontroller.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <QHash>
#include <QMimeDatabase>
#include <QMultiHash>
#include <QMutexLocker>
#include <QRecursiveMutex>
#include <QRegularExpression>
#include <QThread>

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

bool containsWildcardCharacter(QString::const_iterator first, QString::const_iterator last)
{
    const auto isWildcardCharacter = [](QChar character) {
        const auto u = character.unicode();
        return u == '*' || u == '?' || u == '[';
    };
    return std::any_of(first, last, isWildcardCharacter);
}

using KDevelop::ILanguageSupport;

class MimeTypeCache
{
public:
    void addMimeType(const QString& mimeTypeName, ILanguageSupport* language);
    QList<ILanguageSupport*> languagesForFileName(const QString& fileName) const;

private:
    void addGlobPattern(const QString& pattern, ILanguageSupport* language);

    using StringLanguagePair = std::pair<QString, ILanguageSupport*>;

    /// key() is the lower-cased last character of the suffix; value().first is e.g. ".cpp".
    /// The last character key performs better than the entire lower-cased (last) extension key. Perhaps
    /// because no lower-cased temporary fileName's extension string is created in languagesForFileName().
    QMultiHash<QChar, StringLanguagePair> m_suffixes;
    std::vector<StringLanguagePair> m_literalPatterns; ///< contains e.g. "CMakeLists.txt" as front().first
    /// fallback, hopefully empty in practice
    std::vector<std::pair<QRegularExpression, ILanguageSupport*>> m_regularExpressions;
};

void MimeTypeCache::addMimeType(const QString& mimeTypeName, ILanguageSupport* language)
{
    const QMimeType mime = QMimeDatabase().mimeTypeForName(mimeTypeName);
    if (!mime.isValid()) {
        qCWarning(SHELL) << "could not create mime-type" << mimeTypeName;
        return;
    }

    const auto globPatterns = mime.globPatterns();
    for (const QString& pattern : globPatterns) {
        addGlobPattern(pattern, language);
    }
}

QList<ILanguageSupport*> MimeTypeCache::languagesForFileName(const QString& fileName) const
{
    if (fileName.isEmpty()) {
        // The file name of a remote URL that ends with a slash is empty, but such a URL can still reference a file.
        // An empty file name cannot match a MIME type.
        return {};
    }

    QList<ILanguageSupport*> languages;
    // lastLanguageEquals() helps to improve performance by skipping checks for an already added language.
    const auto lastLanguageEquals = [&languages](const ILanguageSupport* lang) {
        return !languages.empty() && languages.constLast() == lang;
    };

    const auto lastChar = fileName.back().toLower();
    for (auto it = m_suffixes.constFind(lastChar); it != m_suffixes.cend() && it.key() == lastChar; ++it) {
        const auto lang = it.value().second;
        if (!lastLanguageEquals(lang) && fileName.endsWith(it.value().first, Qt::CaseInsensitive)) {
            languages.push_back(lang);
        }
    }

    for (const auto& p : m_literalPatterns) {
        if (fileName.compare(p.first, Qt::CaseInsensitive) == 0 && !lastLanguageEquals(p.second)) {
            languages.push_back(p.second);
        }
    }

    for (const auto& p : m_regularExpressions) {
        if (!lastLanguageEquals(p.second) && p.first.match(fileName).hasMatch()) {
            languages.push_back(p.second);
        }
    }

    return languages;
}

void MimeTypeCache::addGlobPattern(const QString& pattern, ILanguageSupport* language)
{
    if (pattern.isEmpty()) {
        qCWarning(SHELL) << "Attempt to add an invalid empty glob pattern.";
        return; // An empty pattern won't match a filename.
    }

    if (pattern.front() == QLatin1Char{'*'}) {
        if (pattern.size() > 1 && !containsWildcardCharacter(pattern.cbegin() + 1, pattern.cend())) {
            const auto lastChar = pattern.back().toLower();
            StringLanguagePair suffix{pattern.mid(1).toLower(), language};
            if (!m_suffixes.contains(lastChar, suffix)) {
                m_suffixes.insert(lastChar, std::move(suffix));
            }
            return;
        }
    } else if (!containsWildcardCharacter(pattern.cbegin(), pattern.cend())) {
        m_literalPatterns.emplace_back(pattern, language);
        return;
    }

    QRegularExpression regularExpression(QRegularExpression::wildcardToRegularExpression(pattern),
                                         QRegularExpression::CaseInsensitiveOption);
    m_regularExpressions.emplace_back(std::move(regularExpression), language);
}

} // namespace

namespace KDevelop {


using LanguageHash = QHash<QString, ILanguageSupport*>;
using LanguageCache = QHash<QString, QList<ILanguageSupport*>>;

class LanguageControllerPrivate
{
public:
    explicit LanguageControllerPrivate(LanguageController *controller)
        : backgroundParser(new BackgroundParser(controller))
        , staticAssistantsManager(nullptr)
        , m_cleanedUp(false)
        , problemModelSet(new ProblemModelSet(controller))
    {}

    mutable QRecursiveMutex dataMutex;

    LanguageHash languages; //Maps language-names to languages
    LanguageCache languageCache; //Maps mimetype-names to languages
    MimeTypeCache mimeTypeCache; //Maps mimetype-glob-patterns to languages

    BackgroundParser* const backgroundParser;
    StaticAssistantsManager* staticAssistantsManager;
    bool m_cleanedUp;

    void addLanguageSupport(ILanguageSupport* support, const QStringList& mimetypes);
    void addLanguageSupport(ILanguageSupport* support);

    ProblemModelSet* const problemModelSet;
};

void LanguageControllerPrivate::addLanguageSupport(ILanguageSupport* languageSupport,
                                                            const QStringList& mimetypes)
{
    Q_ASSERT(!languages.contains(languageSupport->name()));
    languages.insert(languageSupport->name(), languageSupport);

    for (const QString& mimeTypeName : mimetypes) {
        qCDebug(SHELL) << "adding supported mimetype:" << mimeTypeName << "language:" << languageSupport->name();
        languageCache[mimeTypeName] << languageSupport;
        mimeTypeCache.addMimeType(mimeTypeName, languageSupport);
    }
}

void LanguageControllerPrivate::addLanguageSupport(KDevelop::ILanguageSupport* languageSupport)
{
    if (languages.contains(languageSupport->name()))
        return;

    Q_ASSERT(dynamic_cast<IPlugin*>(languageSupport));

    KPluginMetaData info = Core::self()->pluginController()->pluginInfo(dynamic_cast<IPlugin*>(languageSupport));
    const auto mimetypes = info.value(KEY_SupportedMimeTypes(), QStringList());
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

    d->languages = {};
    d->languageCache = {};
    d->mimeTypeCache = {};

    d->backgroundParser->loadSettings();

    delete d->staticAssistantsManager;
    d->staticAssistantsManager = new StaticAssistantsManager(this);

    d->m_cleanedUp = false;

    // make sure the DUChain is setup before we try to access it from different threads at the same time
    DUChain::self();
}

void LanguageController::cleanup()
{
    Q_D(LanguageController);

    QMutexLocker lock(&d->dataMutex);
    d->m_cleanedUp = true;
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
    for (ILanguageSupport* lang : std::as_const(d->languages)) {
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
            for (auto support : std::as_const(supports)) {
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

QList<ILanguageSupport*> LanguageController::languagesForUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        // The URL of an unsaved document is empty.
        // The code below cannot find a language for an empty URL.
        return {};
    }

    Q_D(LanguageController);

    QMutexLocker lock(&d->dataMutex);

    if(d->m_cleanedUp)
        return {};

    ///non-crashy part: Use the mime-types of known languages
    auto languages = d->mimeTypeCache.languagesForFileName(url.fileName());

    //Never use findByUrl from within a background thread, and never load a language support
    //from within the backgruond thread. Both is unsafe, and can lead to crashes
    if(!languages.isEmpty() || QThread::currentThread() != thread())
        return languages;

    const auto mimeType = QMimeDatabase().mimeTypeForUrl(url);
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
