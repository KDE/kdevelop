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

#include <qlistview.h>

class PropertyAccessor;
class PropertyItem;
class PropertyGroupItem;
class PropertyWidget;
class Property;
struct Machine;

/** @file propertyeditor.h
@short Contains @ref PropertyEditor class.
*/

/**
@short %Property editor.

Displays a list of properties in a table form. Also performs grouping and
creation of property widgets from the machine factory.
@see PropertyWidget
@see Machine
@see PropertyMachineFactory
*/
class PropertyEditor: public QListView{
   Q_OBJECT
public:
    /**Constructs the property editor.*/
    PropertyEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~PropertyEditor();
    
    /**@return @ref Machine for given property name.
    Uses cache to store created machines. 
    Cache will be cleared only with @ref clearMachineCache.*/
    Machine *machine(const QString &name);
    
public slots:
    /**Shows properties from accessor.*/
    void populateProperties(PropertyAccessor *accessor);
    /**Clears property list, disconnects accessor from the editor and deletes it.*/
    void clearProperties();
    /**Deletes cached machines.*/
    void clearMachineCache();

protected slots:
    /**Updates property widget in the editor.*/
    void propertyValueChanged(Property* property);
    /**Updates accessor when new value is selected in the editor.*/
    void propertyChanged(const QString &name, const QVariant &value);

    /**Shows property editor.*/
    void slotClicked(QListViewItem* item);
    void updateEditorSize();
    
protected:
    void editItem(QListViewItem*, int);
    void hideEditor();
    void showEditor(PropertyItem *item);
    void placeEditor(PropertyItem *item);
    PropertyWidget *prepareEditor(PropertyItem *item);

    void addGroup(const QString &name);
    void addProperty(PropertyGroupItem *group, const QString &name);
    void addProperty(const QString &name);
    
private:
    PropertyAccessor *m_accessor;

    //machines cache for property types, machines will be deleted
    QMap<int, Machine* > m_registeredForType;

    PropertyItem *m_currentEditItem;
    PropertyWidget *m_currentEditWidget;
    
    bool m_doubleClickForEdit;
    QListViewItem* m_lastClickedItem;
    
friend class PropertyItem;
};

#endif
