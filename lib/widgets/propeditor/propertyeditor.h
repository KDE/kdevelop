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
#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#ifndef PURE_QT
#include <klistview.h>
#else
#include <qlistview.h>
#define KListView QListView
#define KListViewItem QListViewItem
#endif

#include "propertylist.h"

class QPushButton;
class QGridLayout;

namespace PropertyLib{

class PropertyItem;
class PropertyGroupItem;
class PropertyWidget;
class Property;
class MultiProperty;
struct Machine;

/** @file propertyeditor.h
@short Contains @ref PropertyLib::PropertyEditor class.
*/

/**
@short %Property editor.

Displays a list of properties in a table form. Also performs grouping and
creation of property widgets from the machine factory.
@see PropertyWidget
@see Machine
@see PropertyMachineFactory
*/
class PropertyEditor: public KListView{
   Q_OBJECT
public:
    /**Constructs the property editor.*/
    PropertyEditor(QWidget *parent = 0, const char *name = 0);
    ~PropertyEditor();
    
    /**@return @ref Machine for given property.
    Uses cache to store created machines. 
    Cache will be cleared only with @ref clearMachineCache.*/
    Machine *machine(MultiProperty *property);
    
public slots:
    /**Shows properties from a list.*/
    void populateProperties(PropertyList *list);
    /**Clears property list, disconnects accessor from the editor and deletes it.*/
    void clearProperties();
    /**Deletes cached machines.*/
    void clearMachineCache();

signals:
    /**Emitted when something is changed in property editor.*/
    void changed();

protected slots:
    /**Updates property widget in the editor.*/
    void propertyValueChanged(Property* property);
    /**Updates property in the list when new value is selected in the editor.*/
    void propertyChanged(MultiProperty *property, const QVariant &value);

    /**Shows property editor.*/
    void slotClicked(QListViewItem* item);
    void updateEditorSize();
    
    /**Undoes the last change in property editor.*/
    void undo();
    
protected:
    void editItem(QListViewItem*, int);
    void hideEditor();
    void showEditor(PropertyItem *item);
    void placeEditor(PropertyItem *item);
    PropertyWidget *prepareEditor(PropertyItem *item);

    void addGroup(const QString &name);
    void addProperty(PropertyGroupItem *group, const QString &name);
    void addProperty(const QString &name);
    void addChildProperties(PropertyItem *parent);
    
private:
    PropertyList *m_list;
    PropertyList m_detailedList;

    //machines cache for property types, machines will be deleted
    QMap<QString, Machine* > m_registeredForType;

    PropertyItem *m_currentEditItem;
    PropertyWidget *m_currentEditWidget;
    QWidget *m_currentEditArea;
    QGridLayout *m_currentEditLayout;
    
    bool m_doubleClickForEdit;
    QListViewItem* m_lastClickedItem;
    
    QPushButton *m_undoButton;
    
friend class PropertyItem;
};

}

#endif
