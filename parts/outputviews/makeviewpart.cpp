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
#define GIDEON

#include <qwhatsthis.h>
#include <qdir.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "kdevproject.h"
#include "kdevcore.h"
#include "outputviewsfactory.h"
#include "makewidget.h"
#include "makeviewpart.h"


MakeViewPart::MakeViewPart(KDevApi *api, QObject *parent, const char *name)
    : KDevMakeFrontend(api, parent, name)
{
#ifndef GIDEON
    setInstance(OutputFactory::instance());
#endif
    
    setXMLFile("kdevmakeview.rc");

    m_dcop = new KDevMakeFrontendIface(this);

    m_widget = new MakeWidget(this);
    m_widget->setCaption(i18n("Messages output"));
    QWhatsThis::add(m_widget, i18n("Messages output\n\n"
                                   "The messages window shows the output of the compiler and "
                                   "used utilities like kdoc reference documentation. "
                                   "For compiler error messages, click on the error message. "
                                   "This will automatically open the source file and set the "
                                   "cursor to the line that caused the compiler error/warning."));

    core()->embedWidget(m_widget, KDevCore::OutputView, i18n("Messages"));

    KAction *action;
    action = new KAction( i18n("&Next error"), Key_F4, m_widget, SLOT(nextError()),
                          actionCollection(), "view_next_error");
    action->setStatusText( i18n("Switches to the file and line the next error was reported") );
    action = new KAction( i18n("&Previous error"), SHIFT+Key_F4, m_widget, SLOT(prevError()),
                          actionCollection(), "view_previous_error");
    action->setStatusText( i18n("Switches to the file and line the previous error was reported") );


    connect( core(), SIGNAL(stopButtonClicked()),
             m_widget, SLOT(killJob()) );
}


MakeViewPart::~MakeViewPart()
{
    delete m_widget;
    delete m_dcop;
}


void MakeViewPart::startMakeCommand(const QString &dir, const QString &command)
{
#if 0
    // We allow non-project related make commands, for example
    // for the appwizard
    if (!project()) {
        kdDebug(9004) << "MakeViewPart: compilation started with project?" << endl;
        return;
    }
#endif
    //    QString dir = project()? project()->projectDirectory() : QString::null;

    m_widget->startJob(dir, command);
}


bool MakeViewPart::isRunning()
{
    return m_widget->isRunning();
}

#include "makeviewpart.moc"
