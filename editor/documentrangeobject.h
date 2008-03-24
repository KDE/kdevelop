/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVDOCUMENTRANGEOBJECT_H
#define KDEVDOCUMENTRANGEOBJECT_H

#include <QtCore/QMutex>

#include <ktexteditor/range.h>
#include <ktexteditor/rangefeedback.h>

#include "editorexport.h"

#include "documentcursor.h"
#include "documentrange.h"
#include "simplecursor.h"
#include "simplerange.h"

namespace KDevelop
{
class HashedString;
class DocumentRangeObjectPrivate;
/**
 * Base class for any object which has an associated range of text.
 *
 * This allows text without a currently loaded text editor to be represented.
 */
class KDEVPLATFORMEDITOR_EXPORT DocumentRangeObject : public KTextEditor::SmartRangeWatcher
{
public:
    /**
     * @param range May be invalid, then it is not changed(So it can come from a copy constructor)
     * @param document May be emty, then it will not be changed(can come from a copy constructor)
     * */
    DocumentRangeObject(const HashedString& document, const SimpleRange& range);
    virtual ~DocumentRangeObject();

    enum RangeOwning{
        Own /**< this object owns the range */,
        DontOwn /**< this object does not own the range */
    };
    Q_DECLARE_FLAGS( RangeOwnings, RangeOwning )

    /**
     * Sets the text \a range to this object.  If \a ownsRange is false, the range won't be deleted when the object is deleted.
     * Does not change the url associated to this object, because that cannot be retrieved from the smart-range in a thread-safe way.
     */
    void setSmartRange(KTextEditor::SmartRange* range, RangeOwning ownsRange = Own);

    void setRangeOwning(RangeOwning ownsRange);

    void setRange(const SimpleRange& range);
    ///Returns the text-range. Needs to be a reference for performance-reasons.
    SimpleRange range() const;
    ///If this document's range is a SmartRange, returns it. Else 0.
    KTextEditor::SmartRange* smartRange() const;

    //static HashedString url(const KTextEditor::Range* cursor);

    RangeOwning ownsRange() const;

    void setUrl(const HashedString& document);
    ///Returns the url, for efficiency as a HashedString. This allows fast comparison. It is was from a real url using prettyUrl() at some point.
    HashedString url() const;

    bool contains(const SimpleCursor& cursor) const;
    //bool contains(const KTextEditor::Cursor& cursor) const;

    /// Take the range from this object. USE WITH CARE!! You must be willing to
    /// immediately delete this range object if you take its range.
    KTextEditor::SmartRange* takeRange();

protected:
    // Mutex protects all DocumentRangeObject internals from threading-conditioned corruption
    static QMutex m_mutex;

    DocumentRangeObject(DocumentRangeObjectPrivate& dd);
    DocumentRangeObject(DocumentRangeObjectPrivate& dd, const HashedString& document, const SimpleRange& range);
    
    DocumentRangeObjectPrivate* const d_ptr;
    
    virtual void rangeDeleted(KTextEditor::SmartRange* range);
private:

    Q_DISABLE_COPY(DocumentRangeObject)
    Q_DECLARE_PRIVATE(DocumentRangeObject)
};


Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::DocumentRangeObject::RangeOwnings )

}
#endif // RANGEOBJECT_H


