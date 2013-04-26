/***************************************************************************
 *   Copyright 2011 Yannick Motta   <yannick.motta@gmail.com>              *
 *                  Martin Heide    <martin.heide@gmx.net>                 *
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
#ifndef KDEVPLATFORM_IBUDDYDOCUMENTFINDER_H
#define KDEVPLATFORM_IBUDDYDOCUMENTFINDER_H

#include <QObject>
#include <QVector>
#include <KUrl>
#include "interfacesexport.h"

namespace KDevelop {

/**
 * @short Implement this to add buddy document functionality to your language plugin.
 *
 * It enables the DocumentController (shell) to find related documents
 * (normally declaration/definition files, like foo.h and foo.cpp).
 * The DocumentController will position tabs of buddy documents next to
 * each other if the option UISettings.TabBarArrangeBuddies is 1.
 * The implementation also determines the order of the tabs in the tabbar.
 *
 * For finding a buddy document, the DocumentController addresses the
 * IBuddyDocumentFinder object that is registerd for this mimetype:
 *
 * This class provides static "registry" functions to handle a specific buddy finding
 * method (i.e. an object of class @class IBuddyDocumentFinder) for different
 * mimetypes. Like this, you can have for example an IBuddyDocumentFinder
 * for C++ files which considers foo.cpp and foo.h as buddies, and an
 * IBuddyDocumentFinder for Ada files which considers bar.adb and bar.ads as
 * buddies.
 * Like this, the concept of buddy documents is extensible for every language
 * for which support is added to KDevelop.
 *
 * If you want to add @class IBuddyDocumentFinder functionality to your language
 * plugin, your main class will inherit from IBuddyDocumentFinder (or have an
 * attribute of this type). Then in your constructor, call
 * @code addFinder(mimetype, this) @endcode
 * for each mimetype that your plugin supports. It is no problem to register the
 * same IBuddyDocumentFinder for several mimetypes.
 *
 * In the same way, in your destructor, you'll call
 * @code removeFinder(mimetype, this) @endcode
 * for each supported mimetype, to avoid that your IBuddyDocumentFinder object
 * is used beyond its lifetime.
 *
 * After you have registered it, your IBuddyDocumentFinder implementation will be found by
 * @code finderForMimeType(mimetype) @endcode
 * For example, the shell's DocumentController calls this function with idoc->mimeType()
 * in order to find a buddy document of a particular IDocument *idoc.
 */
class KDEVPLATFORMINTERFACES_EXPORT IBuddyDocumentFinder
{
public:
    virtual ~IBuddyDocumentFinder(){}

    /**
     * Called to determine if two document URLs should be considered as related.
     *
     * @return true, if the two documents are buddies.
     * For example, a C++ implementation would return true for
     * areBuddies(KUrl("...../foo.h"), KUrl("...../foo.cpp")).
     */
    virtual bool areBuddies(const KUrl& url1, const KUrl& url2) = 0;

    /**
     * Called to determine the order of two documents in the tabbar.
     *
     * Example: a C++ implementation that wants to place the tab of the .h
     * file left of the .cpp tab must return true for
     *   buddyOrder(KUrl("...../foo.h"),   KUrl("...../foo.cpp"))
     * and false for
     *   buddyOrder(KUrl("...../foo.cpp"), KUrl("...../foo.h")).
     *
     * @param url1 @param url2: two documents which are buddies,
     * this means areBuddies(url1,url2) returned true.
     * @return true, if url1's tab should be placed left of url2's tab.
     *         false, for the inverse.
     */
    virtual bool buddyOrder(const KUrl& url1, const KUrl& url2) = 0;


    /**
     * Returns a list of KUrls of potential buddies of the document
     * provided by @p url.
     *
     * The urls are potential buddies and it is not ensured that the files
     * really exist.
     *
     * @returns list of potential buddy documents or an empty list
     *  if non are available.
     */
    virtual QVector<KUrl> getPotentialBuddies(const KUrl& url) const = 0;

    /**
     * Registers a IBuddyDocumentFinder object for a mimetype.
     *
     * To be called in the constructor of language plugins.
     * Afterwards, finderForMimeType(@param mimeType) will return @param finder,
     * as long as the entry is not overwritten by another call to addFinder.
     */
    static void addFinder(QString mimeType, IBuddyDocumentFinder* finder);

    /**
     * Un-registers a IBuddyDocumentFinder object for a mimetype.
     *
     * To be called in the destructor of language plugins.
     * Afterwards, finderForMimeType(@param mimeType) will return 0, until a new
     * entry for this mimetype is created by addFinder().
     */
    static void removeFinder(QString mimeType);

    /**
     * Returns the registered IBuddyDocumentFinder for this mimetype, or 0.
     *
     * Used in the DocumentController (shell).
     */
    static IBuddyDocumentFinder* finderForMimeType(QString mimeType);

private:
    struct Private;
};

}

#endif
