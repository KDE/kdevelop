/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classviewconfigwidget.h"

#include <qradiobutton.h>
#include <qspinbox.h>
#include "domutil.h"
#include "classviewpart.h"


ClassViewConfigWidget::ClassViewConfigWidget(ClassViewPart *part,
                                             QWidget *parent, const char *name)
    : ClassViewConfigWidgetBase(parent, name)
{
    m_part = part;
    
    readConfig();
}


ClassViewConfigWidget::~ClassViewConfigWidget()
{}


void ClassViewConfigWidget::readConfig()
{
    QDomDocument &dom = *m_part->projectDom();
    bool foldersAsHierarchy = DomUtil::readBoolEntry(dom, "/kdevclassview/folderhierarchy", true);
    
    if (foldersAsHierarchy) {
        hierarchyButton->setChecked(true);
        depthBox->setEnabled(false);
    } else {
        int depth = DomUtil::readIntEntry(dom, "/kdevclassview/depthoffolders");
        if (depth == 0)
            depth = 2;
        depthBox->setValue(depth);
    }
}


void ClassViewConfigWidget::storeConfig()
{
    QDomDocument &dom = *m_part->projectDom();

    DomUtil::writeBoolEntry(dom, "/kdevclassview/folderhierarchy", hierarchyButton->isChecked());
    DomUtil::writeIntEntry(dom, "/kdevclassview/depthoffolders", depthBox->value());
}


void ClassViewConfigWidget::accept()
{
    storeConfig();
    emit m_part->configChange();
}

#include "classviewconfigwidget.moc"
