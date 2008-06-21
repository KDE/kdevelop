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
 * \file  checkmodel.h
 *
 * \brief Declares class CheckModel.
 */

#ifndef QXCHECK_CHECKMODEL_H
#define QXCHECK_CHECKMODEL_H

#include <qxrunner/runnermodel.h>

namespace QxCheck
{

/*!
 * \brief The CheckModel class maintains CppUnit data and executes
 *        CppUnit unit tests.
 *
 * This class stores CppUnit test data using CppUnitItem objects. The
 * CppUnit tests referenced by the CppUnitItem objects can be executed.
 * Test results are prepared for representation in views.
 *
 * The expected results from this model are:
 *
 */

class CheckModel : public QxRunner::RunnerModel
{
    Q_OBJECT

public: // Operations

    CheckModel(QObject* parent = 0);
    ~CheckModel();

    void readTests(const QFileInfo& exe);

    QString name() const;
    QString about() const;
};

} // namespace

#endif // CHECKMODEL_H
