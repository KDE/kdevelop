/***************************************************************************
                             appoutputwidget.cpp
                             -------------------                                         

    copyright            : (C) 1999 The KDevelop Team
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <klocale.h>
#include "outputviews.h"
#include "appoutputwidget.h"


AppOutputWidget::AppOutputWidget()
    : ProcessView(0, "app output widget")
{}


AppOutputWidget::~AppOutputWidget()
{}


void AppOutputWidget::childFinished(bool normal, int status)
{
    QString s;
    ProcessListBoxItem::Type t;
    
    if (normal) {
        if (status) {
            s = i18n("*** Exited with status: %1 ***").arg(status);
            t = ProcessListBoxItem::Error;
        } else {
            s = i18n("*** Exited normally ***");
            t = ProcessListBoxItem::Diagnostic;
        }
    } else {
        s = i18n("*** Process aborted ***");
        t = ProcessListBoxItem::Error;
    }
    
    insertItem(new ProcessListBoxItem(s, t));
}


void AppOutputWidget::compilationAborted()
{
    if (isRunning()) {
        killJob();
        insertItem(i18n("Killed process."));
    }
}
