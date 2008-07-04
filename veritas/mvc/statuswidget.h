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
 * \file  statuswidget.h
 *
 * \brief Declares class StatusWidget.
 */

#ifndef VERITAS_STATUSWIDGET_H
#define VERITAS_STATUSWIDGET_H

#include "ui_statuswidget.h"

namespace Veritas
{

/*!
 * \brief The StatusWidget class presents Veritas status information.
 *
 * The information mainly consists of counters such as number of items,
 * number of errrors, execution progress and the like. Actual values
 * must be inserted by clients. The status widget is placed in the
 * statusbar of the main window.
 *
 * \sa \ref main_window
 */

class StatusWidget : public QWidget
{
    Q_OBJECT

public: // Operations

    /*!
     * Constructs a status widget with the given \a parent.
     */
    StatusWidget(QWidget* parent);

    /*!
     * Destroys this status widget.
     */
    ~StatusWidget();

public: // Attributes

    Ui::StatusWidget ui;

private: // Operations

    // Copy and assignment not supported.
    StatusWidget(const StatusWidget&);
    StatusWidget& operator=(const StatusWidget&);
};

} // namespace

#endif // VERITAS_STATUSWIDGET_H
