/***************************************************************************
 *   Copyright (C) 2002-2004 by Alexander Dymo                             *
 *   cloudtemple@mskat.net                                                 *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "propertyeditor.h"

#ifndef PURE_QT
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#else
#include "compat_tools.h"
#endif

#include <q3table.h>
#include <qlayout.h>
#include <qpainter.h>
#include <q3ptrlist.h>
#include <q3valuelist.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>

#include "property.h"
#include "multiproperty.h"
#include "propertymachinefactory.h"

namespace PropertyLib{

class PropertyItem: public KListViewItem{
public:
    PropertyItem(PropertyEditor *parent, MultiProperty *property)
        :KListViewItem(parent, property->description()), m_editor(parent), m_property(property),
        m_changed(false)
    {
    }

    PropertyItem(PropertyEditor *editor, KListViewItem *parent, MultiProperty *property)
        :KListViewItem(parent, property->description()), m_editor(editor),
        m_property(property), m_changed(false)
    {
    }

/*    int type() const
    {
        return m_property->type();
    }

    QString name() const
    {
        return m_property->name();
    }
        */
    MultiProperty *property() const
    {
        return m_property;
    }

    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
    {
        if ((column == 0) && m_changed)
        {
            QFont font;
            font.setBold(true);
            p->setFont(font);
            p->setBrush(cg.highlight());
            p->setPen(cg.highlightedText());
        }
        if (column == 1)
        {
            QRect r(0, 0, m_editor->header()->sectionSize(1), height());
            //FIXME: this is ugly, but how else can we deal with ValueFromList properties?
            QVariant valueToDraw;
            if (m_property->type() == Property::ValueFromList)
                valueToDraw = m_property->findValueDescription();
            else
                valueToDraw = m_property->value();
            QColorGroup icg(cg);
#ifndef PURE_QT
            icg.setColor(QColorGroup::Background, backgroundColor());
#else
            icg.setColor(QColorGroup::Background, Qt::white);
#endif
            m_editor->machine(m_property)->propertyEditor->drawViewer(p, icg, r, valueToDraw);
            return;
        }
        KListViewItem::paintCell(p, cg, column, width, align);
    }

    virtual void setup()
    {
        KListViewItem::setup();
        setHeight(static_cast<int>(height()*1.5));
    }

    void setChanged(bool changed)
    {
        m_changed = changed;
    }

private:
    PropertyEditor *m_editor;
    MultiProperty *m_property;
    bool m_changed;
};


class PropertyGroupItem: public KListViewItem{
public:
    PropertyGroupItem(KListView *parent, const QString &name)
        :KListViewItem(parent)
    {
        init();
    }
    PropertyGroupItem(KListViewItem *parent, const QString &name)
        :KListViewItem(parent)
    {
        init();
    }

    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
    {
        if (column == 0)
        {
            QFont font;
            font.setBold(true);
            p->setFont(font);
            p->setBrush(cg.highlight());
            p->setPen(cg.highlightedText());
        }
        KListViewItem::paintCell(p, cg, column, width, align);
    }
    virtual void setup()
    {
        KListViewItem::setup();
        setHeight(static_cast<int>(height()*1.4));
    }

private:
    void init()
    {
        setOpen(true);
    }
};

class SeparatorItem: public KListViewItem{
public:
    SeparatorItem(KListView *parent)
        :KListViewItem(parent)
    {
        setSelectable(false);
    }
};
PropertyEditor::PropertyEditor(QWidget *parent)
    :KListView(parent)
{
    setSorting(-1);

    addColumn(i18n("Name"));
    addColumn(i18n("Value"));
    setAllColumnsShowFocus(true);
    setColumnWidthMode(0, Q3ListView::Maximum);
    setResizeMode(Q3ListView::LastColumn);

    header()->setClickEnabled(false);

    connect(header(), SIGNAL(sizeChange(int, int, int)),
        this, SLOT(updateEditorSize()));
    connect(this, SIGNAL(currentChanged(Q3ListViewItem*)),
        this, SLOT(slotClicked(Q3ListViewItem*)));

    m_currentEditItem = 0;
    m_doubleClickForEdit = true;
    m_lastClickedItem = 0;
    m_currentEditWidget = 0;
    m_list = 0;

    m_currentEditArea = new QWidget(viewport());
    m_currentEditArea->hide();
    m_undoButton = new QPushButton(m_currentEditArea);
#ifndef PURE_QT
    m_undoButton->setPixmap(SmallIcon("undo"));
#else
    m_undoButton->setPixmap( QPixmap("undo.xpm") );
#endif
    m_undoButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    m_undoButton->resize(m_undoButton->height(), m_undoButton->height());
    m_undoButton->hide();
    connect(m_undoButton, SIGNAL(clicked()), this, SLOT(undo()));
    m_currentEditLayout = new QGridLayout(m_currentEditArea, 1, 2, 0, 0);
//    m_currentEditLayout->addWidget(m_undoButton, 0, 1);
}

PropertyEditor::~PropertyEditor()
{
    clearMachineCache();
}

void PropertyEditor::populateProperties(PropertyList *list)
{
    if (list == 0)
        return;
    m_list = list;
    connect(m_list, SIGNAL(propertyValueChanged(Property*)), this, SLOT(propertyValueChanged(Property*)));
    const QList<QPair<QString, QList<QString> > >& groups = m_list->propertiesOfGroup();
    for (QList<QPair<QString, QList<QString> > >::const_iterator it = groups.begin();
        it != groups.end(); ++it)
    {
//        qWarning("PropertyEditor::populateProperties:    adding group %s", (*it).first.ascii());
        PropertyGroupItem *group = 0;
        if ( (!(*it).first.isEmpty()) && ((*it).second.count() > 0) )
            group = new PropertyGroupItem(this, (*it).first);
        const QList<QString> &properties = (*it).second;
        for (QList<QString>::const_iterator it2 = properties.begin(); it2 != properties.end(); ++it2)
        {
//            qWarning("PropertyEditor::populateProperties:    adding property %s", (*it2).ascii());
            if (group)
                addProperty(group, *it2);
            else
                addProperty(*it2);
        }
    }
    if (firstChild())
    {
        setCurrentItem(firstChild());
        setSelected(firstChild(), true);
        slotClicked(firstChild());
    }
}

void PropertyEditor::addProperty(PropertyGroupItem *group, const QString &name)
{
    if ((*m_list)[name] == 0)
        return;
//        qWarning("%s = name : object null ", name.ascii());
    PropertyItem *pitem = new PropertyItem(this, group, (*m_list)[name]);
    addChildProperties(pitem);
}

void PropertyEditor::addProperty(const QString &name)
{
    if ((*m_list)[name] == 0)
        return;
//        qWarning("%s = name : object null ", name.ascii());
    PropertyItem *pitem = new PropertyItem(this, (*m_list)[name]);
    addChildProperties(pitem);
}

void PropertyEditor::addChildProperties(PropertyItem *parent)
{
    MultiProperty *prop = parent->property();
    //force machine creation to get detailed properties appended to current multiproperty
    if ( !m_registeredForType.contains(prop->name())
        && (PropertyMachineFactory::getInstance()->hasDetailedEditors(prop->type())) )
    {
        //FIXME: find better solution
        machine(prop);
    }

//     qWarning("seeking children: count: %d", prop->details.count());

    parent->setOpen(true);
    for (QList<ChildProperty>::iterator it = prop->details.begin(); it != prop->details.end(); ++it)
    {
//         qWarning("found child %s", (*it).name().ascii());
        new PropertyItem(this, parent, new MultiProperty(&m_detailedList, &(*it)));
    }
}

void PropertyEditor::clearProperties()
{
    m_detailedList.clear();
    if (!m_list)
        return;

    hideEditor();

    disconnect(m_list, SIGNAL(propertyValueChanged(Property*)), this, SLOT(propertyValueChanged(Property*)));
    clear();
    delete m_list;
    m_list = 0;
}

void PropertyEditor::propertyValueChanged(Property *property)
{
//     qWarning("PropertyEditor::propertyValueChanged");
    if (m_currentEditWidget->propertyName() == property->name())
        m_currentEditWidget->setValue(property->value(), false);
    else
    {
//        repaint all items
        Q3ListViewItemIterator it(this);
        while (it.current())
        {
            repaintItem(it.current());
            ++it;
        }
    }
}

void PropertyEditor::propertyChanged(MultiProperty *property, const QVariant &value)
{
    if (!property)
        return;

    qWarning("editor: assign %s to %s", property->name().latin1(), value.toString().latin1());
    property->setValue(value, false);

    //highlight changed properties
    if (m_currentEditItem && (m_currentEditItem->property() == property))
    {
        m_currentEditItem->setChanged(true);
        repaintItem(m_currentEditItem);
    }

    emit changed();

/*    if (m_list->contains(name))
    {
        (*m_list)[name]->setValue(value, false);
//    else if (m_detailedList->contains(*/
}

void PropertyEditor::hideEditor()
{
    m_lastClickedItem = 0;
    m_currentEditItem = 0;
    if (m_currentEditWidget)
    {
        m_currentEditLayout->remove(m_currentEditWidget);
        m_currentEditWidget->hide();
    }
    m_currentEditLayout->remove(m_undoButton);
    m_undoButton->hide();
    m_currentEditArea->hide();
    m_currentEditWidget = 0;
}

void PropertyEditor::showEditor(PropertyItem *item)
{
    m_currentEditItem = item;
    placeEditor(item);
    m_currentEditWidget->show();
    m_undoButton->show();
    m_currentEditArea->show();
}

void PropertyEditor::placeEditor(PropertyItem *item)
{
    QRect r = itemRect(item);
    if (!r.size().isValid())
    {
        ensureItemVisible(item);
        r = itemRect(item);
    }

    r.setX(header()->sectionPos(1));
    r.setWidth(header()->sectionSize(1));

    // check if the column is fully visible
    if (visibleWidth() < r.right())
        r.setRight(visibleWidth());

    r = QRect(viewportToContents(r.topLeft()), r.size());

    if (item->pixmap(1))
    {
        r.setX(r.x() + item->pixmap(1)->width());
    }

    if (PropertyWidget* editor = prepareEditor(item))
    {
        m_currentEditLayout->addWidget(editor, 0, 0);
        m_currentEditLayout->addWidget(m_undoButton, 0, 1);
        m_currentEditArea->resize(r.size());
//        m_currentEditLayout->invalidate();
        moveChild(m_currentEditArea, r.x(), r.y());
        m_currentEditWidget = editor;
    }
}

PropertyWidget* PropertyEditor::prepareEditor(PropertyItem *item)
{
    PropertyWidget *editorWidget = 0;
/*    if (item->depth() >= 2)
    {
        editorWidget = machine(item->name())->propertyEditor;
        editorWidget->setValue(m_accessor->value(item->name()), false);
    }
    else
    {*/
    editorWidget = machine(item->property())->propertyEditor;
    editorWidget->setProperty(item->property());
    if (item->property()->type() == Property::ValueFromList)
        editorWidget->setValueList(item->property()->valueList());
    editorWidget->setValue(item->property()->value(), false);
    //}
    return editorWidget;
}

void PropertyEditor::updateEditorSize()
{
    if (m_currentEditItem)
        placeEditor(m_currentEditItem);
}

void PropertyEditor::slotClicked(Q3ListViewItem *item)
{
    if (item == 0)
    {
        hideEditor();
        return;
    }
    if (item != m_lastClickedItem)
    {
        hideEditor();
        PropertyItem *it = dynamic_cast<PropertyItem*>(item);
        if (it)
        {
            showEditor(it);
        }
    }

    m_lastClickedItem = item;
}

Machine *PropertyEditor::machine(MultiProperty *property)
{
//    int type = property->type();
    QString name = property->name();
    QMap<QString, QVariant> values = property->valueList();
    if (m_registeredForType[name] == 0)
    {
        m_registeredForType[name] = PropertyMachineFactory::getInstance()->machineForProperty(property);
        connect(m_registeredForType[name]->propertyEditor, SIGNAL(propertyChanged(MultiProperty*, const QVariant&)),
		this, SLOT(propertyChanged(MultiProperty*, const QVariant&)));
        m_registeredForType[name]->propertyEditor->reparent(m_currentEditArea, 0, m_currentEditArea->childrenRect().topLeft());
        m_registeredForType[name]->propertyEditor->hide();
    }
    return m_registeredForType[name];
}

void PropertyEditor::clearMachineCache()
{
    for (QMap<QString, Machine* >::iterator it = m_registeredForType.begin(); it != m_registeredForType.end(); ++it)
    {
        delete it.data();
    }
    m_registeredForType.clear();
}

void PropertyEditor::undo()
{
    if ((m_currentEditItem == 0) || (m_currentEditWidget == 0)
        || (!m_currentEditWidget->isVisible()))
        return;

    m_currentEditWidget->undo();
    m_currentEditItem->setChanged(false);
    repaintItem(m_currentEditItem);
}

}

#ifndef PURE_QT
#include "propertyeditor.moc"
#endif
