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

#include <qlayout.h>

#include <klocale.h>

#include "diffdlg.h"
#include "diffwidget.h"

DiffDlg::DiffDlg( QWidget *parent, const char *name ):
    KDialogBase( parent, name, true, i18n("Difference Viewer"), Ok )
{
  diffWidget = new DiffWidget( this, "Main Diff Widget" );
  setMainWidget( diffWidget );

  QVBoxLayout* layout = new QVBoxLayout( this, 0, spacingHint() );
  layout->addWidget( diffWidget );
}

DiffDlg::~DiffDlg()
{
}

void DiffDlg::slotClear()
{
  diffWidget->slotClear();
}

void DiffDlg::setDiff( const QString& diff )
{
  diffWidget->setDiff( diff );
}

void DiffDlg::openURL( const KURL& url )
{
  diffWidget->openURL( url );
}

#include "diffdlg.moc"
