/*
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

#include <KDesktopFile>
#include <KConfigGroup>
#include <KMimeType>

#include <QSet>

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

/**
 * Generates the base path (without extension) and the file type
 * for the specified url.
 *
 * @returns pair of base path and file type which has been found for @p url.
 */
QPair<QString,FileType> basePathAndTypeForUrl(const KUrl& url)
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
    KDesktopFile desktopFile("services", QString("kdevclangsupport.desktop"));
    const KConfigGroup& desktopGroup = desktopFile.desktopGroup();
    QString mimeTypesStr = desktopGroup.readEntry("X-KDevelop-SupportedMimeTypes", "");
    return mimeTypesStr.split(QChar(','), QString::SkipEmptyParts);
}

bool DocumentFinderHelpers::areBuddies(const KUrl& url1, const KUrl& url2)
{
    auto type1 = basePathAndTypeForUrl(url1);
    auto type2 = basePathAndTypeForUrl(url2);
    return(type1.first == type2.first && ((type1.second == Header && type2.second == Source) ||
                                          (type1.second == Source && type2.second == Header)));
}

bool DocumentFinderHelpers::buddyOrder(const KUrl& url1, const KUrl& url2)
{
    auto type1 = basePathAndTypeForUrl(url1);
    auto type2 = basePathAndTypeForUrl(url2);
    // Precondition is that the two URLs are buddies, so don't check it
    return(type1.second == Header && type2.second == Source);
}

QVector< KUrl > DocumentFinderHelpers::getPotentialBuddies(const KUrl& url)
{
    auto type = basePathAndTypeForUrl(url);
    // Don't do anything for types we don't know
    if (type.second == Unknown) {
        return {};
    }

    // Depending on the buddy's file type we either generate source extensions (for headers)
    // or header extensions (for sources)
    const auto& extensions = ( type.second == Header ? getSourceFileExtensions() : getHeaderFileExtensions() );
    QVector< KUrl > buddies;
    for(const QString& extension : extensions) {
        buddies.append(KUrl(type.first + '.' + extension));
    }

    return buddies;
}
