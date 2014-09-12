/* KDevelop Project Settings
 *
 * Copyright 2006  Matt Rogers <mattr@kde.org>
 * Copyright 2007  Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_BGPREFERENCES_H
#define KDEVPLATFORM_BGPREFERENCES_H

#include <kcmodule.h>

#include <kurl.h>

namespace Ui
{
class BGSettings;
}

namespace KDevelop
{

class BGPreferences : public KCModule
{
    Q_OBJECT
public:
    BGPreferences( QWidget *parent, const QVariantList &args );
    virtual ~BGPreferences();

    virtual void save();
    virtual void load();

private:
    Ui::BGSettings *preferencesDialog;

};

}
#endif
