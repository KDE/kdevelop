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

namespace KDevelop
{

/**
 * Simple compobox which allows each plugin to decide which environment
 * variable group to use.
 *
 * Can be used just like a KComboBox in Configuration dialogs including usage
 * with KConfigXT.
 *
 * Example code to populate the list:
 * \code
 * EnvironmentGroupList envlist( PluginSettings::self()->config() );
 * ui->kcfg_environmentGroup->addItems( envlist.groups()) );
 * \endcode
 *
 * The .kcfg file for that would include an entry like this:
 * \code
 * <entry name="environmentGroup" key="Make Environment Group" type="string">
 *   <default>default</default>
 * </entry>
 * \endcode
 *
 * It is important to list "default" as the default value, because that is currently
 * the only way to avoid an empty entry in the combo box.
 *
 */
class KDEVPLATFORMUTIL_EXPORT EnvironmentSelectionWidget : public KComboBox
{
    Q_OBJECT
    Q_PROPERTY( QString currentProfile READ currentProfile WRITE setCurrentProfile USER true )

public:
    explicit EnvironmentSelectionWidget( QWidget *parent = 0 );
    ~EnvironmentSelectionWidget();

    /**
     * Return the currently selected text as special property so this widget
     * works with KConfigXT
     * @returns the currently selected items text
     */
    QString currentProfile() const;

    /**
     * Setter for the KConfigXT property
     * @param text the item text which should be set as current.
     */
    void setCurrentProfile( const QString& text );

private:
    class EnvironmentSelectionWidgetPrivate* const d;
    friend class EnvironmentSelectionWidgetPrivate;

};

}

#endif
