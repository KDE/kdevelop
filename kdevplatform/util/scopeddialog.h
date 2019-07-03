/* This file is part of KDevelop
 *
 * Copyright 2017 Christoph Roick <chrisito@gmx.de>
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

#ifndef KDEVPLATFORM_SCOPEDDIALOG_H
#define KDEVPLATFORM_SCOPEDDIALOG_H

#include <QPointer>

namespace KDevelop {

/**
 * Wrapper class for QDialogs which should not be instantiated on stack.
 *
 * Parents of QDialogs may be unintentionally deleted during the execution of the
 * dialog and automatically delete their children. When returning to the calling
 * function they get intentionally deleted again, which will lead to a crash. This
 * can be circumvented by using a QPointer which keeps track of the QDialogs validity.
 * See this
 * <a href="https://blogs.kde.org/2009/03/26/how-crash-almost-every-qtkde-application-and-how-fix-it-0">blog entry</a>
 * for explanation. The ScopedDialog utility allows using the dialog like a
 * common pointer.
 *
 * Instead of
 * \code
   QFileDialog dlg(this);
   if (dlg.exec())
       return;
  \endcode
  simply use
 * \code
   ScopedDialog<QFileDialog> dlg(this);
   if (dlg->exec())
       return;
  \endcode
  without need to manually clean up afterwards.
 */
template<typename DialogType>
class ScopedDialog
{
public:
    /// Construct the dialog with any set of allowed arguments
    /// for the construction of DialogType
    template<typename ... Arguments>
    explicit ScopedDialog(Arguments ... args) : ptr(new DialogType(args ...))
    {
    }
    /// Automatically deletes the dialog if it is still present
    ~ScopedDialog()
    {
        delete ptr;
    }

    /// Access the raw pointer to the dialog
    DialogType* data() const
    {
        return ptr;
    }
    /// Access members of the dialog
    DialogType* operator->() const
    {
        return ptr;
    }
    /// Access the dialog
    DialogType& operator*() const
    {
        return *ptr;
    }
    /// Return the corresponding pointer
    operator DialogType*() const {
        return ptr;
    }

private:
    Q_DISABLE_COPY(ScopedDialog)

    QPointer<DialogType> ptr;
};

}

#endif // KDEVPLATFORM_SCOPEDDIALOG_H
