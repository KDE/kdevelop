/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qwhatsthis.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "scriptprojectwidget.h"
#include "scriptprojectpart.h"

typedef KGenericFactory<ScriptProjectPart> ScriptProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevscriptproject, ScriptProjectFactory( "kdevscriptproject" ) );

ScriptProjectPart::ScriptProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject(parent, name)
{
    setInstance(ScriptProjectFactory::instance());

    //    setXMLFile("kdevscriptproject.rc");

    m_widget = new ScriptProjectWidget();
    m_widget->setIcon(SmallIcon("make"));
    m_widget->setCaption(i18n("Project"));
    
    topLevel()->embedSelectView(m_widget, i18n("Project"));

    connect( m_widget, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
}


ScriptProjectPart::~ScriptProjectPart()
{
    topLevel()->removeView(m_widget);
    delete m_widget;
}


void ScriptProjectPart::openProject(const QString &dirName)
{
    m_widget->openProject(dirName);
}


void ScriptProjectPart::closeProject()
{
    m_widget->closeProject();
}


QString ScriptProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevscriptproject/run/mainprogram");
}


QString ScriptProjectPart::projectDirectory()
{
    return m_widget->projectDirectory();
}


QStringList ScriptProjectPart::allSourceFiles()
{
    return m_widget->allSourceFiles();
}


void ScriptProjectPart::slotItemExecuted(QListViewItem *item)
{
    ScriptProjectItem *spitem = static_cast<ScriptProjectItem*>(item);
    if (spitem->type() == ScriptProjectItem::File)
        partController()->editDocument(KURL(spitem->path()));
}

#include "scriptprojectpart.moc"
