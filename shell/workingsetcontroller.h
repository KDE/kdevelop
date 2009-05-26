
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

class KConfigGroup;

namespace Sublime {
class Area;
class AreaIndex;
}

namespace KDevelop {
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

private:
    void saveFromArea(Sublime::Area* area, Sublime::AreaIndex * areaIndex, KConfigGroup & group);
    void loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex, KConfigGroup group);
    
    WorkingSet(const WorkingSet& rhs) {
        m_id =  rhs.m_id + "_copy_";
    }
    QString m_id;
//     QList<Sublime::Area*> m_areas;
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
private:
    QMap<QString, WorkingSet*> m_workingSets;
    KDevelop::Core* m_core;
};
}

#endif // WORKINGSETMANAGER_H
