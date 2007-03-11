/* KDevelop QMake Support
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

#ifndef QMAKEBUILDERPREFERENCES_H
#define QMAKEBUILDERPREFERENCES_H

#include "configmodule.h"

#include <kurl.h>
#include <kstandarddirs.h>

class QWidget;
class QStringList;

namespace Ui { class QMakeConfig; }

/**
 * @author Andreas Pakulat <apaku@gmx.de>
 */
class QMakeBuilderPreferences : public KDevelop::ConfigModule
{
public:
    QMakeBuilderPreferences(QWidget* parent = 0, const QStringList& args = QStringList());
    ~QMakeBuilderPreferences();

    virtual void load();
    virtual void save();
    virtual void defaults();

    virtual KUrl localNonShareableFile() const;


private slots:
    void settingsChanged();

private:
    Ui::QMakeConfig* m_prefsUi;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
