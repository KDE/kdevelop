/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ENVIRONMENTSELECTIONWIDGET_H
#define KDEVPLATFORM_ENVIRONMENTSELECTIONWIDGET_H

#include <QWidget>

#include "utilexport.h"

namespace KDevelop {
class EnvironmentProfileList;
class EnvironmentSelectionWidgetPrivate;

/**
 * Simple combobox which allows each plugin to decide which environment
 * variable group to use.
 *
 * Can be used just like a KComboBox in Configuration dialogs including usage
 * with KConfigXT.
 *
 * @note    The widget is populated and defaulted automatically.
 *
 */
class KDEVPLATFORMUTIL_EXPORT EnvironmentSelectionWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentProfile READ currentProfile WRITE setCurrentProfile NOTIFY currentProfileChanged USER true)

public:
    explicit EnvironmentSelectionWidget(QWidget* parent = nullptr);
    ~EnvironmentSelectionWidget() override;

    /**
     * @returns The currently selected environment profile name, as written to KConfigXT
     */
    QString currentProfile() const;

    /**
     * Sets the environment profile to be written to KConfigXT and updates the combo-box.
     *
     * @param text The environment profile name to select
     */
    void setCurrentProfile(const QString& text);

    /**
     * @returns The currently effective environment profile name (like @ref currentProfile(),
     *          but with empty value resolved to the default profile).
     */
    QString effectiveProfileName() const;

    /**
     * @returns The @ref EnvironmentProfileList which has been used to populate this
     *          widget.
     */
    EnvironmentProfileList environmentProfiles() const;

public Q_SLOTS:
    /**
     * Makes the widget re-read its environment group list.
     */
    void reconfigure();

Q_SIGNALS:
    void currentProfileChanged(const QString& currentProfile);

private:
    const QScopedPointer<class EnvironmentSelectionWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(EnvironmentSelectionWidget)
    friend class EnvironmentSelectionWidgetPrivate;
};

}

#endif
