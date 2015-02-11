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

#ifndef KDEVPLATFORM_WORKINGSETFILELABEL_H
#define KDEVPLATFORM_WORKINGSETFILELABEL_H

#include <QLabel>

class QMouseEvent;

namespace KDevelop {

class WorkingSetFileLabel : public QLabel {
    Q_OBJECT

public:
    WorkingSetFileLabel();
    virtual void mouseReleaseEvent(QMouseEvent* ev) override;
    void setIsActiveFile(bool active);

    bool isActive() const;

    void emitClicked();

Q_SIGNALS:
    void clicked();

private:
    bool m_isActive;
};

}

#endif // KDEVPLATFORM_WORKINGSETFILELABEL_H
