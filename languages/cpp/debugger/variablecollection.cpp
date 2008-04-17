/*
 * GDB Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
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

#include "variablecollection.h"
#include "tooltipwidget.h"

#include "gdbcontroller.h"
#include "variableitem.h"
#include "frameitem.h"
#include "watchitem.h"
#include "tooltipwidget.h"

#include "qt4/qstringvariableitem.h"
#include "qt4/qlistvariableitem.h"

#include "mi/gdbmi.h"
#include "gdbcommand.h"

#include "../stringhelpers.h"

#include "icore.h"
#include "idocumentcontroller.h"
#include "iuicontroller.h"
#include "sublime/controller.h"
#include "sublime/view.h"

#include <KLocale>
#include <KDebug>
#include <KTextEditor/TextHintInterface>
#include <KTextEditor/Document>
#include <KParts/PartManager>


#include <QFont>


//#include "modeltest.h"

using namespace GDBDebugger;

Variable::Variable(TreeModel* model, TreeItem* parent, 
                   GDBController* controller, const QString& expression)
: TreeItem(model, parent), controller_(controller), activeCommands_(0)
{
    expression_ = expression;
    // FIXME: should not duplicate the data, instead overload 'data'
    // and return expression_ directly.
    setData(QVector<QString>() << expression << "");
}

Variable::Variable(TreeModel* model, TreeItem* parent, 
                   GDBController* controller,
                   const GDBMI::Value& r)
: TreeItem(model, parent), controller_(controller), activeCommands_(0)
{
    varobj_ = r["name"].literal();
    itemData.push_back(r["exp"].literal());
    itemData.push_back(r["value"].literal());
    setHasMoreInitial(r["numchild"].toInt());
    allVariables_[varobj_] = this;
}

void Variable::handleCreation(const GDBMI::Value& r)
{
    varobj_ = r["name"].literal();
    setHasMore(r["numchild"].toInt());
    itemData[1] = r["value"].literal();
    allVariables_[varobj_] = this;
}

Variable::~Variable()
{
    if (!varobj_.isEmpty())
        allVariables_.remove(varobj_);
}

void Variable::createVarobjMaybe()
{
    if (!varobj_.isEmpty())
        return;

    if (!controller_->stateIsOn(s_appNotStarted))
    {
        controller_->addCommand(
            new GDBCommand(
                GDBMI::VarCreate, 
                QString("var%1 @ %2").arg(nextId_++).arg(expression_),
                this, &Variable::handleCreated, true));
    }
}

void Variable::update(const GDBMI::Value& value)
{
    Q_ASSERT(!value.hasField("type_changed")
             || value["type_changed"].literal() == "false");
    itemData[1] = value["value"].literal();
    reportChange();
}

void Variable::fetchMoreChildren()
{
    // FIXME: should not even try this if app is not started.
    // Probably need to disable open, or something
    if (!controller_->stateIsOn(s_appNotStarted))
    {
        activeCommands_ = 1;
        controller_->addCommand(
            new GDBCommand(GDBMI::VarListChildren,
                           QString("--all-values %1").arg(varobj_),
                           this,
                           // FIXME: handle error?
                           &Variable::handleChildren, this));
    }
}

void Variable::handleCreated(const GDBMI::ResultRecord &r)
{
    if (r.reason == "error")
    {
        /* Probably should mark this disabled, or something.  */        
    }
    else
    {
        handleCreation(r);
        reportChange();
    }
}

void Variable::handleChildren(const GDBMI::ResultRecord &r)
{
    --activeCommands_;
    const GDBMI::Value& children = r["children"];
    for (int i = 0; i < children.size(); ++i)
    {
        const GDBMI::Value& child = children[i];
        const QString& exp = child["exp"].literal();
        if (exp == "public" || exp == "protected" || exp == "private")
        {
            ++activeCommands_;
            controller_->addCommand(
                new GDBCommand(GDBMI::VarListChildren,
                               QString("--all-values %1")
                               .arg(child["name"].literal()),
                               this,
                               // FIXME: handle error?
                               &Variable::handleChildren, this));
        }
        else
        {
            Variable* var = new Variable(model(), this, controller_,
                                         child);
            appendChild(var);
        }
    }

    setHasMore(activeCommands_ != 0);
}

Variable* Variable::findByName(const QString& name)
{
    return allVariables_[name];
}

void Variable::markAllDead()
{
    QMap<QString, Variable*>::iterator i, e;
    for (i = allVariables_.begin(), e = allVariables_.end(); i != e; ++i)
    {
        i.value()->varobj_.clear();
    }
    allVariables_.clear();
}

int Variable::nextId_ = 0;

QMap<QString, Variable*> Variable::allVariables_;

Watches::Watches(TreeModel* model, TreeItem* parent)
: TreeItem(model, parent)
{
    setData(QVector<QString>() << "Auto" << "");
}

Variable* Watches::add(const QString& expression)
{
    Variable* v = new Variable(model(), this,
                               controller(), expression);
    appendChild(v);
    v->createVarobjMaybe();
    return v;
}

GDBController* Watches::controller()
{
    return static_cast<VariablesRoot*>(parent())->controller();
}


QVariant Watches::data(int column, int role) const
{
#if 0
    if (column == 0 && role == Qt::FontRole)
    {
        /* FIXME: is creating font again and agian efficient? */
        QFont f = font();
        f.setBold(true);
        return f;
    }
#endif
    return TreeItem::data(column, role);
}

void Watches::reinstall()
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        Variable* v = static_cast<Variable*>(child(i));
        v->createVarobjMaybe();
    }
}

VariablesRoot::VariablesRoot(TreeModel* model)
: TreeItem(model)
{
    watches_ = new Watches(model, this);
    appendChild(watches_, true);
}

GDBController* VariablesRoot::controller()
{
    return static_cast<VariableCollection*>(model())->controller();
}

VariableCollection::VariableCollection(GDBController* parent)
: TreeModel(QVector<QString>() << "Name" << "Value", parent),
  controller_(parent)
{
    universe_ = new VariablesRoot(this);
    setRootItem(universe_);

    //new ModelTest(this);

    foreach(KParts::Part* p, KDevelop::ICore::self()->partManager()->parts())
        slotPartAdded(p);
    connect(KDevelop::ICore::self()->partManager(),
            SIGNAL(partAdded(KParts::Part*)),
            this,
            SLOT(slotPartAdded(KParts::Part*)));
}



GDBController* VariableCollection::controller()
{
    return controller_;
}


VariableCollection::~ VariableCollection()
{
}

void VariableCollection::slotPartAdded(KParts::Part* part)
{
    if (KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>(part))
        foreach (KTextEditor::View* v, doc->views())
            slotViewAdded(v);
}

void VariableCollection::slotViewAdded(KTextEditor::View* view)
{
    using namespace KTextEditor;
    TextHintInterface *iface = dynamic_cast<TextHintInterface*>(view);
    if( !iface )
        return;

    iface->enableTextHints(500);

    connect(view, 
            SIGNAL(needTextHint(const KTextEditor::Cursor&, QString&)),
            this, 
            SLOT(textHintRequested(const KTextEditor::Cursor&, QString&)));    
}

void VariableCollection::
textHintRequested(const KTextEditor::Cursor& cursor, QString&)
{
    // Don't do anything if there's already an open tooltip.
    if (activeTooltip_)
        return;

    if (controller_->stateIsOn(s_appNotStarted))
        return;

    // Figure what is the parent widget and what is the text to show    
    KTextEditor::View* view = dynamic_cast<KTextEditor::View*>(sender());
    if (!view)
        return;

    KTextEditor::Document* doc = view->document();
    QString line = doc->line(cursor.line());
    int index = cursor.column();
    QChar c = line[index];
    if (!c.isLetterOrNumber() && c != '_')
        return;

    int start = Utils::expressionAt(line, index);
    int end = index;
    for (; end < line.size(); ++end)
    {
        QChar c = line[end];
        if (!(c.isLetterOrNumber() || c == '_'))
            break;
    }
    if (!(start < end))
        return;

    QString expression(line.mid(start, end-start));
    expression = expression.trimmed();

    kDebug(9012) << "expression " << expression << "\n";
    


#if 0
    QChar current = doc->character(c);    


    KTextEditor::Cursor c = cursor;
    bool moved = false;
    for (;;)
    {
        QChar current = doc->character(c);
        kDebug(9012) << "backward scan " << c;
        if (current.isLetterOrNumber() || current == '_')
        {
            if (c.column() == 0)
                break;
            else {
                c.setColumn(c.column() - 1);
                moved = true;
            }
        }       
        else {
            if (moved)
                c.setColumn(c.column() + 1);
            break;
        }
    }
    QString identifier;
    for (;;)
    {
        QChar current = doc->character(c);      
        kDebug(9012) << "forward scan " << c;
        if (current.isLetterOrNumber() || current == '_')
        {
            identifier.append(current);
            /* We hope that when we run out of the end of the line,
               Kate will return some sufficiently broken character.  */
            c.setColumn(c.column() + 1);
        }
        else
            break;
    }

    kDebug(9012) << "The identifier is: " << identifier;
#endif

    if (expression.isEmpty())
        return;

    QPoint local = view->cursorToCoordinate(cursor);
    QPoint global = view->mapToGlobal(local);
    QWidget* w = view->childAt(local);
    if (!w)
        w = view;
    
    activeTooltip_ = new VariableToolTip(w, global, controller_, expression);
}

void VariableCollection::slotEvent(event_t event)
{
    switch(event)
    {
        case program_exited:
        case debugger_exited:
        {
            Variable::markAllDead();
#if 0
            // Remove all locals.
            foreach (AbstractVariableItem* item, m_items) {
                // only remove frame items
                if (qobject_cast<FrameItem*>(item))
                {
                    deleteItem(item);
                }
                else
                {
                    item->deregisterWithGdb();
                }
            }
#endif
            break;
        }

        case connected_to_program:
            watches()->reinstall();
            break;

        case program_state_changed:

            // Fall-through intended.

        case thread_or_frame_changed:

            update();

            #if 0
            {
                FrameItem *frame = currentFrame();

                frame->setDirty();
            }
            #endif
            break;

        default:
            break;
    }
}

void VariableCollection::update()
{
    controller()->addCommand(
        new GDBCommand(GDBMI::VarUpdate, "--all-values *", this,
                       &VariableCollection::handleVarUpdate));
}

void VariableCollection::handleVarUpdate(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& changed = r["changelist"];
    for (int i = 0; i < changed.size(); ++i)
    {
        const GDBMI::Value& var = changed[i];
        Variable* v = Variable::findByName(var["name"].literal());
        v->update(var);        
    }
}

#if 0
void VariableCollection::addItem(AbstractVariableItem * item)
{
    item->registerWithGdb();

    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items.append(item);
    endInsertRows();
}

void VariableCollection::deleteItem(AbstractVariableItem * item)
{
    int index = m_items.indexOf(item);
    Q_ASSERT(index != -1);
    if (index == -1)
        return;

    item->deleteAllChildren();

    beginRemoveRows(QModelIndex(), index, index);
    delete m_items.takeAt(index);
    endRemoveRows();

#if 0
    if (item->isRegisteredWithGdb())
        item->deregisterWithGdb();
#endif
}

GDBController * VariableCollection::controller() const
{
    return static_cast<GDBController*>(const_cast<QObject*>(QObject::parent()));
}

int VariableCollection::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_items.count();

    if (parent.column() != 0)
        return 0;

    AbstractVariableItem* item = itemForIndex(parent);
    if (!item)
        return 0;

    return item->children().count();
}

QModelIndex VariableCollection::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || column > AbstractVariableItem::ColumnLast)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row >= m_items.count())
            return QModelIndex();

        return createIndex(row, column, m_items.at(row));
    }

    if (parent.column() != 0)
        return QModelIndex();

    AbstractVariableItem* item = itemForIndex(parent);
    if (item && row < item->children().count())
        return createIndex(row, column, item->children().at(row));

    return QModelIndex();
}

QModelIndex VariableCollection::indexForItem(AbstractVariableItem * item, int column) const
{
    if (!item)
        return QModelIndex();

    if (AbstractVariableItem* parent = item->abstractParent()) {
        int row = parent->children().indexOf(item);
        if (row == -1)
            return QModelIndex();

        return createIndex(row, column, item);
    }

    int row = m_items.indexOf(item);
    if (row == -1)
        return QModelIndex();

    return createIndex(row, column, item);
}

AbstractVariableItem * VariableCollection::itemForIndex(const QModelIndex & index) const
{
    return static_cast<AbstractVariableItem*>(index.internalPointer());
}

AbstractVariableItem * VariableCollection::parentForIndex(const QModelIndex & index) const
{
    if (AbstractVariableItem* item = itemForIndex(index))
        return item->abstractParent();

    return 0;
}

int VariableCollection::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return AbstractVariableItem::ColumnLast + 1;
}

QVariant VariableCollection::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (AbstractVariableItem* item = itemForIndex(index))
        return item->data(index.column(), role);

    return "<error - no item>";
}

Qt::ItemFlags VariableCollection::flags(const QModelIndex & index) const
{
    if (AbstractVariableItem* item = itemForIndex(index))
        return item->flags(index.column());

    return 0;
}

QVariant VariableCollection::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    switch (role) {
        case Qt::DisplayRole:
            switch (section) {
                case AbstractVariableItem::ColumnName:
                    return i18n("Variable");

                case AbstractVariableItem::ColumnValue:
                    return i18n("Value");

                case AbstractVariableItem::ColumnType:
                    return i18n("Type");
            }
            break;
    }

    return QVariant();
}

QModelIndex VariableCollection::parent(const QModelIndex & index) const
{
    AbstractVariableItem* parent = parentForIndex(index);
    if (!parent)
        return QModelIndex();

    AbstractVariableItem* grandParent = parent->abstractParent();
    if (!grandParent)
        return createIndex(m_items.indexOf(parent), 0, parent);

    return createIndex(grandParent->children().indexOf(parent), 0, parent);
}

bool VariableCollection::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.column() != AbstractVariableItem::ColumnName)
        return false;

    VariableItem* item = qobject_cast<VariableItem*>(itemForIndex(index));
    if (!item)
        return false;

    if (role == Qt::EditRole) {
        item->setVariableName(value.toString());
        return true;

    } else {
        kWarning() << "Unsupported set data role" << role;
    }

    return false;
}

void VariableCollection::slotAddWatchVariable(const QString &watchVar)
{
    // FIXME need thread +/- frame info??
    VariableItem *varItem = new VariableItem(findWatch());
    varItem->setExpression(watchVar);
    findWatch()->addChild(varItem);
}

void VariableCollection::slotEvaluateExpression(const QString &expression)
{
    if (!recentExpressions_)
    {
        recentExpressions_ = new RecentItem(this);
        addItem(recentExpressions_);
    }

    // FIXME need thread +/- frame info??
    VariableItem *varItem = new VariableItem(recentExpressions_);
    varItem->setExpression(expression);
    varItem->setFrozen();
    addItem(varItem);
}

FrameItem *VariableCollection::findFrame(int frameNo, int threadNo) const
{
    foreach (AbstractVariableItem* item, m_items)
        if (FrameItem* frame = qobject_cast<FrameItem*>(item))
            if (frame->matchDetails(frameNo, threadNo))
                return frame;

    return 0;
}

WatchItem *VariableCollection::findWatch()
{
    foreach (AbstractVariableItem* item, m_items)
        if (WatchItem* watch = qobject_cast<WatchItem*>(item))
            return watch;

    WatchItem* item = new WatchItem(this);
    addItem(item);
    return item;
}

void VariableCollection::prepareInsertItems(AbstractVariableItem * parent, int index, int count)
{
    beginInsertRows(indexForItem(parent), index, index + count - 1);
}

void VariableCollection::completeInsertItems()
{
    endInsertRows();
}

void VariableCollection::prepareRemoveItems(AbstractVariableItem * parent, int index, int count)
{
    beginRemoveRows(indexForItem(parent), index, index + count - 1);
}

void VariableCollection::completeRemoveItems()
{
    endRemoveRows();
}

void VariableCollection::dataChanged(AbstractVariableItem * item, int column)
{
    QModelIndex index = indexForItem(item, column);
    if (index.isValid())
        emit QAbstractItemModel::dataChanged(index, index);
}

AbstractVariableItem * VariableCollection::itemForVariableObject(const QString & variableObject) const
{
    if (m_variableObjectMap.contains(variableObject))
        return m_variableObjectMap[variableObject];

    return 0;
}

void VariableCollection::addVariableObject(const QString & variableObject, AbstractVariableItem * item)
{
    m_variableObjectMap.insert(variableObject, item);
}

void VariableCollection::removeVariableObject(const QString & variableObject)
{
    m_variableObjectMap.remove(variableObject);
}

FrameItem* VariableCollection::currentFrame()
{
    FrameItem* frame = findFrame(controller()->currentFrame(), controller()->currentThread());
    if (!frame)
    {
        frame = new FrameItem(this);
        frame->setThread(controller()->currentThread());
        frame->setFrame(controller()->currentFrame());
        addItem(frame);
    }
    return frame;
}

bool VariableCollection::canFetchMore(const QModelIndex & parent) const
{
    if (AbstractVariableItem* item = itemForIndex(parent))
        if (item->isChildrenDirty() && item->hasChildren())
            return true;

    return false;
}

void VariableCollection::fetchMore(const QModelIndex & parent)
{
    if (AbstractVariableItem* item = itemForIndex(parent))
        item->refreshChildren();
}

bool VariableCollection::hasChildren(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_items.count();

    if (parent.column() != 0)
        return false;

    if (AbstractVariableItem* item = itemForIndex(parent))
        return item->hasChildren();

    // Shouldn't hit this
    Q_ASSERT(false);
    return false;
}

VariableItem* VariableCollection::createVariableItem(const QString & type, AbstractVariableItem * parent)
{
    static QRegExp qstring("^(const)?[ ]*QString[ ]*&?$");
    if (qstring.exactMatch(type))
        return new QStringVariableItem(parent);

    static QRegExp qlist("^(const)?[ ]*QList.*");
    if (qlist.exactMatch(type))
        return new QListVariableItem(parent);

    return new VariableItem(parent);
}

#endif

#include "variablecollection.moc"
