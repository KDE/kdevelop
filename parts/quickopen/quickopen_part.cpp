/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "quickopen_part.h"
#include "quickopendialog.h"
#include "quickopenclassdialog.h"

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include <kmainwindow.h>

#include <kdevmainwindow.h>
#include <kdevcore.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>

typedef KGenericFactory<QuickOpenPart> QuickOpenFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevquickopen, QuickOpenFactory( "kdevquickopen" ) )

QuickOpenPart::QuickOpenPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin("KDevPart", "kdevpart", parent, name ? name : "QuickOpenPart" )
{
    setInstance(QuickOpenFactory::instance());
    setXMLFile("kdevpart_quickopen.rc");

    m_actionQuickOpen = new KAction( i18n("Quick Open..."), CTRL + SHIFT + Key_O,
				       this, SLOT(slotQuickOpen()),
				       actionCollection(), "quick_open" );
    m_actionQuickOpen->setToolTip(i18n("Quick open file in project"));
    m_actionQuickOpen->setWhatsThis(i18n("<b>Quick open</b><p>Provides a file name input form with completion listbox to quickly open file in a project."));

    m_actionQuickOpenClass = new KAction( i18n("Find Class..."), 0,
				          this, SLOT(slotQuickOpenClass()),
				          actionCollection(), "quick_open_class" );
    m_actionQuickOpenClass->setToolTip(i18n("Find class in project"));
    m_actionQuickOpenClass->setWhatsThis(i18n("<b>Find class</b><p>Provides a class name input form with completion listbox to quickly open a file where the class is defined."));

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
}


QuickOpenPart::~QuickOpenPart()
{
}

void QuickOpenPart::slotProjectOpened( )
{
}

void QuickOpenPart::slotProjectClosed( )
{
}

void QuickOpenPart::slotQuickOpen( )
{
    QuickOpenDialog dlg( this, mainWindow()->main() );
    dlg.exec();
}

void QuickOpenPart::slotQuickOpenClass( )
{
    QuickOpenClassDialog dlg( this, mainWindow()->main() );
    dlg.exec();
}


#include "quickopen_part.moc"
