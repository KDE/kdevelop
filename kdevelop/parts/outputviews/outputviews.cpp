/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qwhatsthis.h>
#include <qdir.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "projectspace.h"
#include "main.h"
#include "makewidget.h"
#include "appoutputwidget.h"
#include "outputviews.h"


MakeView::MakeView(QObject *parent, const char *name)
    : KDevMakeFrontend(parent, name)
{
    setInstance(OutputFactory::instance());
    setXMLFile("kdevmakeview.rc");

    m_widget = 0;
    m_dcop = new KDevMakeFrontendIface(this);
}


MakeView::~MakeView()
{
    delete m_widget;
    delete m_dcop;
}


void MakeView::setupGUI()
{
    kdDebug(9004) << "Building MakeWidget" << endl;

    m_widget = new MakeWidget(this);
    m_widget->setCaption(i18n("Messages output"));
    QWhatsThis::add(m_widget, i18n("Messages output\n\n"
                                   "The messages window shows the output of the compiler and "
                                   "used utilities like kdoc reference documentation. "
                                   "For compiler error messages, click on the error message. "
                                   "This will automatically open the source file and set the "
                                   "cursor to the line that caused the compiler error/warning."));

    emit embedWidget(m_widget, OutputView, i18n("Messages"), i18n("messages output view"));

    KAction *action;
    action = new KAction( i18n("&Next error"), Key_F4, m_widget, SLOT(nextError()),
                          actionCollection(), "view_next_error");
    action->setStatusText( i18n("Switches to the file and line the next error was reported") );
    action = new KAction( i18n("&Previous error"), SHIFT+Key_F4, m_widget, SLOT(prevError()),
                          actionCollection(), "view_previous_error");
    action->setStatusText( i18n("Switches to the file and line the previous error was reported") );
}


void MakeView::executeMakeCommand(const QString &command)
{
    if (!projectSpace()) {
        kdDebug(9004) << "MakeView: compilation started with project?" << endl;
        return;
    }
    
    m_widget->startJob(projectSpace()->absolutePath(), command);
}


AppOutputView::AppOutputView(QObject *parent, const char *name)
    : KDevAppFrontend(parent, name)
{
    setInstance(OutputFactory::instance());
}


AppOutputView::~AppOutputView()
{
    delete m_widget;
}


void AppOutputView::setupGUI()
{
    kdDebug(9004) << "Building AppOutputWidget" << endl;

    m_widget = new AppOutputWidget();
    m_widget->setCaption(i18n("Application output"));
    QWhatsThis::add(m_widget, i18n("Application output\n\n"
                                   "The stdout/stderr output window is a replacement for "
                                   "terminal-based application communication. Running terminal "
                                   "applications are using this instead of a terminal window."));

    emit embedWidget(m_widget, OutputView, i18n("Application"), i18n("application output view"));
}


void AppOutputView::executeAppCommand(const QString &command)
{
    m_widget->startJob(QDir::homeDirPath(), command);
}


void AppOutputView::stopButtonClicked()
{
    m_widget->stopButtonClicked();
}

#include "outputviews.moc"
