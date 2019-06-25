/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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

namespace KDevelop
{

class CheckerStatusPrivate;

/**
* Status / Progress reporter for checker tools. It shows a progress bar as more and more items are checked.
* As part of initialization the max. number of items have to be set, and then when an item is checked that has to be indicated to the class. When stopped the progressbar first filled up to max, then it disappears.
*
* Initialization:
* @code
* m_status = new CheckerStatus();
* m_status->setCheckerName(QStringLiteral("SomeChecker"));
* ICore::self()->uiController()->registerStatus(m_status);
* @endcode
*
* Starting:
* @code
* m_status->setMaxItems(9001);
* m_status->start();
* @endcode
*
* Stopping:
* @code
* m_status->stop();
* @endcode
*
* Showing progress:
* @code
* m_status->itemChecked();
* @endcode
*/
class KDEVPLATFORMSHELL_EXPORT CheckerStatus : public QObject, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)
public:
    CheckerStatus();
    ~CheckerStatus() override;

    QString statusName() const override;

    /// Sets the name of the checker tool
    void setCheckerName(const QString &name);

    /// Sets the maximum number of items that will be checked
    void setMaxItems(int maxItems);

    /// Increases the number of checked items
    void itemChecked();

    /// Starts status / progress reporting
    void start();

    /// Stops status / progress reporting
    void stop();

Q_SIGNALS:
    void clearMessage(KDevelop::IStatus*) override;
    void showMessage(KDevelop::IStatus*, const QString &message, int timeout = 0) override;
    void showErrorMessage(const QString & message, int timeout = 0) override;
    void hideProgress(KDevelop::IStatus*) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;

private:
    const QScopedPointer<class CheckerStatusPrivate> d_ptr;
    Q_DECLARE_PRIVATE(CheckerStatus)
};

}

#endif
