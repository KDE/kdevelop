/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "filecreate_newfile.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <kurlrequester.h>
#include <kcombobox.h>
#include <klocale.h>

namespace FileCreate {

  NewFileChooser::NewFileChooser(QWidget * parent) :
    KDialogBase(KDialogBase::Plain, i18n("New file dialog (title)", "New file"), KDialogBase::Ok|KDialogBase::Cancel,
                KDialogBase::Ok, parent, "New file", true)
  {
    m_grid = new QGridLayout(plainPage(), 3, 3, 10, 10);
    m_grid->addMultiCellWidget( new QLabel(i18n("New file creation"), plainPage() ), 0, 0, 0, 2 );
    m_grid->addWidget( new QLabel(i18n("Directory"), plainPage() ), 1, 0 );
    m_grid->addMultiCellWidget( m_urlreq = new KURLRequester(plainPage(), "url request"), 1, 1, 1, 2);
    m_grid->addWidget( new QLabel(i18n("Filename"), plainPage() ) , 2, 0 );
    m_grid->addMultiCellWidget( m_filename = new QLineEdit( plainPage() ), 2, 2, 1, 2);
    m_grid->addMultiCellWidget( m_filetypes = new KComboBox( plainPage(), "combo" ), 3, 3, 0, 1 );
    m_grid->addWidget( m_addToProject = new QCheckBox(i18n("Add to project (on checkbox)", "Add to project"), plainPage(), "addproject"), 3, 2);
    m_grid->setColStretch( 0, 1 );
    m_grid->setColStretch( 1, 1 );
    m_grid->setColStretch( 2, 1 );

    m_filename->setFocus();
    m_addToProject->setChecked(true);

    m_urlreq->setMode((int) KFile::Directory);
  }

  NewFileChooser::~NewFileChooser() {
  }

  void NewFileChooser::setFileTypes(QPtrList<FileType> filetypes) {
    for(FileType * filetype = filetypes.first();
	filetype;
	filetype=filetypes.next()) {

      if (filetype->enabled()) {

	if (filetype->subtypes().count()==0)
          addType(filetype);

	QPtrList<FileType> subtypes = filetype->subtypes();
	for(FileType * subtype = subtypes.first();
	    subtype;
	    subtype=subtypes.next()) {
	  if (subtype->enabled())
            addType(subtype);

	}

      }

    }

  }

  KURL NewFileChooser::url() const {

    KURL result = m_urlreq->url();
    result.cd( m_filename->text() );
    return result;
  }

  bool NewFileChooser::addToProject() const {
    return m_addToProject->isChecked();
  }

  const FileType *NewFileChooser::selectedType() const {
    if (!m_filetypes->count()) return NULL;
    return m_typeInCombo[m_filetypes->currentItem()];
  }

  void NewFileChooser::addType(const FileType * filetype) {
    m_typeInCombo[m_filetypes->count()]=filetype;
    m_filetypes->insertItem( filetype->name() +
                             (filetype->ext()!="" ? " (." + filetype->ext() + ")" : "" ) );
  }

  void NewFileChooser::setCurrent(const FileType *filetype) {
    int changeToRow = -1;
    QMap<int,const FileType*>::Iterator it;
    for ( it = m_typeInCombo.begin(); it != m_typeInCombo.end() && changeToRow==-1; ++it ) {
      if (it.data()==filetype)
	changeToRow=it.key();
    }
    if (changeToRow>-1) m_filetypes->setCurrentItem(changeToRow);
  }

  void NewFileChooser::setDirectory(const QString & url) {
    m_urlreq->setURL(url);
  }

  void NewFileChooser::setName(const QString & name) {
    m_filename->setText(name);
  }


}
