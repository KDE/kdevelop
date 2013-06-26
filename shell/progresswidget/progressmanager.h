/***************************************************************************
 *   (C) 2004 Till Adam <adam@kde.org>                                  *
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

#ifndef KDEVPLATFORM_PROGRESSMANAGER_H
#define KDEVPLATFORM_PROGRESSMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QHash>
#include <QWeakPointer>
#include <QPointer>
namespace Akonadi {
class AgentInstance;
}

namespace KDevelop {

class ProgressItem;
class ProgressManager;
typedef QMap<ProgressItem *, bool> ProgressItemMap;

class ProgressItem : public QObject
{
    Q_OBJECT
    friend class ProgressManager;

public:

    /**
     * @return The id string which uniquely identifies the operation
     *         represented by this item.
     */
    const QString &id() const { return mId; }

    /**
     * @return The parent item of this one, if there is one.
     */
    ProgressItem *parent() const { return mParent.data(); }

    /**
     * @return The user visible string to be used to represent this item.
     */
    const QString &label() const { return mLabel; }

    /**
     * @param v Set the user visible string identifying this item.
     */
    void setLabel( const QString &v );

    /**
     * @return The string to be used for showing this item's current status.
     */
    const QString &status() const { return mStatus; }
    /**
     * Set the string to be used for showing this item's current status.
     * @param v The status string.
     */
    void setStatus( const QString &v );

    /**
     * @return Whether this item can be canceled.
     */
    bool canBeCanceled() const { return mCanBeCanceled; }

    /**
     * @return Whether this item uses secure communication
     * (Account uses ssl, for example.).
     */
    bool usesCrypto() const { return mUsesCrypto; }

    /**
     * Set whether this item uses crypted communication, so listeners
     * can display a nice crypto icon.
     * @param v The value.
     */
    void setUsesCrypto( bool v );

    /**
     * @return whether this item uses a busy indicator instead of real progress display
     */
    bool usesBusyIndicator() const { return mUsesBusyIndicator; }

    /**
     * Sets whether this item uses a busy indicator instead of real progress for its progress bar.
     * If it uses a busy indicator, you are still responsible for calling setProgress() from time to
     * time to update the busy indicator.
     */
    void setUsesBusyIndicator( bool useBusyIndicator );

    /**
     * @return The current progress value of this item in percent.
     */
    unsigned int progress() const { return mProgress; }

    /**
     * Set the progress (percentage of completion) value of this item.
     * @param v The percentage value.
     */
    void setProgress( unsigned int v );

    /**
     * Tell the item it has finished. This will emit progressItemCompleted()
     * result in the destruction of the item after all slots connected to this
     * signal have executed. This is the only way to get rid of an item and
     * needs to be called even if the item is canceled. Don't use the item
     * after this has been called on it.
     */
    void setComplete();

    /**
     * Reset the progress value of this item to 0 and the status string to
     * the empty string.
     */
    void reset()
    {
        setProgress( 0 );
        setStatus( QString() );
        mCompleted = 0;
    }

    void cancel();

    // Often needed values for calculating progress.
    void setTotalItems( unsigned int v ) { mTotal = v; }
    unsigned int totalItems() const { return mTotal; }
    void setCompletedItems( unsigned int v ) { mCompleted = v; }
    void incCompletedItems( unsigned int v = 1 ) { mCompleted += v; }
    unsigned int completedItems() const { return mCompleted; }

    /**
     * Recalculate progress according to total/completed items and update.
     */
    void updateProgress()
    {
        setProgress( mTotal? mCompleted * 100 / mTotal : 0 );
    }

    void addChild( ProgressItem *kiddo );
    void removeChild( ProgressItem *kiddo );

    bool canceled() const { return mCanceled; }
    void setBusy( bool busy );

Q_SIGNALS:
    /**
     * Emitted when a new ProgressItem is added.
     * @param The ProgressItem that was added.
     */
    void progressItemAdded( KDevelop::ProgressItem * );

    /**
     * Emitted when the progress value of an item changes.
     * @param  The item which got a new value.
     * @param  The value, for convenience.
     */
    void progressItemProgress( KDevelop::ProgressItem *, unsigned int );

    /**
     * Emitted when a progress item was completed. The item will be
     * deleted afterwards, so slots connected to this are the last
     * chance to work with this item.
     * @param The completed item.
     */
    void progressItemCompleted( KDevelop::ProgressItem * );

    /**
     * Emitted when an item was canceled. It will _not_ go away immediately,
     * only when the owner sets it complete, which will usually happen. Can be
     * used to visually indicate the canceled status of an item. Should be used
     * by the owner of the item to make sure it is set completed even if it is
     * canceled. There is a ProgressManager::slotStandardCancelHandler which
     * simply sets the item completed and can be used if no other work needs to
     * be done on cancel.
     * @param The canceled item;
     */
    void progressItemCanceled( KDevelop::ProgressItem * );

    /**
     * Emitted when the status message of an item changed. Should be used by
     * progress dialogs to update the status message for an item.
     * @param  The updated item.
     * @param  The new message.
     */
    void progressItemStatus( KDevelop::ProgressItem *, const QString & );

    /**
     * Emitted when the label of an item changed. Should be used by
     * progress dialogs to update the label of an item.
     * @param  The updated item.
     * @param  The new label.
     */
    void progressItemLabel( KDevelop::ProgressItem *, const QString & );

    /**
     * Emitted when the crypto status of an item changed. Should be used by
     * progress dialogs to update the crypto indicator of an item.
     * @param  The updated item.
     * @param  The new state.
     */
    void progressItemUsesCrypto( KDevelop::ProgressItem *, bool );

    /**
     * Emitted when the busy indicator state of an item changes. Should be used
     * by progress dialogs so that they can adjust the display of the progress bar
     * to the new mode.
     * @param item The updated item
     * @param value True if the item uses a busy indicator now, false otherwise
     */
    void progressItemUsesBusyIndicator( KDevelop::ProgressItem *item, bool value );

protected:
    /* Only to be used by our good friend the ProgressManager */
    ProgressItem( ProgressItem *parent, const QString &id, const QString &label,
                  const QString &status, bool isCancellable, bool usesCrypto );
    virtual ~ProgressItem();

private:
    QString mId;
    QString mLabel;
    QString mStatus;
    QWeakPointer<ProgressItem>mParent;
    bool mCanBeCanceled;
    unsigned int mProgress;
    ProgressItemMap mChildren;
    unsigned int mTotal;
    unsigned int mCompleted;
    bool mWaitingForKids;
    bool mCanceled;
    bool mUsesCrypto;
    bool mUsesBusyIndicator;
    bool mCompletedCalled;
};

struct ProgressManagerPrivate;

/**
 * The ProgressManager singleton keeps track of all ongoing transactions
 * and notifies observers (progress dialogs) when their progress percent value
 * changes, when they are completed (by their owner), and when they are canceled.
 * Each ProgressItem emits those signals individually and the singleton
 * broadcasts them. Use the ::createProgressItem() statics to acquire an item
 * and then call ->setProgress( int percent ) on it every time you want to
 * update the item and ->setComplete() when the operation is done. This will
 * delete the item. Connect to the item's progressItemCanceled() signal to be
 * notified when the user cancels the transaction using one of the observing
 * progress dialogs or by calling item->cancel() in some other way. The owner
 * is responsible for calling setComplete() on the item, even if it is canceled.
 * Use the standardCancelHandler() slot if that is all you want to do on cancel.
 *
 * Note that if you request an item with a certain id and there is already
 * one with that id, there will not be a new one created but the existing
 * one will be returned. This is convenient for accessing items that are
 * needed regularly without the to store a pointer to them or to add child
 * items to parents by id.
 */
class ProgressManager : public QObject
{

    Q_OBJECT

    friend struct ProgressManagerPrivate;

public:
    virtual ~ProgressManager();

    /**
     * @return The singleton instance of this class.
     */
    static ProgressManager *instance();

    /**
     * Use this to acquire a unique id number which can be used to discern
     * an operation from all others going on at the same time. Use that
     * number as the id string for your progressItem to ensure it is unique.
     * @return
     */
    static QString getUniqueID()
    {
        return QString::number( ++uID );
    }

    /**
      * Creates a ProgressItem with a unique id and the given label.
      * This is the simplest way to acquire a progress item. It will not
      * have a parent and will be set to be cancellable and not using crypto.
      */
    static ProgressItem *createProgressItem( const QString &label )
    {
        return instance()->createProgressItemImpl( 0, getUniqueID(), label,
                                                   QString(), true, false );
    }

    /**
     * Creates a new progressItem with the given parent, id, label and initial
     * status.
     *
     * @param parent Specify an already existing item as the parent of this one.
     * @param id Used to identify this operation for cancel and progress info.
     * @param label The text to be displayed by progress handlers
     * @param status Additional text to be displayed for the item.
     * @param canBeCanceled can the user cancel this operation?
     * @param usesCrypto does the operation use secure transports (SSL)
     * Cancelling the parent will cancel the children as well (if they can be
     * canceled) and ongoing children prevent parents from finishing.
     * @return The ProgressItem representing the operation.
     */
    static ProgressItem *createProgressItem( ProgressItem *parent,
                                             const QString &id,
                                             const QString &label,
                                             const QString &status = QString(),
                                             bool canBeCanceled = true,
                                             bool usesCrypto = false )
    {
        return instance()->createProgressItemImpl( parent, id, label, status,
                                                   canBeCanceled, usesCrypto );
    }

    /**
     * Use this version if you have the id string of the parent and want to
     * add a subjob to it.
     */
    static ProgressItem *createProgressItem( const QString &parent,
                                             const QString &id,
                                             const QString &label,
                                             const QString &status = QString(),
                                             bool canBeCanceled = true,
                                             bool usesCrypto = false )
    {
        return instance()->createProgressItemImpl( parent, id, label,
                                                   status, canBeCanceled, usesCrypto );
    }

    /**
     * Version without a parent.
     */
    static ProgressItem *createProgressItem( const QString &id,
                                             const QString &label,
                                             const QString &status = QString(),
                                             bool canBeCanceled = true,
                                             bool usesCrypto = false )
    {
        return instance()->createProgressItemImpl( 0, id, label, status,
                                                   canBeCanceled, usesCrypto );
    }

    /**
     * Version for Akonadi agents.
     * This connects all the proper signals so that you do not have to
     * worry about updating the progress or reacting to progressItemCanceled().
     */
    static ProgressItem *createProgressItem( ProgressItem *parent,
                                             const Akonadi::AgentInstance &agent,
                                             const QString &id,
                                             const QString &label,
                                             const QString &status = QString(),
                                             bool canBeCanceled = true,
                                             bool usesCrypto = false )
    {
        return instance()->createProgressItemForAgent( parent, agent, id, label,
                                                       status, canBeCanceled, usesCrypto );
    }

    /**
     * @return true when there are no more progress items.
     */
    bool isEmpty() const
    {
        return mTransactions.isEmpty();
    }

    /**
     * @return the only top level progressitem when there's only one.
     * Returns 0 if there is no item, or more than one top level item.
     * Since this is used to calculate the overall progress, it will also return
     * 0 if there is an item which uses a busy indicator, since that will invalidate
     * the overall progress.
     */
    ProgressItem *singleItem() const;

    /**
     * Ask all listeners to show the progress dialog, because there is
     * something that wants to be shown.
     */
    static void emitShowProgressDialog()
    {
        instance()->emitShowProgressDialogImpl();
    }

Q_SIGNALS:
    /** @see ProgressItem::progressItemAdded() */
    void progressItemAdded( KDevelop::ProgressItem * );
    /** @see ProgressItem::progressItemProgress() */
    void progressItemProgress( KDevelop::ProgressItem *, unsigned int );
    /** @see ProgressItem::progressItemCompleted() */
    void progressItemCompleted( KDevelop::ProgressItem * );
    /** @see ProgressItem::progressItemCanceled() */
    void progressItemCanceled( KDevelop::ProgressItem * );
    /** @see ProgressItem::progressItemStatus() */
    void progressItemStatus( KDevelop::ProgressItem *, const QString & );
    /** @see ProgressItem::progressItemLabel() */
    void progressItemLabel( KDevelop::ProgressItem *, const QString & );
    /** @see ProgressItem::progressItemUsesCrypto() */
    void progressItemUsesCrypto( KDevelop::ProgressItem *, bool );
    /** @see ProgressItem::progressItemUsesBusyIndicator */
    void progressItemUsesBusyIndicator( KDevelop::ProgressItem*, bool );

    /**
     * Emitted when an operation requests the listeners to be shown.
     * Use emitShowProgressDialog() to trigger it.
     */
    void showProgressDialog();

public Q_SLOTS:

    /**
     * Calls setCompleted() on the item, to make sure it goes away.
     * Provided for convenience.
     * @param item the canceled item.
     */
    void slotStandardCancelHandler( KDevelop::ProgressItem *item );

    /**
     * Aborts all running jobs. Bound to "Esc"
     */
    void slotAbortAll();

private Q_SLOTS:
    void slotTransactionCompleted( KDevelop::ProgressItem *item );

private:
    ProgressManager();
    // prevent unsolicited copies
    ProgressManager( const ProgressManager & );

    virtual ProgressItem *createProgressItemImpl( ProgressItem *parent,
                                                  const QString &id,
                                                  const QString &label,
                                                  const QString &status,
                                                  bool cancellable,
                                                  bool usesCrypto );
    virtual ProgressItem *createProgressItemImpl( const QString &parent,
                                                  const QString &id,
                                                  const QString &label,
                                                  const QString &status,
                                                  bool cancellable,
                                                  bool usesCrypto );
    ProgressItem *createProgressItemForAgent( ProgressItem *parent,
                                              const Akonadi::AgentInstance &instance,
                                              const QString &id,
                                              const QString &label,
                                              const QString &status,
                                              bool cancellable,
                                              bool usesCrypto );
    void emitShowProgressDialogImpl();

    QHash< QString, ProgressItem* > mTransactions;
    static unsigned int uID;
};

}

#endif // __KDevelop_PROGRESSMANAGER_H__
