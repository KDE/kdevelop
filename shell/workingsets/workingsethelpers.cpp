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

#include "workingsethelpers.h"

#include <interfaces/iuicontroller.h>

#include <sublime/view.h>
#include <sublime/area.h>

#include <shell/core.h>

#include "partdocument.h"
#include "mainwindow.h"

namespace KDevelop {

MainWindow* mainWindow() {
    MainWindow* window = dynamic_cast<MainWindow*>(Core::self()->uiController()->activeMainWindow());
    Q_ASSERT(window);
    return window;
}

void filterViews(QSet< QString > keepFiles)
{
    MainWindow* window = mainWindow();

    foreach(Sublime::View* view, window->area()->views()) {

        PartDocument* partDoc = dynamic_cast<PartDocument*>(view->document());
        if(partDoc && !keepFiles.contains(partDoc->documentSpecifier())) {
            if(view->document()->views().count() == 1) {
                partDoc->close();
                continue;
            }

            window->area()->closeView(view);
        }
    }
}

}
