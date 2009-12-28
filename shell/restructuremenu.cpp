/*
    Copyright David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "restructuremenu.h"
#include <QMenuBar>
#include <kdebug.h>
 
uint qHash(const QStringList& l)
{
    uint ret = 0;
    for(QList< QString >::const_iterator it = l.begin(); it != l.end(); ++it)
    {
        ret = (ret << 17) + (ret >> 3) + qHash(*it);
    }
    return ret;
}

QString nameForAction(QAction* action)
{
    if(action->isSeparator())
        return QString("%1").arg((size_t)action);
    else
        return action->text().remove('&');
}

RestructureMenu::RestructureMenu(QMenuBar* _menuBar) : menuBar(_menuBar) {
}

void RestructureMenu::record() {
    m_recorded.clear();
    
    foreach(QAction* action, menuBar->actions())
    {
        record(QStringList(), action);
    }
}
    
void RestructureMenu::recordDifference() {
    foreach(QAction* action, menuBar->actions())
    {
        recordDifference(QStringList(), action);
    }
}

void RestructureMenu::map(QStringList from, QStringList to) {
    m_map.push_back(qMakePair(from, to));
}

void RestructureMenu::setOrder(QStringList order)
{
    m_order = order;
}


void RestructureMenu::restructure() {
    m_processed.clear();
    
    menuBar->setUpdatesEnabled(false);
    
    for(QList< QPair< QStringList, QStringList > >::iterator mapIt = m_map.begin(); mapIt != m_map.end(); ++mapIt)
    {
        QStringList mapFrom = (*mapIt).first;
        QStringList mapTo = (*mapIt).second;
        
//             kDebug() << "CHECKING map from" << mapFrom << "to" << mapTo;
        
        for(uint difference = 0; difference < 5; ++difference)
        {
            for(QHash< QStringList, QList<QPointer< QAction > > >::const_iterator it = m_difference.begin(); it != m_difference.end(); ++it)
            {
                for(QList< QPointer< QAction > >::const_iterator actionIt = (*it).begin(); actionIt != (*it).end(); ++actionIt)
                {
                    QStringList scope = it.key();
                    scope << nameForAction(*actionIt);
                    kDebug() << "checking" << scope;
                    
                    if(m_processed.find(scope) != m_processed.end())
                        continue;
                    
                    if(scope.length() == mapFrom.length()+difference && (scope.isEmpty() || scope.mid(0, mapFrom.length()) == mapFrom))
                    {
                        QStringList oldPrefix = scope;
                        oldPrefix.removeLast();
                        markProcessed(oldPrefix, (*actionIt));
                        
                        QStringList newPrefix = mapTo;
                        newPrefix += scope.mid(mapFrom.length());
                        newPrefix.removeLast();
//                             kDebug() << "mapping from" << oldPrefix << "to" << newPrefix;

                        if(oldPrefix != newPrefix) {
                            removeActionFromMenu(oldPrefix, (*actionIt));
                            
                            insertAction(newPrefix, (*actionIt));
                        }
                    }
                }
            }
        }
    }

    if(!m_order.isEmpty())
    {
        QMap<QString, QAction*> actions;
        foreach(QAction* action, menuBar->actions())
            actions[nameForAction(action)] = action;
        
        //Ensure correct odering
        for(uint a = 0; a < m_order.size(); ++a)
        {
            if(actions.contains(m_order[a]))
            {
                QAction* action = actions[m_order[a]];

                //Disable 'separator' actions
                if(action->text().length() == 1)
                    action->setEnabled(false);
                
                //Make sure this action is before all actions that should be behind
                
                for(uint b = a+1; b < m_order.size(); ++b)
                {
                    if(actions.contains(m_order[b]))
                    {
                        QAction* laterAction = actions[m_order[b]];
                        
                        if(menuBar->actions().indexOf(action) > menuBar->actions().indexOf(laterAction))
                        {
                            menuBar->removeAction(action);
                            menuBar->insertAction(laterAction, action);
                        }
                    }
                }
            }
        }
    }

    menuBar->setUpdatesEnabled(true);
}
    
void RestructureMenu::record(QStringList prefix, QAction* action) {
    prefix << nameForAction(action);
    m_recorded.insert(prefix, action);
    
    if(action->menu())
        m_menuActions.insert(prefix, action);
    
    kDebug() << "recording" << prefix;
    
    if(action->menu())
    {
        foreach(QAction* child, action->menu()->actions())
            record(prefix, child);
    }
}

void RestructureMenu::markProcessed(QStringList prefix, QAction* action) {
    prefix << nameForAction(action);
    m_processed.insert(prefix, action);
    
    if(action->menu())
    {
        foreach(QAction* child, action->menu()->actions())
            markProcessed(prefix, child);
    }
}

void RestructureMenu::recordDifference(QStringList prefix, QAction* action) {
    
    QStringList nextPrefix = prefix;
    nextPrefix << nameForAction(action);
    
    if(!m_recorded.contains(nextPrefix))
    {
        kDebug() << "difference:" << nextPrefix << action->isSeparator();
        
        m_difference[prefix] << action;
    }
    
    if(action->menu())
    {
        foreach(QAction* child, action->menu()->actions())
            recordDifference(nextPrefix, child);
    }
}

void RestructureMenu::insertAction(QStringList prefix, QAction* action) {
    Q_ASSERT(action && menuBar);
    QMenu* previousMenu = 0;
    
    kDebug() << "inserting" << action->text() << action->isSeparator();
    
    for(uint a = 0; a < prefix.size(); ++a)
    {
        QAction* menuAction = 0;
        QHash< QStringList, QPointer< QAction > >::iterator it = m_menuActions.find(prefix.mid(0, a+1));
        
        if(it != m_menuActions.end())
            menuAction = *it;
        
        if(menuAction)
        {
            previousMenu = menuAction->menu();
        }else{
            if(previousMenu)
            {
                QMenu* newMenu = new QMenu(prefix.at(a));
                menuAction = previousMenu->addMenu(newMenu);
                m_menuActions.insert(prefix.mid(0, a+1), menuAction);
            }else{
                Q_ASSERT(a == 0);
                
                QMenu* newMenu = new QMenu(prefix.at(a));
                menuAction = menuBar->addMenu(newMenu);
                m_menuActions.insert(prefix.mid(0, a+1), menuAction);
            }
        }
        Q_ASSERT(menuAction);
        previousMenu = menuAction->menu();
        Q_ASSERT(previousMenu);
    }
    
    if(previousMenu)
        previousMenu->addAction(action);
    else
        menuBar->addAction(action);
}

void RestructureMenu::removeActionFromMenu(QStringList prefix, QAction* action) {
    Q_ASSERT(action && menuBar);
    QMenu* previousMenu = 0;
    for(uint a = 0; a < prefix.size(); ++a)
    {
        QHash< QStringList, QPointer< QAction > >::iterator it = m_menuActions.find(prefix.mid(0, a+1));
        
        if(it != m_menuActions.end())
        {
            previousMenu = (*it)->menu();
        }
        Q_ASSERT(previousMenu);
    }
    
    if(previousMenu)
        previousMenu->removeAction(action);
    else
        menuBar->removeAction(action);
}
