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
 * \file  runner.h
 *
 * \brief Declares class Runner.
 */

#ifndef RUNNER_H
#define RUNNER_H

#include "qxrunner_global.h"
#include "qxrunnerexport.h"

class QIcon;

namespace QxRunner
{

class RunnerModel;

/*!
 * \brief The Runner class starts a QxRunner application.
 *
 * This class creates the main window and shows it on screen. When
 * the main window closes QApplication also quits.
 *
 * The runner can be given an application icon, typically displayed in
 * the top-left corner of windows. This icon overrides the QxRunner
 * default icon. Not to be confused with the icon of the executable
 * application file itself as presented on the desktop. To change this
 * it is necessary to employ a platform-dependent technique as
 * described in the Qt documentation.
 */

class QXRUNNER_EXPORT Runner
{
public: // Operations

    /*!
     * Constructs a runner with the given \a model.
     */
    Runner(RunnerModel* model);

    /*!
     * Destroys this runner.
     *
     * \note
     * Deleting the model provided at construction time is left to the
     * owner of the model instance.
     */
    virtual ~Runner();

    /*!
     * Shows the main window.
     */
    void run();

    /*!
     * Sets the application \a icon.
     */
    void setWindowIcon(const QIcon& icon);

private: // Operations

    // Copy and assignment not supported.
    Runner(const Runner&);
    Runner& operator=(const Runner&);

private: // Attributes

    RunnerModel* m_model;

    QIcon* m_icon;
};

} // namespace

#endif // RUNNER_H
