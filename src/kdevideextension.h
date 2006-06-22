/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef KDEVIDEEXTENSION_H
#define KDEVIDEEXTENSION_H

#include <shellextension.h>
#include "shellexport.h"
class KDialog;
class SettingsWidget;

class KDEVSHELL_EXPORT KDevIDEExtension : public ShellExtension {
public:
    static void init();

    virtual void createGlobalSettingsPage(KPageDialog *dlg);
    virtual void acceptGlobalSettingsPage(KPageDialog *dlg);

    virtual QString xmlFile();

    virtual QString defaultProfile();

protected:
    KDevIDEExtension();

private:
    SettingsWidget *gsw;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
