/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    auto* window = qobject_cast<MainWindow*>(Core::self()->uiController()->activeMainWindow());
    Q_ASSERT(window);
    return window;
}

void filterViews(const QSet< QString >& keepFiles)
{
    MainWindow* window = mainWindow();

    const auto views = window->area()->views();
    for (Sublime::View* view : views) {

        auto* partDoc = qobject_cast<PartDocument*>(view->document());
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
