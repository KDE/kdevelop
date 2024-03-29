/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchainbase.h"

#include <QMutexLocker>
#include <QThreadStorage>

#include "duchainpointer.h"
#include "parsingenvironment.h"
#include <serialization/indexedstring.h>
#include "topducontext.h"
#include "duchainregister.h"
#include <util/foregroundlock.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <backgroundparser/backgroundparser.h>
#include <backgroundparser/documentchangetracker.h>

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(DUChainBase);

uint DUChainBaseData::classSize() const
{
    return DUChainItemSystem::self().dataClassSize(*this);
}

DUChainBase::DUChainBase(const RangeInRevision& range)
    : d_ptr(new DUChainBaseData)
{
    d_func_dynamic()->m_range = range;
    d_func_dynamic()->setClassId(this);
}

DUChainBase::DUChainBase(DUChainBaseData& dd, const RangeInRevision& range)
    : d_ptr(&dd)
{
    d_func_dynamic()->m_range = range;
}

DUChainBase::DUChainBase(DUChainBaseData& dd)
    : d_ptr(&dd)
{
}

DUChainBase::DUChainBase(DUChainBase& rhs)
    : d_ptr(new DUChainBaseData(*rhs.d_func()))
{
    d_func_dynamic()->setClassId(this);
}

IndexedString DUChainBase::url() const
{
    TopDUContext* top = topContext();
    if (top)
        return top->TopDUContext::url();
    else
        return IndexedString();
}

void DUChainBase::setData(DUChainBaseData* data, bool constructorCalled)
{
    Q_ASSERT(data);
    Q_ASSERT(d_ptr);

    if (d_ptr->m_dynamic) {
        Q_ASSERT(constructorCalled);
        DUChainItemSystem::self().deleteDynamicData(d_ptr);
    } else if (constructorCalled) {
        // If the data object isn't dynamic, then it is part of a central repository, and cannot be deleted here.
        // we still need to call the destructor though
        KDevelop::DUChainItemSystem::self().callDestructor(static_cast<DUChainBaseData*>(d_ptr));
    }

    d_ptr = data;
}

DUChainBase::~DUChainBase()
{
    if (m_ptr)
        m_ptr->m_base = nullptr;

    if (d_ptr->m_dynamic) {
        DUChainItemSystem::self().deleteDynamicData(d_ptr);
        d_ptr = nullptr;
    }
}

TopDUContext* DUChainBase::topContext() const
{
    ///@todo Move the reference to the top-context right into this class, as it's common to all inheriters
    return nullptr;
}

namespace {
QMutex weakPointerMutex;
}

const QExplicitlySharedDataPointer<DUChainPointerData>& DUChainBase::weakPointer() const
{
    if (!m_ptr) {
        QMutexLocker lock(&weakPointerMutex); // The mutex is used to make sure we don't create m_ptr twice at the same time
        m_ptr = new DUChainPointerData(const_cast<DUChainBase*>(this));
        m_ptr->m_base = const_cast<DUChainBase*>(this);
    }

    return m_ptr;
}

void DUChainBase::rebuildDynamicData(DUContext* parent, uint ownIndex)
{
    Q_UNUSED(parent)
    Q_UNUSED(ownIndex)
}

void DUChainBase::makeDynamic()
{
    Q_ASSERT(d_ptr);
    if (!d_func()->m_dynamic) {
        Q_ASSERT(d_func()->classId);
        DUChainBaseData* newData = DUChainItemSystem::self().cloneData(*d_func());
        {
            auto* const baseData = static_cast<DUChainBaseData*>(d_ptr);
            const DUChainReferenceCountingEnabler rcEnabler(d_ptr, DUChainItemSystem::self().dynamicSize(*baseData));
            //We don't delete the previous data, because it's embedded in the top-context when it isn't dynamic.
            //However we do call the destructor, to keep semantic stuff like reference-counting within the data class working correctly.
            DUChainItemSystem::self().callDestructor(baseData);
        }
        d_ptr = newData;
        Q_ASSERT(d_ptr);
        Q_ASSERT(d_func()->m_dynamic);
        Q_ASSERT(d_func()->classId);
    }
}

RangeInRevision DUChainBase::range() const
{
    return d_func()->m_range;
}

KTextEditor::Range DUChainBase::rangeInCurrentRevision() const
{
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(url());

    if (tracker && topContext() && topContext()->parsingEnvironmentFile()) {
        qint64 revision = topContext()->parsingEnvironmentFile()->modificationRevision().revision;
        return tracker->transformToCurrentRevision(d_func()->m_range, revision);
    }

    // If the document is not open, we can simply cast the range over, as no translation can be done
    return d_func()->m_range.castToSimpleRange();
}

PersistentMovingRange::Ptr DUChainBase::createRangeMoving() const
{
    VERIFY_FOREGROUND_LOCKED
    return PersistentMovingRange::Ptr(new PersistentMovingRange(rangeInCurrentRevision(), url()));
}

CursorInRevision DUChainBase::transformToLocalRevision(const KTextEditor::Cursor& cursor) const
{
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(url());

    if (tracker && topContext() && topContext()->parsingEnvironmentFile()) {
        qint64 revision = topContext()->parsingEnvironmentFile()->modificationRevision().revision;
        return tracker->transformToRevision(cursor, revision);
    }

    return CursorInRevision::castFromSimpleCursor(cursor);
}

RangeInRevision DUChainBase::transformToLocalRevision(const KTextEditor::Range& range) const
{
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(url());

    if (tracker && topContext() && topContext()->parsingEnvironmentFile()) {
        qint64 revision = topContext()->parsingEnvironmentFile()->modificationRevision().revision;
        return tracker->transformToRevision(range, revision);
    }

    return RangeInRevision::castFromSimpleRange(range);
}

KTextEditor::Range DUChainBase::transformFromLocalRevision(const KDevelop::RangeInRevision& range) const
{
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(url());

    if (tracker && topContext() && topContext()->parsingEnvironmentFile()) {
        qint64 revision = topContext()->parsingEnvironmentFile()->modificationRevision().revision;
        return tracker->transformToCurrentRevision(range, revision);
    }

    return range.castToSimpleRange();
}

KTextEditor::Cursor DUChainBase::transformFromLocalRevision(const KDevelop::CursorInRevision& cursor) const
{
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(url());

    if (tracker && topContext() && topContext()->parsingEnvironmentFile()) {
        qint64 revision = topContext()->parsingEnvironmentFile()->modificationRevision().revision;
        return tracker->transformToCurrentRevision(cursor, revision);
    }

    return cursor.castToSimpleCursor();
}

void DUChainBase::setRange(const RangeInRevision& range)
{
    d_func_dynamic()->m_range = range;
}

QThreadStorage<bool> shouldCreateConstantDataStorage;

bool& DUChainBaseData::shouldCreateConstantData()
{
    return shouldCreateConstantDataStorage.localData();
}
}
