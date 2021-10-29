/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WORKINGSETHELPERS_H
#define KDEVPLATFORM_WORKINGSETHELPERS_H

#include <QSet>

namespace KDevelop {

class MainWindow;

//returns the active MainWindow
MainWindow* mainWindow();

//Filters the views in the main-window so they only contain the given files to keep
void filterViews(const QSet< QString >& keepFiles);

}

#endif // KDEVPLATFORM_WORKINGSETHELPERS_H
