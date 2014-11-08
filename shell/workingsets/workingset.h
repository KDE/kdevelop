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

#ifndef KDEVPLATFORM_WORKINGSET_H
#define KDEVPLATFORM_WORKINGSET_H

#include <QObject>
#include <QIcon>
#include <KConfigGroup>
#include <QPointer>

namespace Sublime {
class Area;
class AreaIndex;
class View;
}

namespace KDevelop {

/// Contains all significant parameters which control the appearance of a working set icon
struct WorkingSetIconParameters {
    WorkingSetIconParameters(const QString& id)
        : setId(qHash(id) % 268435459)
        , coloredCount((setId % 15 < 4) ? 1 : (setId % 15 < 10) ? 2 : (setId % 15 == 14) ? 4 : 3)
        , hue((setId % 273 * 83) % 360)
        , swapDiagonal(setId % 31 < 16)
    { };
    // calculate layout and colors depending on the working set ID
    // modulo it so it's around 2^28, leaving some space before uint overflows
    const uint setId;
    // amount of colored squares in this icon (the rest is grey or whatever you set as default color)
    // use 4-6-4-1 weighting for 1, 2, 3, 4 squares, because that's the number of arrangements for each
    const uint coloredCount;
    const uint hue;
    bool swapDiagonal;
    // between 0 and 100, 100 = very similar, 0 = very different
    // 20 points should make a significantly different icon.
    uint similarity(const WorkingSetIconParameters& other) const {
        int sim = 100;
        uint hueDiff = qAbs<int>(hue - other.hue);
        hueDiff = hueDiff > 180 ? 360 - hueDiff : hueDiff;
        sim -= hueDiff > 35 ? 50 : (hueDiff * 50) / 180;
        if ( coloredCount != other.coloredCount ) {
            sim -= 50;
        }
        else if ( coloredCount == 2 && swapDiagonal != other.swapDiagonal ) {
            sim -= 35;
        }
        return sim;
    };
};


class WorkingSet : public QObject {
    Q_OBJECT

public:
    WorkingSet(const QString& id);

    bool isConnected(Sublime::Area* area);

    QIcon icon() const;

    bool isPersistent() const;

    void setPersistent(bool persistent);

    QString id() const;

    QStringList fileList() const;

    bool isEmpty() const;

    ///Updates this working-set from the given area and area-index
    void saveFromArea(Sublime::Area* area, Sublime::AreaIndex * areaIndex);

    ///Loads this working-set directly from the configuration file, and stores it in the given area
    ///Does not ask the user, this should be done beforehand.
    void loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex);

    bool hasConnectedAreas() const;

    bool hasConnectedAreas(QList<Sublime::Area*> areas) const;

    void connectArea(Sublime::Area* area);

    void disconnectArea(Sublime::Area* area);

    void deleteSet(bool force, bool silent = false);

private slots:
    void areaViewAdded(Sublime::AreaIndex* /*index*/, Sublime::View* /*view*/);
    void areaViewRemoved(Sublime::AreaIndex* /*index*/, Sublime::View* /*view*/);

signals:
    void setChangedSignificantly();
    void aboutToRemove(WorkingSet*);

private:
    void changed(Sublime::Area* area);

    void saveFromArea(Sublime::Area* area, Sublime::AreaIndex *areaIndex, KConfigGroup setGroup, KConfigGroup areaGroup);
    void loadToArea(Sublime::Area* area, Sublime::AreaIndex *areaIndex, KConfigGroup setGroup, KConfigGroup areaGroup, QMultiMap<QString, Sublime::View*>& recycle);

    QString m_id;
    QIcon m_icon;
    QList<QPointer<Sublime::Area> > m_areas;
    static bool m_loading;
};

}

#endif // KDEVPLATFORM_WORKINGSET_H
