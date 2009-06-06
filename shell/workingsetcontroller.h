
/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

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

#ifndef WORKINGSETCONTROLLER_H
#define WORKINGSETCONTROLLER_H

#include <QObject>
#include <QMap>
#include "uicontroller.h"
#include <sublime/area.h>
#include <qlabel.h>

class KConfigGroup;

namespace Sublime {
class Area;
class AreaIndex;
}

namespace KDevelop {
class UiController;
class MainWindow;
class Core;

class WorkingSet : public QObject {
    Q_OBJECT
public:
    WorkingSet(QString id) : m_id(id) {
    }
    QString id() {
        return m_id;
    }
    ///Creates a copy of this working-set with a new identity
    WorkingSet* clone() {
        WorkingSet* ret = new WorkingSet(*this);
        return ret;
    }

    ///Updates this working-set from the given area and area-index
    void saveFromArea(Sublime::Area* area, Sublime::AreaIndex * areaIndex);
    
    ///Loads this working-set directly from the configuration file, and stores it in the given area
    void loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex);

    void connectArea(Sublime::Area* area) {
        if(m_areas.contains(area)) {
            kDebug() << "tried to double-connect area";
            return;
        }
        
        loadToArea(area, area->rootIndex());
        m_areas.push_back(area);
        connect(area, SIGNAL(viewAdded(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewAdded(Sublime::AreaIndex*,Sublime::View*)));
        connect(area, SIGNAL(viewRemoved(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewRemoved(Sublime::AreaIndex*,Sublime::View*)));
    }
    
    void disconnectArea(Sublime::Area* area) {
        if(!m_areas.contains(area)) {
            kDebug() << "tried to disconnect not connected area";
            return;
        }
        
        disconnect(area, SIGNAL(viewAdded(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewAdded(Sublime::AreaIndex*,Sublime::View*)));
        disconnect(area, SIGNAL(viewRemoved(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewRemoved(Sublime::AreaIndex*,Sublime::View*)));
        m_areas.removeAll(area);
    }

private slots:
    void areaViewAdded(Sublime::AreaIndex* /*index*/, Sublime::View* /*view*/) {
        Sublime::Area* area = qobject_cast<Sublime::Area*>(sender());
        Q_ASSERT(area);
        changed(area);
    }
    
    void areaViewRemoved(Sublime::AreaIndex* /*index*/, Sublime::View* /*view*/) {
        Sublime::Area* area = qobject_cast<Sublime::Area*>(sender());
        Q_ASSERT(area);
        changed(area);
    }
private:
    
    void changed(Sublime::Area* area) {
        saveFromArea(area, area->rootIndex());
        for(QList< QPointer< Sublime::Area > >::iterator it = m_areas.begin(); it != m_areas.end(); ++it) {
            if((*it) != area) {
                loadToArea((*it), (*it)->rootIndex());
            }
        }
    }
    
    void saveFromArea(Sublime::Area* area, Sublime::AreaIndex * areaIndex, KConfigGroup & group);
    void loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex, KConfigGroup group);
    
    WorkingSet(const WorkingSet& rhs) {
        m_id =  rhs.m_id + "_copy_";
    }

    QString m_id;
    QList<QPointer<Sublime::Area> > m_areas;
};

class WorkingSetController : public QObject
{
Q_OBJECT
public:
    WorkingSetController(KDevelop::Core* core) ;
    ///Returns a working-set management widget
//     QWidget* createManagerWidget(QObject* parent);
    
    WorkingSet* getWorkingSet(QString id);
    void initialize() {}
    void cleanup();

    //The returned widget is owned by the caller
    QWidget* createSetManagerWidget(MainWindow* parent, bool local = false) ;

    void initializeController(UiController* controller) {
        connect(controller, SIGNAL(areaCreated(Sublime::Area*)), this, SLOT(areaCreated(Sublime::Area*)));
    }
private slots:
    void areaCreated(Sublime::Area* area) {
        WorkingSet* set = getWorkingSet(area->workingSet());
        set->connectArea(area);
    }
private:
    QMap<QString, WorkingSet*> m_workingSets;
    KDevelop::Core* m_core;
};
}

#endif // WORKINGSETMANAGER_H
