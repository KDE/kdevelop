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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "distpart_part.h"

#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>

#include "kdevcore.h"

#include "distpart_widget.h"

#include <kdialogbase.h>

typedef KDevGenericFactory<DistpartPart> DistpartFactory;
static const KAboutData data("kdevdistpart", I18N_NOOP("Distribution & Publishing"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevdistpart, DistpartFactory( &data ) )

DistpartPart::DistpartPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin("Distribution", "dist", parent, name ? name : "DistpartPart") {

    kdDebug(9007) << "DistpartPart::DistpartPart()" << endl;
    setInstance(DistpartFactory::instance());

    setXMLFile("kdevpart_distpart.rc");

    m_action =  new KAction( i18n("Distribution & Publishing"), "package", 0,
                             this, SLOT(show()),
                             actionCollection(), "make_dist" );

    m_action->setStatusText(i18n("Make source and binary distribution"));
    m_action->setWhatsThis(i18n("<b>Project distribution & publishing</b><p>Helps users package and publish their software."));
    //QWhatsThis::add(m_widget, i18n("This will help users package and publish their software."));

    m_dlg = new KDialogBase( widget() , "dist_part", false, i18n("Distribution & Publishing"), KDialogBase::Ok|KDialogBase::Cancel);

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
