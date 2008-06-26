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
 * \file  stoppingdialog.h
 *
 * \brief Declares class StoppingDialog.
 */

#ifndef VERITAS_STOPPINGDIALOG_H
#define VERITAS_STOPPINGDIALOG_H

#include "ui_stoppingdialog.h"

namespace Veritas
{

class RunnerModel;

/*!
 * \brief The StoppingDialog class tries to stop item execution.
 *
 * The stopping dialog instructs RunnerModel to stop item execution
 * without the dialog being visible on the screen. If not successful
 * at first attempt the dialog is shown to the user to give a visual
 * hint as well as a chance to interrupt the stopping.
 */

class StoppingDialog : public QDialog
{
    Q_OBJECT

public: // Operations

    /*!
     * Constructs a stopping dialog with the given \a parent and
     * \a model.
     */
    StoppingDialog(QWidget* parent, RunnerModel* model);

    /*!
     * Destroys this stopping dialog.
     */
    ~StoppingDialog();

    /*!
     * Reimplemented from QDialog. Returns \c QDialog::Accepted when
     * stopping was successful, otherwise \c QDialog::Rejected.
     */
    int exec();

private slots:

    /*!
     * Reimplemented from QDialog. Ensures that the dialog gets
     * closed properly.
     */
    void reject();

private: // Operations

    // Copy and assignment not supported.
    StoppingDialog(const StoppingDialog&);
    StoppingDialog& operator=(const StoppingDialog&);

private: // Attributes

    Ui::StoppingDialog ui;

    RunnerModel* m_model;

    bool m_shouldClose;
};

} // namespace

#endif // VERITAS_STOPPINGDIALOG_H
