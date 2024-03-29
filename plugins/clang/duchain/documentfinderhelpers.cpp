/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "documentfinderhelpers.h"

#include "duchain/clanghelpers.h"

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/duchainutils.h>

#include <KDesktopFile>

using namespace KDevelop;

namespace {

enum FileType {
    Unknown, ///< Doesn't belong to C++
    Header,  ///< Is a header file
    Source   ///< Is a C(++) file
};

class PotentialBuddyCollector : public DUChainUtils::DUChainItemFilter
{
public:
    enum BuddyMode {
        Header,
        Source
    };

    explicit PotentialBuddyCollector(BuddyMode mode)
     : mode(mode)
    {}

    bool accept(Declaration* decl) override
    {
        if (decl->range().isEmpty())
            return false;

        if (mode == Header && decl->isFunctionDeclaration()) {
            // Search for definitions of our declarations
            auto* def = FunctionDefinition::definition(decl);
            if (def) {
                vote(def->url().toUrl());
            }

            return true;
        }
        else if (mode == Source && decl->isFunctionDeclaration()) {
            auto* fdef = dynamic_cast<FunctionDefinition*>(decl);
            if (fdef) {
                Declaration* fdecl = fdef->declaration();
                if (fdecl) {
                    vote(fdecl->url().toUrl());
                }
            }

            return true;
        } else {
            return false;
        }
    }
    bool accept(DUContext* ctx) override
    {
        if (ctx->type() == DUContext::Class || ctx->type() == DUContext::Namespace || ctx->type() == DUContext::Global || ctx->type() == DUContext::Other || ctx->type() == DUContext::Helper ) {
            return true;
        } else {
            return false;
        }
    }

    QUrl bestBuddy() const
    {
        QUrl ret;
        int bestCount = 0;
        for (auto it = m_buddyFiles.begin(); it != m_buddyFiles.end(); ++it) {
            if(it.value() > bestCount) {
                bestCount = it.value();
                ret = it.key();
            }
        }

        return ret;
    }
private:
    BuddyMode mode;
    QHash<QUrl, int> m_buddyFiles;

    void vote(const QUrl& url)
    {
        m_buddyFiles[url]++;
    }
};

/**
 * Tries to find a buddy file to the given file by looking at the DUChain.
 *
 * The project might keep source files separate from headers. To cover
 * this situation, we examine DUChain for the most probable buddy file.
 * This of course only works if we have parsed the buddy file, but it is
 * better than nothing.
 *
 * @param url url of the source/header file to find a buddy for
 * @param type type of the file @p url
 *
 * @returns QUrl of the most probable buddy file, or an empty url
 **/
QUrl duchainBuddyFile(const QUrl& url, FileType type)
{
    DUChainReadLocker lock;

    auto ctx = DUChainUtils::standardContextForUrl(url);
    if (ctx) {
        PotentialBuddyCollector collector( type == Header ? PotentialBuddyCollector::Header : PotentialBuddyCollector::Source );
        DUChainUtils::collectItems(ctx, collector);

        return collector.bestBuddy();
    }

    return QUrl();
}

/**
 * Generates the base path (without extension) and the file type
 * for the specified url.
 *
 * @returns pair of base path and file type which has been found for @p url.
 */
QPair<QString,FileType> basePathAndTypeForUrl(const QUrl &url)
{
    QString path = url.toLocalFile();
    int idxSlash = path.lastIndexOf(QLatin1Char('/'));
    int idxDot = path.lastIndexOf(QLatin1Char('.'));
    FileType fileType = Unknown;
    QString basePath;
    if (idxSlash >= 0 && idxDot >= 0 && idxDot > idxSlash) {
        basePath = path.left(idxDot);
        if (idxDot + 1 < path.length()) {
            QString extension = path.mid(idxDot + 1);
            if (ClangHelpers::isHeader(extension)) {
                fileType = Header;
            } else if (ClangHelpers::isSource(extension)) {
                fileType = Source;
            }
        }
    } else {
        basePath = path;
    }

    return qMakePair(basePath, fileType);
}

}

namespace DocumentFinderHelpers {
QStringList mimeTypesList()
{
    static const QStringList mimeTypes = {
        QStringLiteral("text/x-chdr"),
        QStringLiteral("text/x-c++hdr"),
        QStringLiteral("text/vnd.nvidia.cuda.chdr"),
        QStringLiteral("text/x-csrc"),
        QStringLiteral("text/x-c++src"),
        QStringLiteral("text/vnd.nvidia.cuda.csrc"),
        QStringLiteral("text/x-objcsrc")
    };
    return mimeTypes;
}

bool areBuddies(const QUrl &url1, const QUrl& url2)
{
    auto type1 = basePathAndTypeForUrl(url1);
    auto type2 = basePathAndTypeForUrl(url2);

    QUrl headerPath;
    QUrl sourcePath;

    // Check that one file is a header, the other one is source
    if (type1.second == Header && type2.second == Source) {
        headerPath = url1;
        sourcePath = url2;
    } else if (type1.second == Source && type2.second == Header) {
        headerPath = url2;
        sourcePath = url1;
    } else {
        // Some other file constellation
        return false;
    }

    // The simplest directory layout is with header + source in one directory.
    // So check that first.
    if (type1.first == type2.first) {
        return true;
    }

    // Also check if the DUChain thinks this is likely
    if (duchainBuddyFile(sourcePath, Source) == headerPath) {
        return true;
    }

    return false;
}

bool buddyOrder(const QUrl &url1, const QUrl& url2)
{
    auto type1 = basePathAndTypeForUrl(url1);
    auto type2 = basePathAndTypeForUrl(url2);
    // Precondition is that the two URLs are buddies, so don't check it
    return(type1.second == Header && type2.second == Source);
}

QVector<QUrl> potentialBuddies(const QUrl& url, bool checkDUChain)
{
    auto type = basePathAndTypeForUrl(url);
    // Don't do anything for types we don't know
    if (type.second == Unknown) {
        return {};
    }

    // Depending on the buddy's file type we either generate source extensions (for headers)
    // or header extensions (for sources)
    const auto& extensions = ( type.second == Header ? ClangHelpers::sourceExtensions() : ClangHelpers::headerExtensions() );
    QVector< QUrl > buddies;
    buddies.reserve(extensions.size());
    for(const QString& extension : extensions) {
        if (!extension.contains(QLatin1Char('.'))) {
            buddies.append(QUrl::fromLocalFile(type.first + QLatin1Char('.') + extension));
        } else {
            buddies.append(QUrl::fromLocalFile(type.first + extension));
        }
    }

    if (checkDUChain) {
        // Also ask DUChain for a guess
        QUrl bestBuddy = duchainBuddyFile(url, type.second);
        if (!buddies.contains(bestBuddy)) {
            buddies.append(bestBuddy);
        }
    }

    return buddies;
}

QString sourceForHeader(const QString& headerPath)
{
    if (!ClangHelpers::isHeader(headerPath)) {
        return {};
    }

    QString targetUrl;
    const auto buddies = DocumentFinderHelpers::potentialBuddies(QUrl::fromLocalFile(headerPath));
    for (const auto& buddy : buddies) {
        const auto local = buddy.toLocalFile();
        if (QFileInfo::exists(local)) {
            targetUrl = local;
            break;
        }
    }

    return targetUrl;
}

}
