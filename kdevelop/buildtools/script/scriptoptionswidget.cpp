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

#include "scriptoptionswidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include "domutil.h"
#include "kdevlanguagesupport.h"


ScriptOptionsWidget::ScriptOptionsWidget(KDevPlugin *part,
                                         QWidget *parent, const char *name)
    : ScriptOptionsWidgetBase(parent, name)
{
    m_part = part;
    
    QDomDocument &dom = *m_part->projectDom();

    QString includepatterns
        = DomUtil::readEntry(dom, "/kdevscriptproject/general/includepatterns");
    
    if (includepatterns.isNull() && part->languageSupport()){
	QStringList includepatternList; 
	KMimeType::List list = part->languageSupport()->mimeTypes();
	KMimeType::List::Iterator it = list.begin();
	while( it != list.end() ){
	    includepatternList += (*it)->patterns();
	    ++it;
	}
	includepatterns = includepatternList.join( "," );
    }
    
    QString excludepatterns
        = DomUtil::readEntry(dom, "/kdevscriptproject/general/excludepatterns");
    if (excludepatterns.isNull())
        excludepatterns = "*~";
    
    includepatterns_edit->setText(includepatterns);
    excludepatterns_edit->setText(excludepatterns);
}


ScriptOptionsWidget::~ScriptOptionsWidget()
{}


void ScriptOptionsWidget::accept()
{
    QDomDocument &dom = *m_part->projectDom();

    QString includepatterns = includepatterns_edit->text();
    QString excludepatterns = excludepatterns_edit->text();
    
    DomUtil::writeEntry(dom, "/kdevscriptproject/general/includepatterns", includepatterns);
    DomUtil::writeEntry(dom, "/kdevscriptproject/general/excludepatterns", excludepatterns);
}

#include "scriptoptionswidget.moc"
