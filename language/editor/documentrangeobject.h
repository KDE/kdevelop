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
#include "../duchain/appendedlist.h"

namespace KDevelop
{
class HashedString;

///Contains data that is stored to disk
class KDEVPLATFORMLANGUAGE_EXPORT DocumentRangeObjectData
{
    public:
    DocumentRangeObjectData() {
      initializeAppendedLists();
    }
    ~DocumentRangeObjectData() {
      freeAppendedLists();
    }
    DocumentRangeObjectData(const DocumentRangeObjectData& rhs);

    mutable SimpleRange m_range; //Mutable for synchronization

    APPENDED_LISTS_STUB(DocumentRangeObjectData)

    bool isDynamic() const {
      return m_dynamic;
    }

    ///Returns whether initialized objects should be created as dynamic objects
    static bool appendedListDynamicDefault();

    uint classSize() const;
};

class DocumentRangeObjectDynamicPrivate;
/**
 * Base class for any object which has an associated range of text.
 *
 * This allows text without a currently loaded text editor to be represented.
 *
 * \todo Silently synchronising from smart ranges to return the range is not a good idea,
 *       because as soon as the user receives the range, it may be out of date already.
 *       Better to force them to understand that the smart mutex must be locked and thus
 *       retrieve a useful range to start with.
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
    DocumentRangeObject(const SimpleRange& range);
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

    ///Sets a new data pointer. The data will be owned by this object if it is dynamic.
    ///The old data is deleted if it was dynamic.
    ///@param constructorCalled Whether the constructor of the data object was called while its creation.
    ///                         If not, it means that the data is only moved into another place
    virtual void setData(DocumentRangeObjectData*, bool constructorCalled = true);

protected:
    /// Static shared mutex protecting internal data.  May be used to protect private data in subclasses. \returns the internal mutex
    static QMutex* mutex();

    /**
     * Constructor for copy constructors in subclasses.
     * This constructor should be used when sharing the data from another object without owning it.
     * This object will use that data, but will not destroy it on destruction.
     * This object must be destroyed before the object given through @param useDataFrom is destroyed!
     *
     * \param dd data to copy.
     * \param ownsData set to true if this object owns the private data including the smart range and should delete it when this object is deleted, otherwise set to false (the default)
     */
    DocumentRangeObject(DocumentRangeObject& useDataFrom);

    /**
     * Constructor for copy constructors in subclasses.
     *
     * \param dd data to copy.
     * \param document document in which this object is located.
     * \param range text range which this object covers.
     */
    DocumentRangeObject(DocumentRangeObjectData& dd, const SimpleRange& range = SimpleRange::invalid());

    /// Data pointer shared across the inheritance hierarchy
    DocumentRangeObjectData* d_ptr;

    /**
     * Reimplementation of KTextEditor::SmartWatcher::rangeDeleted(), to clean up the smart
     * range when it is deleted (for example, on file close).
     */
    virtual void rangeDeleted(KTextEditor::SmartRange* range);

    //Called before the data in d_ptr is written by DocumentRangeObject
    virtual void aboutToWriteData();

    //Called to test whether the data is allowed to be written automatically(from syncFromSmart)
    virtual bool canWriteData() const;
    
private:
    //Only really synchronized the smart-range if this object is writable eg. canWriteData() returns true.
    //Always returns the correct current range
    SimpleRange syncFromSmart() const;
    void syncToSmart() const;

    class DocumentRangeObjectDynamicPrivate* dd_ptr;
    bool m_ownsData;
};

}
#endif // RANGEOBJECT_H


