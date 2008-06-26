/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*!
 * \file  stoppingdialog.cpp
 *
 * \brief Implements class StoppingDialog.
 */

#include "stoppingdialog.h"
#include "runnermodel.h"

namespace Veritas
{

StoppingDialog::StoppingDialog(QWidget* parent, RunnerModel* model)
        : QDialog(parent, Qt::WindowTitleHint      |
                  Qt::WindowSystemMenuHint |
                  Qt::MSWindowsFixedSizeDialogHint),
        m_model(model)
{
    ui.setupUi(this);

    connect(ui.buttonCancel, SIGNAL(clicked()), SLOT(reject()));

    m_shouldClose = false;
}

StoppingDialog::~StoppingDialog()
{

}

int StoppingDialog::exec()
{
    bool stopped = m_model->stopItems();

    if (stopped) {
        return QDialog::Accepted;
    }

    // If not successful at first attempt then show the dialog.
    adjustSize();
    show();

    while (!(stopped || m_shouldClose)) {
        // Prevent GUI from freezing.
        QCoreApplication::processEvents();

        ui.progressBar->setValue(ui.progressBar->value() + 1);

        stopped = m_model->stopItems();

        if (ui.progressBar->value() >= ui.progressBar->maximum()) {
            ui.progressBar->setValue(ui.progressBar->minimum());
        }
    }

    if (stopped) {
        return QDialog::Accepted;
    } else {
        return QDialog::Rejected;
    }
}

void StoppingDialog::reject()
{
    // Notify the exec() loop to end because the dialog gets closed.
    m_shouldClose = true;

    QDialog::reject();
}

} // namespace
