/***************************************************************************
                          caddexistingfiledlg.cpp  -
                             -------------------

    begin                : Tue Oct 20 1998
    copyright            : (C) 1998 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "caddexistingfiledlg.h"

#include "cproject.h"

#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qfiledialog.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qstringlist.h>
#include <kurlrequester.h>
#include <qgrid.h>
#include <qlayout.h>
#include <klineedit.h>
#include <kbuttonbox.h>

//#include <iostream>
//using namespace std;

CAddExistingFileDlg::CAddExistingFileDlg(QWidget *parent, const char *name,CProject* p_prj ) : QDialog(parent,name,true) {

  prj = p_prj;
  setCaption(i18n("Add Existing Files to Project"));

  QGridLayout *grid1 = new QGridLayout(this,2,4,15,7);
  source_label = new QLabel( this, "source_label" );
  source_label->setFocusPolicy( QWidget::NoFocus );
  source_label->setBackgroundMode( QWidget::PaletteBackground );
  source_label->setText(i18n("Sourcefile(s):") );
  source_label->setAlignment( 289 );
  source_label->setMargin( -1 );
  grid1->addWidget(source_label,0,0);

  source_edit = new KURLRequester( this, "source_edit" );
  source_edit->setFocusPolicy( QWidget::StrongFocus );
  //source_edit->setBackgroundMode( QWidget::PaletteBase );
  source_edit->lineEdit()->setText( "" );
  source_edit->fileDialog()->setMode(KFile::Files);

  grid1->addWidget(source_edit,0,1);

  destination_label = new QLabel( this, "destination_label" );
  destination_label->setFocusPolicy( QWidget::NoFocus );
  destination_label->setBackgroundMode( QWidget::PaletteBackground );
  destination_label->setText(i18n("Destinationdir:") );
  destination_label->setAlignment( 289 );
  destination_label->setMargin( -1 );
  grid1->addWidget(destination_label,1,0);

  destination_edit = new KURLRequester( this, "destination_edit" );
  destination_edit->setFocusPolicy( QWidget::StrongFocus );
  destination_edit->lineEdit()->setText( "" );
  destination_edit->fileDialog()->setMode(KFile::Directory);
  grid1->addWidget(destination_edit,1,1);

  QString sourceMsg = i18n("Select the source files to be added\nto the project here.");
  QWhatsThis::add(source_edit, sourceMsg);
  QWhatsThis::add(source_label, sourceMsg);

  QString destinationMsg = i18n("Select the directory where the new\n"
                                "source files will be copied to here.");
  QWhatsThis::add(destination_edit, destinationMsg);
  QWhatsThis::add(destination_label, destinationMsg);

  template_checkbox = new QCheckBox( this, "addTemplate_checkbox");
  template_checkbox->setFocusPolicy( QWidget::TabFocus );
  template_checkbox->setBackgroundMode( QWidget::PaletteBackground );
  template_checkbox->setText( i18n("Insert header/cpp-templates to added files" ));
  template_checkbox->setAutoRepeat( FALSE );
  template_checkbox->setAutoResize( FALSE );
  template_checkbox->setChecked( TRUE );
  QWhatsThis::add(template_checkbox, i18n("Check this if you want to insert your template to the added files."));
   grid1->addMultiCellWidget(template_checkbox,2,2,0,1);

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  ok_button  = bb->addButton( i18n("OK") );
  ok_button->setFocusPolicy( QWidget::TabFocus );
  ok_button->setBackgroundMode( QWidget::PaletteBackground );
  ok_button->setAutoRepeat( FALSE );
  ok_button->setAutoResize( FALSE );
  ok_button->setDefault( TRUE );
  
  cancel_button =  bb->addButton( i18n("Cancel") );
  cancel_button->setFocusPolicy( QWidget::TabFocus );
  cancel_button->setBackgroundMode( QWidget::PaletteBackground );
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  bb->layout();
  grid1->addWidget(bb,3,1);


  connect(ok_button,SIGNAL(clicked()),SLOT(OK()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
  connect(source_edit,SIGNAL(textChanged(const QString &)),SLOT(sourceTextChanged(const QString &)));
  connect(destination_edit,SIGNAL(textChanged(const QString &)),SLOT(destinationTextChanged(const QString &)));
}


CAddExistingFileDlg::~CAddExistingFileDlg(){
}

void CAddExistingFileDlg::sourceTextChanged(const QString &)
{
QStringList files(source_edit->fileDialog()->selectedFiles());
//  files.setAutoDelete(true);
  QString comp_str;
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
      comp_str = comp_str + (*it) + ",";
  }
  source_edit->lineEdit()->setText(comp_str);
  files.clear();
}


void CAddExistingFileDlg::destinationTextChanged(const QString &name)
{
 if(!name.isEmpty()){
    destination_edit->lineEdit()->setText(name);
  }
}

void CAddExistingFileDlg::OK(){
  QFileInfo file_info(source_edit->lineEdit()->text());
  QDir dir(destination_edit->lineEdit()->text());
  //   QString source_name = file_info.fileName();
  QString dest_name = destination_edit->lineEdit()->text();// + source_name

  // if (!file_info.exists()){
//     KMessageBox::message(this,i18n("Error..."),i18n("You must choose an existing sourcefile!")
// 		     ,KMessageBox::EXCLAMATION);
//     return;
//   }
  if(dest_name.contains(prj->getProjectDir()) == 0 ){
    KMessageBox::error(this,
		     i18n("You must choose a destination, that is in your project-dir!"));
    return;
  }
  if(!dir.exists()){
    KMessageBox::error(this,
		     i18n("You must choose a valid dir as a destination!"));
    return;
  }

  accept();
}

bool CAddExistingFileDlg::isTemplateChecked()
{
 return template_checkbox->isChecked();
}

#include "caddexistingfiledlg.moc"
