/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qlineedit.h>
#include <qtextedit.h>

#include "generalinfowidget.h"
#include "domutil.h"

GeneralInfoWidget::GeneralInfoWidget(QDomDocument &projectDom, QWidget *parent, const char *name)
        : GeneralInfoWidgetBase(parent, name), m_projectDom(projectDom) {
    
    readConfig();
}



GeneralInfoWidget::~GeneralInfoWidget() {}

void GeneralInfoWidget::readConfig() {
    this->author_edit->setText(DomUtil::readEntry(m_projectDom,"/general/author"));
    this->email_edit->setText(DomUtil::readEntry(m_projectDom,"/general/email"));    
    this->version_edit->setText(DomUtil::readEntry(m_projectDom,"/general/version"));
    this->description_edit->setText(DomUtil::readEntry(m_projectDom,"/general/description"));
}

void GeneralInfoWidget::writeConfig() {
    DomUtil::writeEntry(m_projectDom,"/general/author",author_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/email",email_edit->text());    
    DomUtil::writeEntry(m_projectDom,"/general/version",version_edit->text());    
    DomUtil::writeEntry(m_projectDom,"/general/description",description_edit->text());    
}

void GeneralInfoWidget::accept() {
    writeConfig();   
}
