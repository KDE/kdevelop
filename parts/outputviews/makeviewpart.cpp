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

#include <q3whatsthis.h>
#include <qdir.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include "kdevproject.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"

#include "makewidget.h"

static const KDevPluginInfo data("kdevmakeview");
typedef KDevGenericFactory< MakeViewPart > MakeViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevmakeview, MakeViewFactory( data ) )

MakeViewPart::MakeViewPart(QObject *parent, const char *name, const QStringList &)
    : KDevMakeFrontend(&data, parent)
{
    setObjectName(QString::fromUtf8(name));
    setInstance(MakeViewFactory::instance());

    setXMLFile("kdevmakeview.rc");

    m_widget = new MakeWidget(this);
    m_widget->setIcon( SmallIcon("exec") );
    m_widget->setCaption(i18n("Messages Output"));
    Q3WhatsThis::add(m_widget, i18n("<b>Messages output</b><p>"
                                   "The messages window shows the output of the compiler and "
                                   "used build tools like make, ant, uic, dcopidl etc. "
                                   "For compiler error messages, click on the error message. "
                                   "This will automatically open the source file and set the "
                                   "cursor to the line that caused the compiler error/warning."));

    mainWindow()->embedOutputView(m_widget, i18n("Messages"), i18n("Compiler output messages"));

    KAction *action;
    action = new KAction( i18n("&Next Error"), Qt::Key_F4, m_widget, SLOT(nextError()),
                          actionCollection(), "view_next_error");
    action->setToolTip( i18n("Go to the next error") );
    action->setWhatsThis(i18n("<b>Next error</b><p>Switches to the file and line where the next error was reported from."));
    action = new KAction( i18n("&Previous Error"), Qt::SHIFT+Qt::Key_F4, m_widget, SLOT(prevError()),
                          actionCollection(), "view_previous_error");
    action->setToolTip( i18n("Go to the previous error") );
    action->setWhatsThis(i18n("<b>Previous error</b><p>Switches to the file and line where the previous error was reported from."));

    connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
             this, SLOT(slotStopButtonClicked(KDevPlugin*)) );
}


MakeViewPart::~MakeViewPart()
{
	if ( m_widget )
	    mainWindow()->removeView( m_widget );
    delete m_widget;
}

void MakeViewPart::slotStopButtonClicked(KDevPlugin* which)
{
    if ( which != 0 && which != this )
        return;
    m_widget->killJob();
}

void MakeViewPart::queueCommand(const QString &dir, const QString &command)
{
    m_widget->queueJob(dir, command);
}


bool MakeViewPart::isRunning()
{
    return m_widget->isRunning();
}

QWidget* MakeViewPart::widget()
{
    return m_widget.operator->();
}

void MakeViewPart::updateSettingsFromConfig()
{
    m_widget->updateSettingsFromConfig();
}

#include "makeviewpart.moc"
