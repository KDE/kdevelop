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

#include "texttoolspart.h"

#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ktexteditor/editinterface.h>

#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"

#include "texttoolswidget.h"


typedef KGenericFactory<TextToolsPart> TextToolsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevtexttools, TextToolsFactory( "kdevtexttools" ) );

TextToolsPart::TextToolsPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(parent, name ? name : "TextToolsPart")
{
    setInstance(TextToolsFactory::instance());
    //    setXMLFile("kdevfileview.rc");

    m_widget = 0;
    
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)) );
}


TextToolsPart::~TextToolsPart()
{
    if (m_widget)
        mainWindow()->removeView(m_widget);
    delete m_widget;
}


void TextToolsPart::createWidget()
{
    if (m_widget)
        return;
    
    m_widget = new TextToolsWidget(this);
    m_widget->setCaption(i18n("Text Structure"));
    QWhatsThis::add(m_widget, i18n("Text Structure\n\n"
                                     "This browser shows the structure of your HTML text."));
    mainWindow()->embedSelectView(m_widget, i18n("Text Structure"), i18n("text structure"));
}


void TextToolsPart::activePartChanged(KParts::Part *part)
{
    if (m_widget)
        m_widget->stop();

    if (!part)
        return;
    KParts::ReadWritePart *rwpart = dynamic_cast<KParts::ReadWritePart*>(part);
    if (!rwpart)
        return;
    QString url = rwpart->url().url();

    if (url.endsWith(".html")) {
        kdDebug(9030) << "set mode html" << endl;
        createWidget();
        m_widget->setMode(TextToolsWidget::HTML, rwpart);
    } else if (url.endsWith(".docbook")) {
        kdDebug(9030) << "set mode Docbook" << endl;
        createWidget();
        m_widget->setMode(TextToolsWidget::Docbook, rwpart);
    } else if (url.endsWith(".tex")) {
        kdDebug(9030) << "set mode LaTeX" << endl;
        createWidget();
        m_widget->setMode(TextToolsWidget::LaTeX, rwpart);
    }
}

#include "texttoolspart.moc"
