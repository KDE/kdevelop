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
#ifndef ENVIRONMENTSELECTIONWIDGET_H
#define ENVIRONMENTSELECTIONWIDGET_H

#include <kcombobox.h>
#include "utilexport.h"

namespace KDevelop
{

/**
 * Simple compobox which allows each plugin to decide which environment
 * variable group to use.
 *
 * At construction, it shows all the environment variable group configurations
 * which was set at project-wide environment setting page. User will select
 * one group among them, and selected groupname will be saved in project config file.
 *
 * @TODO: Make this compatible with KConfigXT.
 * Refer to KDevelop/MakeBuilderPreferences and its .ui file to see how to use this.
 */
class KDEVPLATFORMUTIL_EXPORT EnvironmentSelectionWidget : public KComboBox
{
    Q_OBJECT
    Q_PROPERTY( QString currentProfile READ currentProfile WRITE setCurrentProfile USER true )

public:
    explicit EnvironmentSelectionWidget( QWidget *parent = 0 );
    ~EnvironmentSelectionWidget();

    QString currentProfile() const;
    void setCurrentProfile( const QString& text );

private:
    class EnvironmentSelectionWidgetPrivate* const d;
    friend class EnvironmentSelectionWidgetPrivate;

};

}

#endif
