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
#else
#include "compat_tools.h"
#endif

#include <qtable.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include "property.h"
#include "multiproperty.h"
#include "propertymachinefactory.h"

class PropertyItem: public QListViewItem{
public:
    PropertyItem(PropertyEditor *parent, MultiProperty *property)
        :QListViewItem(parent, property->description()), m_editor(parent), m_property(property)
    {
    }
    
    PropertyItem(PropertyEditor *editor, QListViewItem *parent, MultiProperty *property)
        :QListViewItem(parent, property->description()), m_editor(editor),
        m_property(property)
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
        if (column == 1)
        {
            QRect r(0, 0, m_editor->header()->sectionSize(1), height()-1);
            //FIXME: this is ugly, but how else can we deal with ValueFromList properties?
            QVariant valueToDraw;
            if (m_property->type() == Property::ValueFromList)
                valueToDraw = m_property->findValueDescription();
            else
                valueToDraw = m_property->value();
            m_editor->machine(m_property)->propertyEditor->drawViewer(p, cg, r, valueToDraw);
            return;
        }
        QListViewItem::paintCell(p, cg, column, width, align);
    }

private:
    PropertyEditor *m_editor;
    MultiProperty *m_property;
};


class PropertyGroupItem: public QListViewItem{
public:
    PropertyGroupItem(QListView *parent, const QString &name)
        :QListViewItem(parent, name)
    {
        init();
    }
    PropertyGroupItem(QListViewItem *parent, const QString &name)
        :QListViewItem(parent, name)
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
        QListViewItem::paintCell(p, cg, column, width, align);
    }
private:
    void init()
    {
        setOpen(true);
    }
};

class SeparatorItem: public QListViewItem{
public:
    SeparatorItem(QListView *parent)
        :QListViewItem(parent)
    {
        setSelectable(false);
    }
};

PropertyEditor::PropertyEditor(QWidget *parent, const char *name, WFlags f)
    :QListView(parent, name, f)
{
    setSorting(-1);
    
    addColumn(i18n("Name"));
    addColumn(i18n("Value"));
    setAllColumnsShowFocus(true);
    setResizeMode(QListView::AllColumns);

    header()->setClickEnabled(false);

    connect(header(), SIGNAL(sizeChange(int, int, int)),
        this, SLOT(updateEditorSize()));
    connect(this, SIGNAL(currentChanged(QListViewItem*)),
        this, SLOT(slotClicked(QListViewItem*)));

    m_currentEditItem = 0;
    m_doubleClickForEdit = true;
    m_lastClickedItem = 0;    
    m_currentEditWidget = 0;
    m_list = 0;
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
    const QValueList<QPair<QString, QValueList<QString> > >& groups = m_list->propertiesOfGroup();
    for (QValueList<QPair<QString, QValueList<QString> > >::const_iterator it = groups.begin();
        it != groups.end(); ++it)
    {
//        qWarning("PropertyEditor::populateProperties:    adding group %s", (*it).first.ascii());
        PropertyGroupItem *group = 0;
        if ( (!(*it).first.isEmpty()) && ((*it).second.count() > 0) )
            group = new PropertyGroupItem(this, (*it).first);
        const QValueList<QString> &properties = (*it).second;
        for (QValueList<QString>::const_iterator it2 = properties.begin(); it2 != properties.end(); ++it2)
        {
//            qWarning("PropertyEditor::populateProperties:    adding property %s", (*it2).ascii());
            if (group)
                addProperty(group, *it2);
            else
                addProperty(*it2);
        }
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
    if ( !m_registeredForType.contains(prop->type())
        && (PropertyMachineFactory::getInstance()->hasDetailedEditors(prop->type())) )
    {
        //FIXME: find better solution
        machine(prop);
    }
        
    qWarning("seeking children: count: %d", prop->details.count());

    parent->setOpen(true);
    for (QValueList<ChildProperty>::iterator it = prop->details.begin(); it != prop->details.end(); ++it)
    {
        qWarning("found child %s", (*it).name().ascii());
        new PropertyItem(this, parent, new MultiProperty(&m_detailedList, &(*it)));
    }
}

void PropertyEditor::clearProperties()
{
    m_detailedList.clear();
    if (!m_list)
        return;
    if (m_currentEditWidget)
        m_currentEditWidget->hide();
    
    clear();
    disconnect(m_list, SIGNAL(propertyValueChanged(Property*)), this, SLOT(propertyValueChanged(Property*)));
    delete m_list;
    m_list = 0;
}

void PropertyEditor::propertyValueChanged(Property *property)
{
    if (m_currentEditWidget->propertyName() == property->name())
        m_currentEditWidget->setValue(property->value(), false);
    else
    {
        //repaint all items
        QListViewItemIterator it(this);
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
    
    kdDebug() << "editor: assign " << property->name().latin1() << " to " << value.toString().latin1() << endl;
    property->setValue(value, false);
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
        m_currentEditWidget->hide();
    m_currentEditWidget = 0;
}

void PropertyEditor::showEditor(PropertyItem *item)
{
    m_currentEditItem = item;
    placeEditor(item);
    m_currentEditWidget->show();
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
        editor->resize(r.size());
        moveChild(editor, r.x(), r.y());
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
    editorWidget->setValue(item->property()->value(), false);
    //}
    return editorWidget;
}

void PropertyEditor::updateEditorSize()
{
    if (m_currentEditItem)
        placeEditor(m_currentEditItem);
}

void PropertyEditor::slotClicked(QListViewItem *item)
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
    int type = property->type();
    QString name = property->name();
    QMap<QString, QVariant> values = property->valueList();
    if (m_registeredForType[type] == 0)
    {
        m_registeredForType[type] = PropertyMachineFactory::getInstance()->machineForProperty(property);
        connect(m_registeredForType[type]->propertyEditor, SIGNAL(propertyChanged(MultiProperty*, const QVariant&)),
            this, SLOT(propertyChanged(MultiProperty*, const QVariant&)));
        m_registeredForType[type]->propertyEditor->reparent(viewport(), 0, viewport()->childrenRect().topLeft());
        m_registeredForType[type]->propertyEditor->hide();
    }
    return m_registeredForType[type];
}

void PropertyEditor::clearMachineCache()
{
    for (QMap<int, Machine* >::iterator it = m_registeredForType.begin(); it != m_registeredForType.end(); ++it)
    {
        delete it.data();
    }
    m_registeredForType.clear();
}

#include "propertyeditor.moc"
