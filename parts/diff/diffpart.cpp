/***************************************************************************
 *   Copyright (C) 2001 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kfiledialog.h>

#include <kio/jobclasses.h>
#include <kio/job.h>

#include "kdevcore.h"

#include "diffpart.h"
#include "diffdlg.h"

typedef KGenericFactory<DiffPart> DiffFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdiff, DiffFactory( "kdevdiff" ) );

DiffPart::DiffPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(parent, name)
{
  setInstance(DiffFactory::instance());
  setXMLFile("kdevdiff.rc");

  (void) new KAction( i18n("Difference viewer..."), 0,
                        this, SLOT(slotExecDiff()),
                        actionCollection(), "tools_diff" );
}


DiffPart::~DiffPart()
{
}

void DiffPart::slotExecDiff()
{
  KURL url = KFileDialog::getOpenURL( QString::null, QString::null, 0, i18n("Please select a patch file") );

  if ( url.isEmpty() )
    return;

  DiffDlg* diffDlg = new DiffDlg( 0, "diffDlg" );

  diffDlg->openURL( url );
  diffDlg->exec();
  delete diffDlg;
}

#include "diffpart.moc"
