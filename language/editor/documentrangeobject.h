/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "../languageexport.h"

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
class KDEVPLATFORMLANGUAGE_EXPORT DocumentRangeObject : public KTextEditor::SmartRangeWatcher
{
public:
    /**
     * Constructor.
     *
     * @param document The document which this object is located in. May be empty, then it will not be changed (can come from a copy constructor)
     * @param range The range which this object occupies. May be invalid, then it is not changed (So it can come from a copy constructor)
     * */
    DocumentRangeObject(const HashedString& document, const SimpleRange& range);
    /// Destructor.
    virtual ~DocumentRangeObject();

    /// An enumeration of ownership for the smart range.
    enum RangeOwning{
        Own /**< this object owns the range */,
        DontOwn /**< this object does not own the range */
    };

    /**
     * Access the smart range of this object.
     *
     * \returns the SmartRange if the object has one, otherwise returns null
     */
    KTextEditor::SmartRange* smartRange() const;

    /**
     * Sets the text \a range to this object.  If \a ownsRange is false, the range won't be deleted when the object is deleted.
     * Does not change the url associated to this object, because that cannot be retrieved from the smart-range in a thread-safe way.
     *
     * \warning you need to hold the smart mutex in order to call this function from anywhere but a copy constructor.
     */
    void setSmartRange(KTextEditor::SmartRange* range, RangeOwning ownsRange = Own);

    /**
     * Determine whether this object owns its smart range.
     *
     * \returns true if the object owns its smart range, otherwise false.
     */
    RangeOwning ownsRange() const;

    /**
     * Sets whether this object owns its smart range, and should delete it when it is deleted.
     *
     * \param ownsRange whether this object owns its smart range.
     */
    void setRangeOwning(RangeOwning ownsRange);

    /**
     * Clear the smart range from this object.
     */
    void clearSmartRange();

    /**
     * Returns the text range of this object.
     */
    SimpleRange range() const;

    /**
     * Set the text range (non-smart range) for this object.
     *
     * \param range new text range
     */
    void setRange(const SimpleRange& range);

    /**
     * Returns the url of the document in which this object is located.
     *
     * This is a HashedString to allow for fast comparison. It was from a real url using KUrl::pathOrUrl() at some point.
     *
     * \returns the url of the document in which this object is located.
     */
    HashedString url() const;

    /**
     * Set the url of the document in which this object is located.
     *
     * \param document url of the document in which this object is located.
     */
    void setUrl(const HashedString& document);

    /**
     * Determine if this range contains the given \a cursor.
     *
     * \param cursor cursor to check if it is contained by this range.
     * \returns true if the cursor is contained by this range, otherwise false.
     */
    bool contains(const SimpleCursor& cursor) const;

    /**
     * Take the range from this object, and set its smart range to null.
     *
     * \warning USE WITH CARE!! You must be willing to
     * immediately delete this range object if you take its range.
     *
     * \returns this object's smart range
     */
    KTextEditor::SmartRange* takeRange();

protected:
    /// Static shared mutex protecting internal data.  May be used to protect private data in subclasses. \returns the internal mutex
    static QMutex* mutex();

    /**
     * Constructor for copy constructors in subclasses.
     *
     * \param dd data to copy.
     * \param ownsSmartRange set to true if this object owns the smart range and should delete it when this object is deleted, otherwise set to false (the default)
     */
    DocumentRangeObject(DocumentRangeObjectPrivate& dd, bool ownsSmartRange = true);

    /**
     * Constructor for copy constructors in subclasses.
     *
     * \param dd data to copy.
     * \param document document in which this object is located.
     * \param range text range which this object covers.
     */
    DocumentRangeObject(DocumentRangeObjectPrivate& dd, const HashedString& document, const SimpleRange& range);

    /// Private data pointer.
    DocumentRangeObjectPrivate* const d_ptr;

    /**
     * Reimplementation of KTextEditor::SmartWatcher::rangeDeleted(), to clean up the smart
     * range when it is deleted (for example, on file close).
     */
    virtual void rangeDeleted(KTextEditor::SmartRange* range);

private:
    bool m_ownsData;

    Q_DISABLE_COPY(DocumentRangeObject)
    Q_DECLARE_PRIVATE(DocumentRangeObject)
};

}
#endif // RANGEOBJECT_H


