/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef KDEVPLATFORM_UIPREFERENCES_H
#define KDEVPLATFORM_UIPREFERENCES_H

#include "configpage.h"

namespace Ui{
    class UiConfig;
}

/**
 @author Andreas Pakulat <apaku@gmx.de>
*/

class UiPreferences : public KDevelop::ConfigPage
{
public:
    explicit UiPreferences(QWidget* parent = nullptr);

    ~UiPreferences();

    virtual QString name() const override;
    virtual QString fullName() const override;
    virtual QIcon icon() const override;

public Q_SLOTS:
    // need to customze behaviour
    virtual void apply() override;
private:
    Ui::UiConfig* m_uiconfigUi;
};

#endif

