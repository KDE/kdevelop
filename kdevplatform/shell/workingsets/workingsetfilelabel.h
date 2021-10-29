/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WORKINGSETFILELABEL_H
#define KDEVPLATFORM_WORKINGSETFILELABEL_H

#include <QLabel>

namespace KDevelop {

class WorkingSetFileLabel : public QLabel {
    Q_OBJECT

public:
    WorkingSetFileLabel();
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void setIsActiveFile(bool active);

    bool isActive() const;

    void emitClicked();

Q_SIGNALS:
    void clicked();

private:
    bool m_isActive = false;
};

}

#endif // KDEVPLATFORM_WORKINGSETFILELABEL_H
