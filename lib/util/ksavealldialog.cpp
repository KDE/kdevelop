/***************************************************************************
 *   Copyright (C) 2002 by Harald Fernengel and the KDevelop Team          *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qvbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <kpushbutton.h>
#include <klistbox.h>

#include "ksavealldialog.h"


KSaveAllDialog::KSaveAllDialog( const QStringList& filenames, QWidget* parent ) :
  KDialogBase( parent, "SaveAllDialog", true, i18n("Save modified files?"),
	       Ok | User1 | Close )
{
  m_result = Cancel;

  QVBox *top = makeVBoxMainWidget();

  (void)new QLabel( i18n("The following files have been modified. Save them?"), top );
  KListBox* lb = new KListBox( top );
  lb->setMinimumHeight( lb->fontMetrics().height() * 5 );
  lb->insertStringList( filenames );

  setButtonOKText( i18n("Save &All"), i18n("Saves all modified files") );
  setButtonText( User1, i18n("Save &None") );
  setButtonText( Close, i18n("&Cancel") );
  setButtonTip( User1, i18n("Lose all modifications") );
  setButtonTip( Close, i18n("Cancels the action") );

  connect( this, SIGNAL(closeClicked()), this, SLOT(cancel()) );
  connect( this, SIGNAL(okClicked()), this, SLOT(saveAll()) );
  connect( this, SIGNAL(user1Clicked()), this, SLOT(revert()) );
}

KSaveAllDialog::~KSaveAllDialog()
{
}

void KSaveAllDialog::revert()
{
  m_result = Revert;
  QDialog::accept();
}

void KSaveAllDialog::saveAll()
{
  m_result = SaveAll;
  QDialog::accept();
}

void KSaveAllDialog::cancel()
{
  m_result = Cancel;
  QDialog::reject();
}

#include "ksavealldialog.moc"
