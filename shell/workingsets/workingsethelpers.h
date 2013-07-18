/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef KDEVPLATFORM_WORKINGSETHELPERS_H
#define KDEVPLATFORM_WORKINGSETHELPERS_H

#include <QSet>

namespace KDevelop {

class MainWindow;

//returns the active MainWindow
MainWindow* mainWindow();

//Filters the views in the main-window so they only contain the given files to keep
void filterViews(QSet< QString > keepFiles);

}

#endif // KDEVPLATFORM_WORKINGSETHELPERS_H
