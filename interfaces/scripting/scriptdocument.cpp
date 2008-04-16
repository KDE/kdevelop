/***************************************************************************
 *   Copyright 2008 Harald Fernengel <harry@kdevelop.org>                  *
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

#include "idocument.h"
#include "scripttools.h"

#include "qobject.h"

namespace KDevelop
{

extern QObject *&getWrapper(IDocument *doc);
inline void setWrapper(IDocument *doc, QObject *wrapper)
{
    getWrapper(doc) = wrapper;
}

/* This is a scripting wrapper for IDocument - since it's not a QObject,
   we need to pass this wrapper whenever IDocument* is passed into a script.
   This class is not exported - only accessible through scripting backend
 */
class ScriptDocument : public QObject
{
    Q_OBJECT
    Q_ENUMS(DocumentSaveMode)

public:
    enum DocumentSaveMode
    {
        Default = KDevelop::IDocument::Default,
        Silent = KDevelop::IDocument::Silent
    };

    ScriptDocument(KDevelop::IDocument *document, QObject *parent = 0)
        : QObject(parent), doc(document)
    {
        Q_ASSERT(document);

        /* Register ourselves in the script wrapper, so we get deleted
           when the document dies */
        setWrapper(doc, this);
    }

    /* These are forwarded to the IDocument methods */
    Q_SCRIPTABLE inline KUrl url() const
    {
        return doc->url();
    }

    Q_SCRIPTABLE inline bool save(DocumentSaveMode mode = Default)
    {
        return doc->save(KDevelop::IDocument::DocumentSaveMode(mode));
    }

    Q_SCRIPTABLE inline void reload()
    {
        doc->reload();
    }

    Q_SCRIPTABLE inline void close()
    {
        doc->close();
    }

    Q_SCRIPTABLE inline bool isActive() const
    {
        return doc->isActive();
    }

    /* ### TODO - add the rest of IDocument here */

private:
    KDevelop::IDocument *doc;
};

/* convert between a IDocument pointer (from a signal or slot)
   to the script document wrapper
*/
QObject *ScriptTools::toDocument(const QVariant &variant)
{
    IDocument *doc = qVariantValue<IDocument *>(variant);

    if (!doc)
        // not a QVariant containing an IDocument*
        return 0;

    QObject *existingWrapper = getWrapper(doc);
    if (existingWrapper)
        // there's already a script wrapper - return that one
        return existingWrapper;

    // create and return a new ScriptDocument
    return new ScriptDocument(doc);
}

}

#include "scriptdocument.moc"
