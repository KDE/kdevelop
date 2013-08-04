/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Milian Wolff <mail@milianw.de>

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

#ifndef KDEVPLATFORM_WORKINGSETWIDGET_H
#define KDEVPLATFORM_WORKINGSETWIDGET_H

#include <QPointer>

#include "workingsettoolbutton.h"

namespace Sublime {
class Area;
}

namespace KDevelop {

class WorkingSetController;
class WorkingSet;

/**
 * One instance of this widget is created per area switcher tab, showing
 * the currently opened working set for this area.
 */
class WorkingSetWidget : public WorkingSetToolButton {
    Q_OBJECT

public:
    WorkingSetWidget(Sublime::Area* area, QWidget* parent = 0);
    virtual void setVisible( bool visible );

private slots:
    void changingWorkingSet(Sublime::Area* area, const QString& from, const QString& to);
    void setChangedSignificantly();

private:
    QPointer<Sublime::Area> m_area;
};

}

#endif // KDEVPLATFORM_WORKINGSETWIDGET_H
