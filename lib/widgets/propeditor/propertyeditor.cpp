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

#include "property.h"
#include "propertyaccessor.h"
#include "propertymachinefactory.h"

class PropertyItem: public QListViewItem{
public:
    PropertyItem(PropertyEditor *parent, const QString &name, int type)
        :QListViewItem(parent, name), m_editor(parent), m_type(type)
    {
    }
    
    PropertyItem(PropertyEditor *editor, QListViewItem *parent, const QString &name, int type)
        :QListViewItem(parent, name), m_editor(editor), m_type(type)
    {
    }
    
    int type() const
    {
        return m_type;
    }
        
    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
    {
        if (column == 1)
        {
            QRect r(0, 0, m_editor->header()->sectionSize(1), height()-1);
            //FIXME: this is ugly, but how else can we deal with ValueFromList properties?
            QVariant valueToDraw;
            if (type() == ValueFromList)
                valueToDraw = m_editor->m_accessor->findValueDescription(text(0));
            else
                valueToDraw = m_editor->m_accessor->value(text(0));
            m_editor->machine(text(0))->propertyEditor->drawViewer(p, cg, r, valueToDraw);
            return;
        }
        QListViewItem::paintCell(p, cg, column, width, align);
    }

private:
    PropertyEditor *m_editor;
    int m_type;
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
}

PropertyEditor::~PropertyEditor()
{
    clearMachineCache();
}

void PropertyEditor::populateProperties(PropertyAccessor *accessor)
{
    if (accessor == 0)
        return;
    m_accessor = accessor;
    connect(m_accessor, SIGNAL(propertyValueChanged(Property*)), this, SLOT(propertyValueChanged(Property*)));
    const QValueList<QPair<QString, QValueList<QString> > >& groups = m_accessor->propertiesOfGroup();
    for (QValueList<QPair<QString, QValueList<QString> > >::const_iterator it = groups.begin();
        it != groups.end(); ++it)
    {
        PropertyGroupItem *group = 0;
        if (!(*it).first.isEmpty())
            group = new PropertyGroupItem(this, (*it).first);
        const QValueList<QString> &properties = (*it).second;
        for (QValueList<QString>::const_iterator it2 = properties.begin(); it2 != properties.end(); ++it2)
        {
            if (group)
                addProperty(group, *it2);
            else
                addProperty(*it2);
        }
    }
}

void PropertyEditor::addProperty(PropertyGroupItem *group, const QString &name)
{
    new PropertyItem(this, group, name, m_accessor->type(name));    
}

void PropertyEditor::addProperty(const QString &name)
{
    new PropertyItem(this, name, m_accessor->type(name));
}

void PropertyEditor::clearProperties()
{
    disconnect(m_accessor, SIGNAL(propertyValueChanged(Property*)), this, SLOT(propertyValueChanged(Property*)));
    delete m_accessor;
    m_accessor = 0;
}

void PropertyEditor::propertyValueChanged(Property *property)
{
    if (m_currentEditWidget->propertyName() == property->name())
        m_currentEditWidget->setValue(property->value(), false);
}

void PropertyEditor::propertyChanged(const QString &name, const QVariant &value)
{
    kdDebug() << "editor: assign " << name.latin1() << " to " << value.toString().latin1() << endl;
    m_accessor->setValue(name, value, false);
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
    PropertyWidget *editorWidget = machine(item->text(0))->propertyEditor;    
    editorWidget->setValue(m_accessor->value(item->text(0)), false);
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

Machine *PropertyEditor::machine(const QString &name)
{
    int type = m_accessor->type(name);
    QMap<QString, QVariant> values = m_accessor->valueList(name);
    if (m_registeredForType[type] == 0)
    {
        m_registeredForType[type] = PropertyMachineFactory::getInstance()->machineForProperty(name, type, values);
        connect(m_registeredForType[type]->propertyEditor, SIGNAL(propertyChanged(const QString&, const QVariant&)),
            this, SLOT(propertyChanged(const QString&, const QVariant&)));
        m_registeredForType[type]->propertyEditor->reparent(viewport(), 0, viewport()->childrenRect().topLeft());
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
