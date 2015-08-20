/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CHECKER_STATUS_H
#define CHECKER_STATUS_H

#include <shell/shellexport.h>

#include <interfaces/istatus.h>

struct CheckerStatusPrivate;

namespace KDevelop
{

// Status / Progress reporter for checker tools
class KDEVPLATFORMSHELL_EXPORT CheckerStatus : public QObject, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)
public:
    CheckerStatus();
    ~CheckerStatus();

    virtual QString statusName() const override;

    // Sets the name of the checker tool
    void setCheckerName(const QString &name);

    // Sets the maximum number of items that will be checked
    void setMaxItems(int maxItems);

    // Increases the number of checked items
    void itemChecked();

    // Starts status / progress reporting
    void start();

    // Stops status / progress reporting
    void stop();

signals:
    void clearMessage(KDevelop::IStatus*) override;
    void showMessage(KDevelop::IStatus*, const QString &message, int timeout = 0) override;
    void showErrorMessage(const QString & message, int timeout = 0) override;
    void hideProgress(KDevelop::IStatus*) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;

private:
    QScopedPointer<CheckerStatusPrivate> d;
};

}

#endif
