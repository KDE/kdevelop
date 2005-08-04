/***************************************************************************
 *   Copyright (C) 2004 by ian reinhart geiser                             *
 *   geiseri@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "distpart_part.h"

#include <q3whatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>

#include "kdevcore.h"
#include <kdevplugininfo.h>

#include "distpart_widget.h"

#include <kdialogbase.h>

typedef KDevGenericFactory<DistpartPart> DistpartFactory;
static const KDevPluginInfo data("kdevdistpart");
K_EXPORT_COMPONENT_FACTORY( libkdevdistpart, DistpartFactory( data ) )

DistpartPart::DistpartPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent, name ? name : "DistpartPart") {

    kdDebug(9007) << "DistpartPart::DistpartPart()" << endl;
    setInstance(DistpartFactory::instance());

    setXMLFile("kdevpart_distpart.rc");

    m_action =  new KAction( i18n("Distribution && Publishing"), "package", 0,
                             this, SLOT(show()),
                             actionCollection(), "make_dist" );

    m_action->setToolTip(i18n("Make source and binary distribution"));
    m_action->setWhatsThis(i18n("<b>Project distribution & publishing</b><p>Helps users package and publish their software."));
    //QWhatsThis::add(m_widget, i18n("This will help users package and publish their software."));

    m_dlg = new KDialogBase( 0 , "dist_part", false, i18n("Distribution & Publishing"), KDialogBase::Ok|KDialogBase::Cancel);

    m_dialog = new DistpartDialog(this, m_dlg );
    m_dlg->setMainWidget(m_dialog);

    connect( m_dlg, SIGNAL(okClicked()), m_dialog, SLOT(slotokayPushButtonPressed()));
    connect( m_dlg, SIGNAL(cancelClicked()), m_dialog, SLOT(slotcancelPushButtonPressed()));
    // Package types
    //RpmPackage = new SpecSupport(this);
    //LsmPackage = new LsmSupport(this);
}


DistpartPart::~DistpartPart() {
    kdDebug(9007) << "DistpartPart::~DistpartPart()" << endl;
    delete m_dialog;
    //delete RpmPackage;
    //delete LsmPackage;
}

void DistpartPart::show() {
    m_dlg->exec();
}

void DistpartPart::hide() {
    m_dlg->cancel();
}

#include "distpart_part.moc"
