/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "documentfinderhelpers.h"

#include "duchain/clangparsingenvironment.h"

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/duchainutils.h>

#include <KDesktopFile>
#include <KConfigGroup>
#include <KMimeType>

#include <QSet>
#include <QStandardPaths>

using namespace KDevelop;

/**
 * @return all extensions which match the given @p mimeType.
 */
QSet<QString> getExtensionsByMimeType(const QString& mimeType)
{
    auto ptr = KMimeType::mimeType(mimeType);

    if (!ptr) {
        return {};
    }

    QSet<QString> extensions;
    foreach(const QString& pattern, ptr->patterns()) {
        if (pattern.startsWith("*.")) {
            extensions << pattern.mid(2);
        }
    }

    return extensions;
}

QSet<QString> getHeaderFileExtensions()
{
    return getExtensionsByMimeType("text/x-c++hdr") | getExtensionsByMimeType("text/x-chdr");
}

QSet<QString> getSourceFileExtensions()
{
    return getExtensionsByMimeType("text/x-c++src") | getExtensionsByMimeType("text/x-csrc");
}

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

    virtual bool accept(Declaration* decl)
    {
        if (decl->range().isEmpty())
            return false;

        if (mode == Header && decl->isFunctionDeclaration()) {
            // Search for definitions of our declarations
            FunctionDefinition* def = FunctionDefinition::definition(decl);
            if (def) {
                vote(def->url().toUrl());
            }

            return true;
        }
        else if (mode == Source && decl->isFunctionDeclaration()) {
            FunctionDefinition* fdef = dynamic_cast<FunctionDefinition*>(decl);
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
    virtual bool accept(DUContext* ctx)
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
    int idxSlash = path.lastIndexOf('/');
    int idxDot = path.lastIndexOf('.');
    FileType fileType = Unknown;
    QString basePath;
    if (idxSlash >= 0 && idxDot >= 0 && idxDot > idxSlash) {
        basePath = path.left(idxDot);
        if (idxDot + 1 < path.length()) {
            QString extension = path.mid(idxDot + 1);
            if (getHeaderFileExtensions().contains(extension)) {
                fileType = Header;
            } else if (getSourceFileExtensions().contains(extension)) {
                fileType = Source;
            }
        }
    } else {
        basePath = path;
    }

    return qMakePair(basePath, fileType);
}

QStringList DocumentFinderHelpers::mimeTypesList()
{
    KDesktopFile desktopFile(QStandardPaths::GenericDataLocation, QString("kservices5/kdevclangsupport.desktop"));
    const KConfigGroup& desktopGroup = desktopFile.desktopGroup();
    QString mimeTypesStr = desktopGroup.readEntry("X-KDevelop-SupportedMimeTypes", "");
    return mimeTypesStr.split(QChar(','), QString::SkipEmptyParts);
}

bool DocumentFinderHelpers::areBuddies(const QUrl &url1, const QUrl& url2)
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

bool DocumentFinderHelpers::buddyOrder(const QUrl &url1, const QUrl& url2)
{
    auto type1 = basePathAndTypeForUrl(url1);
    auto type2 = basePathAndTypeForUrl(url2);
    // Precondition is that the two URLs are buddies, so don't check it
    return(type1.second == Header && type2.second == Source);
}

QVector< QUrl > DocumentFinderHelpers::getPotentialBuddies(const QUrl &url)
{
    auto type = basePathAndTypeForUrl(url);
    // Don't do anything for types we don't know
    if (type.second == Unknown) {
        return {};
    }

    // Depending on the buddy's file type we either generate source extensions (for headers)
    // or header extensions (for sources)
    const auto& extensions = ( type.second == Header ? getSourceFileExtensions() : getHeaderFileExtensions() );
    QVector< QUrl > buddies;
    for(const QString& extension : extensions) {
        buddies.append(QUrl::fromLocalFile(type.first + '.' + extension));
    }

    // Also ask DUChain for a guess
    QUrl bestBuddy = duchainBuddyFile(url, type.second);
    if (!bestBuddy.isEmpty()) {
        buddies.append(bestBuddy);
    }

    return buddies;
}
