/*
 * KDevelop Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEV_VARIABLECOLLECTION_H
#define KDEV_VARIABLECOLLECTION_H

#include <QAbstractItemModel>
#include <QMap>
#include <QPointer>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "../util/treemodel.h"
#include "../util/treeitem.h"
#include "../../interfaces/idocument.h"
#include "../debuggerexport.h"
#include "../interfaces/idebugsession.h"

namespace KDevelop
{
class VariableToolTip;

class KDEVPLATFORMDEBUGGER_EXPORT Variable : public TreeItem
{
public:
    Variable(TreeModel* model, TreeItem* parent,
             const QString& expression,
             const QString& display = "");

    QString varobj() const;
    QString expression() const;
    void setInScope(bool v);
    void setVarobj(const QString& v);
    void setValue(const QString &v);
    void setTopLevel(bool v);
    using TreeItem::setHasMore;
    using TreeItem::setHasMoreInitial;
    using TreeItem::clear;
    using TreeItem::appendChild;

    using TreeItem::model;

    ~Variable();

    void createVarobjMaybe(QObject *callback = 0, const char *callbackMethod = 0);

    void die();

    void fetchMoreChildren();

    static Variable *findByName(const QString& name);
    /* Called when GDB dies.  Clears the association between varobj names
       and Variable instances.  */
    static void markAllDead();


private: // TreeItem overrides

    QVariant data(int column, int role) const;

private:

    QString expression_;
    QString varobj_;
    bool inScope_;
    bool topLevel_;

    static QMap<QString, Variable*> allVariables_;
};

class KDEVPLATFORMDEBUGGER_EXPORT TooltipRoot : public TreeItem
{
public:
    Variable* var;

    TooltipRoot(TreeModel* model)
    : TreeItem(model)
    {}

    void init(const QString& expression)
    {
        var = new Variable(model(), this, expression);
        appendChild(var);
    }

    void fetchMoreChildren() {}
};

class KDEVPLATFORMDEBUGGER_EXPORT Watches : public TreeItem
{
public:
    Watches(TreeModel* model, TreeItem* parent);
    Variable* add(const QString& expression);

    void reinstall();

    Variable *addFinishResult(const QString& convenienceVarible);
    void removeFinishResult();

private:
    QVariant data(int column, int role) const;

    void fetchMoreChildren() {}

    Variable* finishResult_;
};

class KDEVPLATFORMDEBUGGER_EXPORT Locals : public TreeItem
{
public:
    Locals(TreeModel* model, TreeItem* parent);
    void updateLocals(QStringList locals);
    using TreeItem::clear;

private:
    void fetchMoreChildren() {}
};

class KDEVPLATFORMDEBUGGER_EXPORT VariablesRoot : public TreeItem
{
public:
    VariablesRoot(TreeModel* model);

    Watches *watches() const { return watches_; }
    Locals *locals() const { return locals_; }

    void fetchMoreChildren() {}

private:
    Watches *watches_;
    Locals *locals_;
};

class KDEVPLATFORMDEBUGGER_EXPORT VariableCollection : public TreeModel
{
    Q_OBJECT
#if 0
    friend class AbstractVariableItem;
    friend class VariableItem;
#endif

public:
    VariableCollection(QObject* parent);
    virtual ~VariableCollection();

    VariablesRoot* root() const { return universe_; }
    Watches* watches() const { return universe_->watches(); }
    Locals* locals() const { return universe_->locals(); }

#if 0
    void addItem(AbstractVariableItem* item);
    void deleteItem(AbstractVariableItem* item);

    // Item model reimplementations
    virtual bool canFetchMore ( const QModelIndex & parent ) const;
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual void fetchMore ( const QModelIndex & parent );
    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
    virtual bool hasChildren( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex & index ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    AbstractVariableItem* itemForIndex(const QModelIndex& index) const;

    FrameItem* findFrame(int frameNo, int threadNo) const;
    WatchItem* findWatch();

    AbstractVariableItem* itemForVariableObject(const QString& variableObject) const;
    void addVariableObject(const QString& variableObject, AbstractVariableItem* item);
    void removeVariableObject(const QString& variableObject);

    VariableItem* createVariableItem(const QString& type, AbstractVariableItem* parent);



Q_SIGNALS:
    void toggleWatchpoint(const QString &varName);

public Q_SLOTS:
    void slotAddWatchVariable(const QString& watchVar);
    void slotEvaluateExpression(const QString& expression);

    void slotEvent(event_t);

protected:
    // Methods for AbstractVariableItem to access
    void prepareInsertItems(AbstractVariableItem* parent, int index, int count);
    void completeInsertItems();
    void prepareRemoveItems(AbstractVariableItem* parent, int index, int count);
    void completeRemoveItems();
    void dataChanged(AbstractVariableItem* item, int column);

private:
    // Callbacks for gdb commands
    void frameIdReady(const QStringList&);
    void handleVarUpdate(const GDBMI::ResultRecord&);
    void handleEvaluateExpression(const QStringList&);
    void variablesFetchDone();
    void fetchSpecialValuesDone();

    /** This is called when address of expression for which
        popup is created is known.

        If there's no address (for rvalue), does nothing
        (leaving "Data breakpoint" item disabled).
        Otherwise, enabled that item, and check is we
        have data breakpoint for that address already.
    */
    void handleAddressComputed(const GDBMI::ResultRecord& r);

private:
    /** Get (if exists) and create (otherwise) frame root for
        the specified frameNo/threadNo combination.
    */
    FrameItem* currentFrame();
    void updateCurrentFrame();

    AbstractVariableItem* parentForIndex(const QModelIndex& index) const;
    QModelIndex indexForItem(AbstractVariableItem* item, int column = 0) const;

    int activeFlag_;
    int iOutRadix;
    bool justPaused_;

    // Root of all recently printed expressions.
    AbstractVariableItem* recentExpressions_;
#endif


private slots:
    void textDocumentCreated( KDevelop::IDocument*);
    void viewCreated(KTextEditor::Document*, KTextEditor::View*);
    void textHintRequested(const KTextEditor::Cursor&, QString&);

private:
    VariablesRoot* universe_;
    QPointer<VariableToolTip> activeTooltip_;

#if 0
    QMap<QString, AbstractVariableItem*> m_variableObjectMap;

    QList<AbstractVariableItem*> m_items;
#endif
};

}

#endif
