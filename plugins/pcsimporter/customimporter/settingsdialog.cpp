/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "settingsdialog.h"

#include <klistbox.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <kdeversion.h>
#include <klocale.h>

// only for KDE < 3.1
#if KDE_VERSION <= 305
#include "../compat/kdeveditlistbox.h"
using namespace KDevCompat;
#else
#include <keditlistbox.h>
#endif

#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <cstdlib>

SettingsDialog::SettingsDialog(QWidget* parent, const char* name, WFlags fl)
    : SettingsDialogBase(parent,name,fl)
{
    KURLRequester *req = new KURLRequester( this );
    req->setMode(KFile::Directory);
    KEditListBox::CustomEditor pCustomEditor;
#if KDE_VERSION > 305
    pCustomEditor = req->customEditor();
#else
    QObjectList* pOL = req->queryList("KLineEdit"); // dirty hack :)
    KLineEdit* pEdit = dynamic_cast<KLineEdit*>(pOL->first());
    assert(pEdit);
    KEditListBox::CustomEditor editor(req, pEdit);
    pCustomEditor = editor;
#endif
    elb = new KEditListBox( i18n("Directories to parse"), pCustomEditor, this );

    grid->addMultiCellWidget(elb, 2, 2, 0, grid->numCols());

    connect(dbName_edit, SIGNAL(textChanged(const QString& )), this, SLOT(nameChanged(const QString& )));
}

SettingsDialog::~SettingsDialog()
{
}

QString SettingsDialog::dbName( ) const
{
    return dbName_edit->text();
}

QStringList SettingsDialog::dirs( ) const
{
    return elb->items();
}

bool SettingsDialog::recursive( ) const
{
    return recursive_box->isChecked();
}

void SettingsDialog::nameChanged( const QString &text )
{
    emit enabled(!text.isEmpty());
}

#include "settingsdialog.moc"


