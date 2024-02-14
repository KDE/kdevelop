/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSSELECTION_H
#define PROCESSSELECTION_H

#include "ui_processselection.h"

#include <QDialog>

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
    explicit ProcessSelectionDialog(QWidget* parent = nullptr);
    ~ProcessSelectionDialog() override;
    long long pidSelected() const;

private Q_SLOTS:
    void selectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection);
    void onProcessesComboActivated(int index);

private:
    Ui::ProcessSelection m_ui;

    KSysGuard::ProcessDataModel* m_dataModel = nullptr;
    ProcessesSortFilterModel* m_sortModel = nullptr;
    int m_pidColumn = -1;

    QPushButton* m_attachButton = nullptr;
};

} // end of namespace KDevMI

#endif
