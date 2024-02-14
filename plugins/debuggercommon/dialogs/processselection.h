/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSSELECTION_H
#define PROCESSSELECTION_H

#include <QDialog>

#include "ui_processselection.h"


class QItemSelection;
class QPushButton;

namespace KSysGuard {
    class ProcessDataModel;
}

class ProcessesSortFilterModel;

namespace KDevMI {

class ProcessSelectionDialog : public QDialog
{
    Q_OBJECT
    public:
        explicit ProcessSelectionDialog( QWidget *parent=nullptr );
        ~ProcessSelectionDialog() override;
        long int pidSelected();
        // QSize sizeHint() const override;

    private Q_SLOTS:
        void selectionChanged( const QItemSelection& selected );
        void onProcessesComboActivated(int index);

    private:
        Ui::ProcessSelection m_ui;

        KSysGuard::ProcessDataModel *m_dataModel = nullptr;
        ProcessesSortFilterModel *m_sortModel = nullptr;

        QPushButton *m_attachButton = nullptr;
};

} // end of namespace KDevMI

#endif
