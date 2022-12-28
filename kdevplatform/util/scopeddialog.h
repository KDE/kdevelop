/*
    SPDX-FileCopyrightText: 2017 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SCOPEDDIALOG_H
#define KDEVPLATFORM_SCOPEDDIALOG_H

#include <QPointer>

#include <utility>

namespace KDevelop {

/// A tag that requests construction of a null ScopedDialog, avoids ambiguity with the variadic template constructor.
struct NullScopedDialog
{
    // The explicit default constructor ensures that ambiguous code `ScopedDialog<QDialog> d{{}};` does not compile.
    explicit constexpr NullScopedDialog() noexcept = default;
};

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
// This class template is final, because inheriting it shouldn't be useful, and
// the forwarding-reference constructor would be invoked in places where slicing
// normally occurs.
template<typename DialogType>
class ScopedDialog final
{
    Q_DISABLE_COPY_MOVE(ScopedDialog)
public:
    // Explicitly delete unconventional overloads of copy and move constructors
    // to prevent a compiler from using the forwarding-reference constructor in
    // places where a copy or a move constructor are normally invoked.
    ScopedDialog(ScopedDialog&) = delete;
    ScopedDialog(const ScopedDialog&&) = delete;

    /// Construct the dialog with any set of allowed arguments
    /// for the construction of DialogType
    template<typename ... Arguments>
    explicit ScopedDialog(Arguments&& ... args)
        : ptr{new DialogType(std::forward<Arguments>(args)...)}
    {
    }
    /// Automatically deletes the dialog if it is still present
    ~ScopedDialog()
    {
        delete ptr;
    }

    /// Construct a null scoped dialog, i.e. don't create a DialogType but initialize the dialog pointer with nullptr
    explicit ScopedDialog(NullScopedDialog)
    {
    }
    /// Destroy the previous dialog if present, then create a dialog with arguments accepted by DialogType()
    template<typename... Arguments>
    void assign(Arguments&&... args)
    {
        delete ptr;
        ptr = new DialogType(std::forward<Arguments>(args)...);
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
    QPointer<DialogType> ptr;
};

}

#endif // KDEVPLATFORM_SCOPEDDIALOG_H
