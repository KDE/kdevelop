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

#include "filecreate_newfile.moc"

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
#include <kdebug.h>
#include <kmessagebox.h>


namespace FileCreate {

  NewFileChooser::NewFileChooser(QWidget * parent) :
    KDialogBase(KDialogBase::Plain, i18n("New file dialog (title)", "New File"), KDialogBase::Ok|KDialogBase::Cancel,
                KDialogBase::Ok, parent, "New file", true)
  {
      QVBoxLayout* lay = new QVBoxLayout( plainPage(), 5, 5 );

      lay->addWidget( new QLabel( i18n("<b>New file creation</b>"), plainPage() ) );

      QGridLayout* grid = new QGridLayout(lay, 2, 2, 5 );
      QLabel * l = new QLabel(i18n("&Directory:"), plainPage() );
      grid->addWidget(l, 0, 0);
      m_urlreq = new KURLRequester( plainPage(), "url request" );
      grid->addWidget(m_urlreq, 0, 1);
      l->setBuddy(m_urlreq);
      l = new QLabel(i18n("&File name:"), plainPage() );
      grid->addWidget(l, 1, 0);
      m_filename = new QLineEdit( plainPage() );
      grid->addWidget(m_filename, 1, 1);
      l->setBuddy(m_filename);
//      lay->addWidget( grid );

      QHBoxLayout* hbox = new QHBoxLayout( lay, 5 );
      m_filetypes = new KComboBox( plainPage(), "combo" );
      hbox->addWidget(m_filetypes);
      m_addToProject = new QCheckBox( i18n("Add to project (on checkbox)", "&Add to project"), plainPage(), "addproject" );
      hbox->addWidget(m_addToProject);

      lay->addStretch(20);

      m_filename->setFocus();
      m_addToProject->setChecked( true );

      m_urlreq->setMode((int) KFile::Directory);
      connect( m_filename,  SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotFileNameChanged(const QString & ) ) );
      slotFileNameChanged( m_filename->text() );
  }

  NewFileChooser::~NewFileChooser() {
  }

    void NewFileChooser::slotFileNameChanged(const QString & _text)
    {
        enableButtonOK( !_text.isEmpty() );
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

    KURL result ( m_urlreq->url() );
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

  void NewFileChooser::setInProjectMode( bool m )
  {
    m_addToProject->setEnabled(m);
    m_addToProject->setChecked(m);
  }

	void NewFileChooser::accept()
	{
		QFileInfo file( url().path() );
		if ( file.exists() )
		{
			KMessageBox::sorry( 0, i18n("A file with this name already exists."), i18n("File Exists") );
			return;
		}
		KDialogBase::accept();
	}

}

