/*
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef RESTRUCTUREMENU_H
#define RESTRUCTUREMENU_H
#include <QHash>
#include <QStringList>
#include <QPointer>
#include <QAction>

class QMenuBar;

/**
 * A helper class that allows recording changes to a menu,
 * and transforming the recorded changes according to specific maps.
 */

class RestructureMenu
{
    public:
    RestructureMenu(QMenuBar* _menuBar);
        
    ///Records the structure of the menu. Should be called _before_ something is merged into the menu
    void record();
    
    ///Should be called _after_ something was merged into the menu. The differences will be recorded,
    ///and can later be re-structured through maps.
    void recordDifference();
    
    ///Add maps to restructure the added parts of the menu
    ///More specific maps must be given first, then the less specific ones
    void map(QStringList from, QStringList to);
    
    ///Restructures the added parts of the menu according to the given maps
    void restructure();
    
    ///Set a fixed order of the main-menu entries that will be enforced
    void setOrder(QStringList order);

    ///Removes all actions in the list from the menu, without deleting them
    void removeActions (QSet< QAction* > actions, QMenu* parentMenu = 0);

    ///Returns all actions that were added
    QList<QPointer<QAction> > addedActions();
    
    private:
    
    void record(QStringList prefix, QAction* action);

    //Removes this action and all its children from the 'difference' set
    void markProcessed(QStringList prefix, QAction* action);

    void recordDifference(QStringList prefix, QAction* action);
    
    void insertAction(QStringList prefix, QAction* action);

    void removeActionFromMenu(QStringList prefix, QAction* action);

    QList<QPair<QStringList, QStringList> > m_map;
    
    QHash<QStringList, QPointer<QAction> > m_menuActions;
    
    QHash<QStringList, QPointer<QAction> > m_recorded;
    QHash<QStringList, QList<QPointer<QAction> > > m_difference;
    
    QHash<QStringList, QPointer<QAction> > m_processed;

    QStringList m_order;
    
    QMenuBar* menuBar;
};
#endif // RESTRUCTUREMENU_H
