/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makeviewpart.h"

#include <qwhatsthis.h>
#include <qdir.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "kdevproject.h"
#include "kdevcore.h"
#include "kdevtoplevel.h"

#include "outputviewsfactory.h"
#include "makewidget.h"


MakeViewPart::MakeViewPart(QObject *parent, const char *name, const QStringList &)
    : KDevMakeFrontend(parent, name)
{
    setInstance(OutputViewsFactory::instance());
    
    setXMLFile("kdevmakeview.rc");

    m_dcop = new KDevMakeFrontendIface(this);

    m_widget = new MakeWidget(this);
    m_widget->setCaption(i18n("Messages Output"));
    QWhatsThis::add(m_widget, i18n("Messages output\n\n"
                                   "The messages window shows the output of the compiler and "
                                   "used utilities like kdoc reference documentation. "
                                   "For compiler error messages, click on the error message. "
                                   "This will automatically open the source file and set the "
                                   "cursor to the line that caused the compiler error/warning."));

    topLevel()->embedOutputView(m_widget, i18n("Messages"), i18n("compiler output messages"));

    KAction *action;
    action = new KAction( i18n("&Next Error"), Key_F4, m_widget, SLOT(nextError()),
                          actionCollection(), "view_next_error");
    action->setStatusText( i18n("Switches to the file and line where the next error was reported from") );
    action = new KAction( i18n("&Previous Error"), SHIFT+Key_F4, m_widget, SLOT(prevError()),
                          actionCollection(), "view_previous_error");
    action->setStatusText( i18n("Switches to the file and line where the previous error was reported from") );

    connect( core(), SIGNAL(stopButtonClicked()),
             m_widget, SLOT(killJob()) );
}


MakeViewPart::~MakeViewPart()
{
    delete m_widget;
    delete m_dcop;
}


void MakeViewPart::queueCommand(const QString &dir, const QString &command)
{
    m_widget->queueJob(dir, command);
}


bool MakeViewPart::isRunning()
{
    return m_widget->isRunning();
}

#include "makeviewpart.moc"
