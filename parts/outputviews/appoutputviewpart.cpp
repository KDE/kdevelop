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

#include <qwhatsthis.h>
#include <qdir.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "kdevproject.h"
#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "outputviewsfactory.h"
#include "appoutputwidget.h"
#include "appoutputviewpart.h"


AppOutputViewPart::AppOutputViewPart(QObject *parent, const char *name, const QStringList &)
    : KDevAppFrontend(parent, name)
{
    setInstance(OutputViewsFactory::instance());

    m_dcop = new KDevAppFrontendIface(this);

    m_widget = new AppOutputWidget();
    m_widget->setCaption(i18n("Application Output"));
    QWhatsThis::add(m_widget, i18n("Application output\n\n"
                                   "The stdout/stderr output window is a replacement for "
                                   "terminal-based application communication. Running terminal "
                                   "applications are using this instead of a terminal window."));

    topLevel()->embedOutputView(m_widget, i18n("Application"));

    connect( core(), SIGNAL(stopButtonClicked()),
             m_widget, SLOT(killJob()) );
}


AppOutputViewPart::~AppOutputViewPart()
{
    delete m_widget;
    delete m_dcop;
}


void AppOutputViewPart::startAppCommand(const QString &command)
{
    m_widget->startJob(QDir::homeDirPath(), command);
    topLevel()->raiseView(m_widget);
}


bool AppOutputViewPart::isRunning()
{
    return m_widget->isRunning();
}


void AppOutputViewPart::insertStdoutLine(const QString &line)
{
    m_widget->insertStdoutLine(line);
}


void AppOutputViewPart::insertStderrLine(const QString &line)
{
    m_widget->insertStderrLine(line);
}

#include "appoutputviewpart.moc"
