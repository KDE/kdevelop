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

#include <qvbox.h>
#include <qgrid.h>

#include <qhbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <kurlrequester.h>
#include <kcombobox.h>
#include <klocale.h>

namespace FileCreate {

  NewFileChooser::NewFileChooser(QWidget * parent) :
    KDialogBase(KDialogBase::Plain, i18n("New file dialog (title)", "New File"), KDialogBase::Ok|KDialogBase::Cancel,
                KDialogBase::Ok, parent, "New file", true)
  {
      QVBoxLayout* lay = new QVBoxLayout( plainPage(), 5, 5 );
      
      lay->addWidget( new QLabel( i18n("<b>New file creation</b>"), plainPage() ) );
      
      QGrid* grid = new QGrid( 2, plainPage() );
      grid->setSpacing( 5 );
      new QLabel(i18n("Directory:"), grid );
      m_urlreq = new KURLRequester( grid, "url request" );
      new QLabel(i18n("File name:"), grid );
      m_filename = new QLineEdit( grid );
      lay->addWidget( grid );
      
      QHBox* hbox = new QHBox( plainPage() );
      hbox->setMargin( 5 );
      hbox->setSpacing( 5 );
      m_filetypes = new KComboBox( hbox, "combo" );
      m_addToProject = new QCheckBox( i18n("Add to project (on checkbox)", "Add to project"), hbox, "addproject" );
      lay->addWidget( hbox );
      
      lay->addItem( new QSpacerItem(10, 20, QSizePolicy::Minimum, QSizePolicy::Expanding) );
      
      m_filename->setFocus();
      m_addToProject->setChecked( true );

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
                             (filetype->ext()!="" ? QString(" (." + filetype->ext() + ")") : QString("") ) );
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
