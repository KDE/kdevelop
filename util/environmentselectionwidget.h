/* This file is part of KDevelop
Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPLATFORM_ENVIRONMENTSELECTIONWIDGET_H
#define KDEVPLATFORM_ENVIRONMENTSELECTIONWIDGET_H

#include <kcombobox.h>

#include "utilexport.h"
#include "environmentgrouplist.h"

namespace KDevelop
{

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
    Q_PROPERTY( QString currentProfile READ currentProfile WRITE setCurrentProfile USER true )

public:
    explicit EnvironmentSelectionWidget( QWidget *parent = 0 );
    ~EnvironmentSelectionWidget();

    /**
     * @returns The currently selected environment profile name, as written to KConfigXT
     */
    QString currentProfile() const;

    /**
     * Sets the environment profile to be written to KConfigXT and updates the combo-box.
     *
     * @param text The environment profile name to select
     */
    void setCurrentProfile( const QString& text );

    /**
     * @returns The currently effective environment profile name (like @ref currentProfile(),
     *          but with empty value resolved to the default profile).
     */
    QString effectiveProfileName() const;

    /**
     * @returns The @ref EnvironmentGroupList which has been used to populate this
     *          widget.
     */
    EnvironmentGroupList environment() const;

public slots:
    /**
     * Makes the widget re-read its environment group list.
     */
    void reconfigure();

private:
    struct EnvironmentSelectionWidgetPrivate* const d;
    friend struct EnvironmentSelectionWidgetPrivate;
};

}

#endif
