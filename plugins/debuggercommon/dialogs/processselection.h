/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSSELECTION_H
#define PROCESSSELECTION_H

#include <QDialog>

class KSysGuardProcessList;
class QItemSelection;
class QPushButton;

namespace KDevMI {

class ProcessSelectionDialog : public QDialog
{
    Q_OBJECT
    public:
        explicit ProcessSelectionDialog( QWidget *parent=nullptr );
        ~ProcessSelectionDialog() override;
        long int pidSelected();
        QSize sizeHint() const override;

    private Q_SLOTS:
        void selectionChanged( const QItemSelection& selected );

    private:
        KSysGuardProcessList* m_processList;
        QPushButton* m_attachButton;
};

} // end of namespace KDevMI

#endif
