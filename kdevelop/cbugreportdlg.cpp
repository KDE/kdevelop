/***************************************************************************
                          cbugreportdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Thu May 6 1999                                           
    copyright            : (C) 1999 by Stefan Bartel                         
    email                : bartel@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "cbugreportdlg.h"

#include <qdir.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qfile.h>
#include <qstring.h>
#include <kprocess.h>
#include <kapp.h>
#include <qdatetime.h>
#include <iostream.h>
#include <kmsgbox.h>

CBugReportDlg::CBugReportDlg(QWidget *parent, const char *name,TBugReportInfo buginfo, QString bug_email) : QTabDialog(parent,name,true) {


  setCaption(i18n("Bug Report"));
  setFixedSize(415,460);

  //+++++++++++++ TAB: General inforamtion +++++++++++++++++++++++++++++++++++++++++

  QWidget* w=new QWidget(this,"General information");
  KQuickHelp::add(w, i18n("Fill in all information,\nwe need to help you."));
 
  QButtonGroup* qtarch_severity_group;
  qtarch_severity_group = new QButtonGroup( w, "severity_group" );
  qtarch_severity_group->setGeometry( 210, 260, 170, 120 );
  qtarch_severity_group->setMinimumSize( 0, 0 );
  qtarch_severity_group->setMaximumSize( 32767, 32767 );
  qtarch_severity_group->setFocusPolicy( QWidget::NoFocus );
  qtarch_severity_group->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_severity_group->setFontPropagation( QWidget::NoChildren );
  qtarch_severity_group->setPalettePropagation( QWidget::NoChildren );
  qtarch_severity_group->setFrameStyle( 49 );
  qtarch_severity_group->setTitle( i18n("Severity") );
  qtarch_severity_group->setAlignment( 1 );
  
  QButtonGroup* qtarch_priority_group;
  qtarch_priority_group = new QButtonGroup( w, "priority_group" );
  qtarch_priority_group->setGeometry( 20, 260, 170, 120 );
  qtarch_priority_group->setMinimumSize( 0, 0 );
  qtarch_priority_group->setMaximumSize( 32767, 32767 );
  qtarch_priority_group->setFocusPolicy( QWidget::NoFocus );
  qtarch_priority_group->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_priority_group->setFontPropagation( QWidget::NoChildren );
  qtarch_priority_group->setPalettePropagation( QWidget::NoChildren );
  qtarch_priority_group->setFrameStyle( 49 );
  qtarch_priority_group->setTitle( i18n("Priority") );
  qtarch_priority_group->setAlignment( 1 );
  
  name_edit = new QLineEdit( w, "name_edit" );
  name_edit->setGeometry( 170, 30, 210, 30 );
  name_edit->setMinimumSize( 0, 0 );
  name_edit->setMaximumSize( 32767, 32767 );
  name_edit->setFocusPolicy( QWidget::StrongFocus );
  name_edit->setBackgroundMode( QWidget::PaletteBase );
  name_edit->setFontPropagation( QWidget::NoChildren );
  name_edit->setPalettePropagation( QWidget::NoChildren );
  name_edit->setText(buginfo.author);
  name_edit->setMaxLength( 32767 );
  name_edit->setEchoMode( QLineEdit::Normal );
  name_edit->setFrame( TRUE );
  
  email_edit = new QLineEdit( w, "email_edit" );
  email_edit->setGeometry( 170, 70, 210, 30 );
  email_edit->setMinimumSize( 0, 0 );
  email_edit->setMaximumSize( 32767, 32767 );
  email_edit->setFocusPolicy( QWidget::StrongFocus );
  email_edit->setBackgroundMode( QWidget::PaletteBase );
  email_edit->setFontPropagation( QWidget::NoChildren );
  email_edit->setPalettePropagation( QWidget::NoChildren );
  email_edit->setText(buginfo.email);
  email_edit->setMaxLength( 32767 );
  email_edit->setEchoMode( QLineEdit::Normal );
  email_edit->setFrame( TRUE );
  
  QLabel* qtarch_name_label;
  qtarch_name_label = new QLabel( w, "name_label" );
  qtarch_name_label->setGeometry( 20, 30, 140, 30 );
  qtarch_name_label->setMinimumSize( 0, 0 );
  qtarch_name_label->setMaximumSize( 32767, 32767 );
  qtarch_name_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_name_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_name_label->setFontPropagation( QWidget::NoChildren );
  qtarch_name_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_name_label->setText( i18n("Your Name :") );
  qtarch_name_label->setAlignment( 289 );
  qtarch_name_label->setMargin( -1 );
  
  QLabel* qtarch_email_label;
  qtarch_email_label = new QLabel( w, "email_label" );
  qtarch_email_label->setGeometry( 20, 70, 140, 30 );
  qtarch_email_label->setMinimumSize( 0, 0 );
  qtarch_email_label->setMaximumSize( 32767, 32767 );
  qtarch_email_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_email_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_email_label->setFontPropagation( QWidget::NoChildren );
  qtarch_email_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_email_label->setText( i18n("Your E-mail Address :") );
  qtarch_email_label->setAlignment( 289 );
  qtarch_email_label->setMargin( -1 );
  
  QLabel* qtarch_subject_label;
  qtarch_subject_label = new QLabel( w, "subject_label" );
  qtarch_subject_label->setGeometry( 20, 110, 140, 30 );
  qtarch_subject_label->setMinimumSize( 0, 0 );
  qtarch_subject_label->setMaximumSize( 32767, 32767 );
  qtarch_subject_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_subject_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_subject_label->setFontPropagation( QWidget::NoChildren );
  qtarch_subject_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_subject_label->setText( i18n("Subject :") );
  qtarch_subject_label->setAlignment( 289 );
  qtarch_subject_label->setMargin( -1 );
  
  subject_edit = new QLineEdit( w, "subject_edit" );
  subject_edit->setGeometry( 170, 110, 210, 30 );
  subject_edit->setMinimumSize( 0, 0 );
  subject_edit->setMaximumSize( 32767, 32767 );
  subject_edit->setFocusPolicy( QWidget::StrongFocus );
  subject_edit->setBackgroundMode( QWidget::PaletteBase );
  subject_edit->setFontPropagation( QWidget::NoChildren );
  subject_edit->setPalettePropagation( QWidget::NoChildren );
  subject_edit->setMaxLength( 32767 );
  subject_edit->setEchoMode( QLineEdit::Normal );
  subject_edit->setFrame( TRUE );
  
  QLabel* qtarch_class_label;
  qtarch_class_label = new QLabel( w, "class_label" );
  qtarch_class_label->setGeometry( 20, 160, 110, 30 );
  qtarch_class_label->setMinimumSize( 0, 0 );
  qtarch_class_label->setMaximumSize( 32767, 32767 );
  qtarch_class_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_class_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_class_label->setFontPropagation( QWidget::NoChildren );
  qtarch_class_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_class_label->setText( i18n("Error Class :") );
  qtarch_class_label->setAlignment( 289 );
  qtarch_class_label->setMargin( -1 );
  
  class_combo = new QComboBox( FALSE, w, "class_combo" );
  class_combo->setGeometry( 170, 160, 210, 30 );
  class_combo->setMinimumSize( 0, 0 );
  class_combo->setMaximumSize( 32767, 32767 );
  class_combo->setFocusPolicy( QWidget::StrongFocus );
  class_combo->setBackgroundMode( QWidget::PaletteBackground );
  class_combo->setFontPropagation( QWidget::AllChildren );
  class_combo->setPalettePropagation( QWidget::AllChildren );
  class_combo->setSizeLimit( 10 );
  class_combo->setAutoResize( FALSE );
  class_combo->insertItem( i18n("software bug") );
  class_combo->insertItem( i18n("documentation bug") );
  class_combo->insertItem( i18n("change-request") );
  class_combo->insertItem( i18n("how do I ...") );
  
  priority_low = new QRadioButton( w, "priority_low" );
  priority_low->setGeometry( 30, 280, 150, 30 );
  priority_low->setMinimumSize( 0, 0 );
  priority_low->setMaximumSize( 32767, 32767 );
  priority_low->setFocusPolicy( QWidget::TabFocus );
  priority_low->setBackgroundMode( QWidget::PaletteBackground );
  priority_low->setFontPropagation( QWidget::NoChildren );
  priority_low->setPalettePropagation( QWidget::NoChildren );
  priority_low->setText( i18n("low") );
  priority_low->setAutoRepeat( FALSE );
  priority_low->setAutoResize( FALSE );
  priority_low->setChecked( TRUE );
  
  priority_medium = new QRadioButton( w, "priority_medium" );
  priority_medium->setGeometry( 30, 310, 150, 30 );
  priority_medium->setMinimumSize( 0, 0 );
  priority_medium->setMaximumSize( 32767, 32767 );
  priority_medium->setFocusPolicy( QWidget::TabFocus );
  priority_medium->setBackgroundMode( QWidget::PaletteBackground );
  priority_medium->setFontPropagation( QWidget::NoChildren );
  priority_medium->setPalettePropagation( QWidget::NoChildren );
  priority_medium->setText( i18n("medium") );
  priority_medium->setAutoRepeat( FALSE );
  priority_medium->setAutoResize( FALSE );
  
  priority_high = new QRadioButton( w, "priority_high" );
  priority_high->setGeometry( 30, 340, 150, 30 );
  priority_high->setMinimumSize( 0, 0 );
  priority_high->setMaximumSize( 32767, 32767 );
  priority_high->setFocusPolicy( QWidget::TabFocus );
  priority_high->setBackgroundMode( QWidget::PaletteBackground );
  priority_high->setFontPropagation( QWidget::NoChildren );
  priority_high->setPalettePropagation( QWidget::NoChildren );
  priority_high->setText( i18n("high") );
  priority_high->setAutoRepeat( FALSE );
  priority_high->setAutoResize( FALSE );
  
  severity_harmless = new QRadioButton( w, "severity_harmless" );
  severity_harmless->setGeometry( 220, 280, 150, 30 );
  severity_harmless->setMinimumSize( 0, 0 );
  severity_harmless->setMaximumSize( 32767, 32767 );
  severity_harmless->setFocusPolicy( QWidget::TabFocus );
  severity_harmless->setBackgroundMode( QWidget::PaletteBackground );
  severity_harmless->setFontPropagation( QWidget::NoChildren );
  severity_harmless->setPalettePropagation( QWidget::NoChildren );
  severity_harmless->setText( i18n("normal") );
  severity_harmless->setAutoRepeat( FALSE );
  severity_harmless->setAutoResize( FALSE );
  severity_harmless->setChecked( TRUE );
  
  severity_serious = new QRadioButton( w, "severity_serious" );
  severity_serious->setGeometry( 220, 310, 150, 30 );
  severity_serious->setMinimumSize( 0, 0 );
  severity_serious->setMaximumSize( 32767, 32767 );
  severity_serious->setFocusPolicy( QWidget::TabFocus );
  severity_serious->setBackgroundMode( QWidget::PaletteBackground );
  severity_serious->setFontPropagation( QWidget::NoChildren );
  severity_serious->setPalettePropagation( QWidget::NoChildren );
  severity_serious->setText( i18n("grave") );
  severity_serious->setAutoRepeat( FALSE );
  severity_serious->setAutoResize( FALSE );
  
  severity_critical = new QRadioButton( w, "severity_critical" );
  severity_critical->setGeometry( 220, 340, 150, 30 );
  severity_critical->setMinimumSize( 0, 0 );
  severity_critical->setMaximumSize( 32767, 32767 );
  severity_critical->setFocusPolicy( QWidget::TabFocus );
  severity_critical->setBackgroundMode( QWidget::PaletteBackground );
  severity_critical->setFontPropagation( QWidget::NoChildren );
  severity_critical->setPalettePropagation( QWidget::NoChildren );
  severity_critical->setText( i18n("critical") );
  severity_critical->setAutoRepeat( FALSE );
  severity_critical->setAutoResize( FALSE );
  
  location_combo = new QComboBox( TRUE, w, "location_combo" );
  location_combo->setGeometry( 170, 210, 210, 30 );
  location_combo->setMinimumSize( 0, 0 );
  location_combo->setMaximumSize( 32767, 32767 );
  location_combo->setFocusPolicy( QWidget::StrongFocus );
  location_combo->setBackgroundMode( QWidget::PaletteBackground );
  location_combo->setFontPropagation( QWidget::AllChildren );
  location_combo->setPalettePropagation( QWidget::AllChildren );
  location_combo->setSizeLimit( 10 );
  location_combo->setAutoResize( FALSE );
  location_combo->insertItem( i18n("I don´t know") );
  location_combo->insertItem( i18n("class tree") );
  location_combo->insertItem( i18n("logical file tree") );
  location_combo->insertItem( i18n("documentation tree") );
  location_combo->insertItem( i18n("editor") );
  location_combo->insertItem( i18n("dialog editor") );
  location_combo->insertItem( i18n("documentation browser") );
  location_combo->insertItem( i18n("app-wizard") );
  location_combo->insertItem( i18n("build process") );
  location_combo->insertItem( i18n("output window") );
  location_combo->insertItem( i18n("configuration") );
  location_combo->insertItem( i18n("project management") );
  location_combo->insertItem( i18n("installation process") );
  
  QLabel* qtarch_location_label;
  qtarch_location_label = new QLabel( w, "location_label" );
  qtarch_location_label->setGeometry( 20, 210, 110, 30 );
  qtarch_location_label->setMinimumSize( 0, 0 );
  qtarch_location_label->setMaximumSize( 32767, 32767 );
  qtarch_location_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_location_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_location_label->setFontPropagation( QWidget::NoChildren );
  qtarch_location_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_location_label->setText( i18n("Error Location :") );
  qtarch_location_label->setAlignment( 289 );
  qtarch_location_label->setMargin( -1 );
  
  qtarch_severity_group->insert( severity_harmless );
  qtarch_severity_group->insert( severity_serious );
  qtarch_severity_group->insert( severity_critical );
  
  qtarch_priority_group->insert( priority_low );
  qtarch_priority_group->insert( priority_medium );
  qtarch_priority_group->insert( priority_high );
  
  addTab(w,i18n("General information"));
  
  //+++++++++ TAB: System information ++++++++++++++++++++++++++++++++
  
  QWidget* w2=new QWidget(this,"System information");
  
  QButtonGroup* qtarch_environment_group;
  qtarch_environment_group = new QButtonGroup( w2, "environment_group" );
  qtarch_environment_group->setGeometry( 10, 110, 380, 210 );
	qtarch_environment_group->setMinimumSize( 0, 0 );
	qtarch_environment_group->setMaximumSize( 32767, 32767 );
	qtarch_environment_group->setFocusPolicy( QWidget::NoFocus );
	qtarch_environment_group->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_environment_group->setFontPropagation( QWidget::NoChildren );
	qtarch_environment_group->setPalettePropagation( QWidget::NoChildren );
	qtarch_environment_group->setFrameStyle( 49 );
	qtarch_environment_group->setTitle( i18n("Environment") );
	qtarch_environment_group->setAlignment( 1 );

	QLabel* qtarch_kdevelop_version_label;
	qtarch_kdevelop_version_label = new QLabel( w2, "kdevelop_version_label" );
	qtarch_kdevelop_version_label->setGeometry( 20, 40, 150, 30 );
	qtarch_kdevelop_version_label->setMinimumSize( 0, 0 );
	qtarch_kdevelop_version_label->setMaximumSize( 32767, 32767 );
	qtarch_kdevelop_version_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_kdevelop_version_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_kdevelop_version_label->setFontPropagation( QWidget::NoChildren );
	qtarch_kdevelop_version_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_kdevelop_version_label->setText( i18n("KDevelop version :") );
	qtarch_kdevelop_version_label->setAlignment( 289 );
	qtarch_kdevelop_version_label->setMargin( -1 );

	kdevelop_version_edit = new QLineEdit( w2, "kdevelop_version_edit" );
	kdevelop_version_edit->setGeometry( 170, 40, 200, 30 );
	kdevelop_version_edit->setMinimumSize( 0, 0 );
	kdevelop_version_edit->setMaximumSize( 32767, 32767 );
	kdevelop_version_edit->setFocusPolicy( QWidget::StrongFocus );
	kdevelop_version_edit->setBackgroundMode( QWidget::PaletteBase );
	kdevelop_version_edit->setFontPropagation( QWidget::NoChildren );
	kdevelop_version_edit->setPalettePropagation( QWidget::NoChildren );
	kdevelop_version_edit->setText(VERSION); // takes the version number from the config.h macro
	kdevelop_version_edit->setMaxLength( 32767 );
	kdevelop_version_edit->setEchoMode( QLineEdit::Normal );
	kdevelop_version_edit->setFrame( TRUE );

	kde_version_edit = new QLineEdit( w2, "kde_version_edit" );
	kde_version_edit->setGeometry( 170, 150, 200, 30 );
	kde_version_edit->setMinimumSize( 0, 0 );
	kde_version_edit->setMaximumSize( 32767, 32767 );
	kde_version_edit->setFocusPolicy( QWidget::StrongFocus );
	kde_version_edit->setBackgroundMode( QWidget::PaletteBase );
	kde_version_edit->setFontPropagation( QWidget::NoChildren );
	kde_version_edit->setPalettePropagation( QWidget::NoChildren );
	kde_version_edit->setText( buginfo.kde_version);
	kde_version_edit->setMaxLength( 32767 );
	kde_version_edit->setEchoMode( QLineEdit::Normal );
	kde_version_edit->setFrame( TRUE );

	QLabel* qtarch_kde_version_label;
	qtarch_kde_version_label = new QLabel( w2, "kde_version_label" );
	qtarch_kde_version_label->setGeometry( 30, 150, 140, 30 );
	qtarch_kde_version_label->setMinimumSize( 0, 0 );
	qtarch_kde_version_label->setMaximumSize( 32767, 32767 );
	qtarch_kde_version_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_kde_version_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_kde_version_label->setFontPropagation( QWidget::NoChildren );
	qtarch_kde_version_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_kde_version_label->setText( i18n("KDE version :") );
	qtarch_kde_version_label->setAlignment( 289 );
	qtarch_kde_version_label->setMargin( -1 );

	qt_version_edit = new QLineEdit( w2, "qt_version_edit" );
	qt_version_edit->setGeometry( 170, 190, 200, 30 );
	qt_version_edit->setMinimumSize( 0, 0 );
	qt_version_edit->setMaximumSize( 32767, 32767 );
	qt_version_edit->setFocusPolicy( QWidget::StrongFocus );
	qt_version_edit->setBackgroundMode( QWidget::PaletteBase );
	qt_version_edit->setFontPropagation( QWidget::NoChildren );
	qt_version_edit->setPalettePropagation( QWidget::NoChildren );
	qt_version_edit->setText(buginfo.qt_version );
	qt_version_edit->setMaxLength( 32767 );
	qt_version_edit->setEchoMode( QLineEdit::Normal );
	qt_version_edit->setFrame( TRUE );

	QLabel* qtarch_qt_version_label;
	qtarch_qt_version_label = new QLabel( w2, "qt_version_label" );
	qtarch_qt_version_label->setGeometry( 30, 190, 140, 30 );
	qtarch_qt_version_label->setMinimumSize( 0, 0 );
	qtarch_qt_version_label->setMaximumSize( 32767, 32767 );
	qtarch_qt_version_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_qt_version_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_qt_version_label->setFontPropagation( QWidget::NoChildren );
	qtarch_qt_version_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_qt_version_label->setText( i18n("QT version :") );
	qtarch_qt_version_label->setAlignment( 289 );
	qtarch_qt_version_label->setMargin( -1 );

	os_edit = new QLineEdit( w2, "os_edit" );
	os_edit->setGeometry( 170, 230, 200, 30 );
	os_edit->setMinimumSize( 0, 0 );
	os_edit->setMaximumSize( 32767, 32767 );
	os_edit->setFocusPolicy( QWidget::StrongFocus );
	os_edit->setBackgroundMode( QWidget::PaletteBase );
	os_edit->setFontPropagation( QWidget::NoChildren );
	os_edit->setPalettePropagation( QWidget::NoChildren );
	os_edit->setText( buginfo.os );
	os_edit->setMaxLength( 32767 );
	os_edit->setEchoMode( QLineEdit::Normal );
	os_edit->setFrame( TRUE );

	QLabel* qtarch_os_label;
	qtarch_os_label = new QLabel( w2, "os_label" );
	qtarch_os_label->setGeometry( 30, 230, 140, 30 );
	qtarch_os_label->setMinimumSize( 0, 0 );
	qtarch_os_label->setMaximumSize( 32767, 32767 );
	qtarch_os_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_os_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_os_label->setFontPropagation( QWidget::NoChildren );
	qtarch_os_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_os_label->setText( i18n("OS / distribution :") );
	qtarch_os_label->setAlignment( 289 );
	qtarch_os_label->setMargin( -1 );

	QLabel* qtarch_compiler_label;
	qtarch_compiler_label = new QLabel( w2, "compiler_label" );
	qtarch_compiler_label->setGeometry( 30, 270, 140, 30 );
	qtarch_compiler_label->setMinimumSize( 0, 0 );
	qtarch_compiler_label->setMaximumSize( 32767, 32767 );
	qtarch_compiler_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_compiler_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_compiler_label->setFontPropagation( QWidget::NoChildren );
	qtarch_compiler_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_compiler_label->setText( i18n("Compiler") );
	qtarch_compiler_label->setAlignment( 289 );
	qtarch_compiler_label->setMargin( -1 );

	compiler_edit = new QLineEdit( w2, "compiler_edit" );
	compiler_edit->setGeometry( 170, 270, 200, 30 );
	compiler_edit->setMinimumSize( 0, 0 );
	compiler_edit->setMaximumSize( 32767, 32767 );
	compiler_edit->setFocusPolicy( QWidget::StrongFocus );
	compiler_edit->setBackgroundMode( QWidget::PaletteBase );
	compiler_edit->setFontPropagation( QWidget::NoChildren );
	compiler_edit->setPalettePropagation( QWidget::NoChildren );
	compiler_edit->setText( buginfo.compiler );
	compiler_edit->setMaxLength( 32767 );
	compiler_edit->setEchoMode( QLineEdit::Normal );
	compiler_edit->setFrame( TRUE );

	addTab(w2,i18n("System information"));


  //+++++++++ TAB: Problem description+++++++++++++++++++++++++++++++++++++
  QWidget* w3=new QWidget(this,"Problem description");
  KQuickHelp::add(w3, i18n("Insert as much information about your\nproblem, so that we are able to help by\nyour description."));

	description_mledit = new QMultiLineEdit( w3, "description_mledit" );
	description_mledit->setGeometry( 20, 40, 360, 130 );
	description_mledit->setMinimumSize( 0, 0 );
	description_mledit->setMaximumSize( 32767, 32767 );
	description_mledit->setFocusPolicy( QWidget::StrongFocus );
	description_mledit->setBackgroundMode( QWidget::PaletteBase );
	description_mledit->setFontPropagation( QWidget::SameFont );
	description_mledit->setPalettePropagation( QWidget::SameFont );
	description_mledit->insertLine( "" );
	description_mledit->setReadOnly( FALSE );
	description_mledit->setOverwriteMode( FALSE );

	QLabel* qtarch_description_label;
	qtarch_description_label = new QLabel( w3, "description_label" );
	qtarch_description_label->setGeometry( 20, 10, 360, 30 );
	qtarch_description_label->setMinimumSize( 0, 0 );
	qtarch_description_label->setMaximumSize( 32767, 32767 );
	qtarch_description_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_description_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_description_label->setFontPropagation( QWidget::NoChildren );
	qtarch_description_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_description_label->setText( i18n("Bug Description :") );
	qtarch_description_label->setAlignment( 289 );
	qtarch_description_label->setMargin( -1 );

	QLabel* qtarch_repeat_label;
	qtarch_repeat_label = new QLabel( w3, "repeat_label" );
	qtarch_repeat_label->setGeometry( 20, 160, 360, 30 );
	qtarch_repeat_label->setMinimumSize( 0, 0 );
	qtarch_repeat_label->setMaximumSize( 32767, 32767 );
	qtarch_repeat_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_repeat_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_repeat_label->setFontPropagation( QWidget::NoChildren );
	qtarch_repeat_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_repeat_label->setText( i18n("How to repeat the error :") );
	qtarch_repeat_label->setAlignment( 289 );
	qtarch_repeat_label->setMargin( -1 );

	repeat_mledit = new QMultiLineEdit( w3, "repeat_mledit" );
	repeat_mledit->setGeometry( 20, 190, 360, 70 );
	repeat_mledit->setMinimumSize( 0, 0 );
	repeat_mledit->setMaximumSize( 32767, 32767 );
	repeat_mledit->setFocusPolicy( QWidget::StrongFocus );
	repeat_mledit->setBackgroundMode( QWidget::PaletteBase );
	repeat_mledit->setFontPropagation( QWidget::SameFont );
	repeat_mledit->setPalettePropagation( QWidget::SameFont );
	repeat_mledit->insertLine( "" );
	repeat_mledit->setReadOnly( FALSE );
	repeat_mledit->setOverwriteMode( FALSE );

	QLabel* qtarch_fix_label;
	qtarch_fix_label = new QLabel( w3, "fix_label" );
	qtarch_fix_label->setGeometry( 20, 260, 360, 30 );
	qtarch_fix_label->setMinimumSize( 0, 0 );
	qtarch_fix_label->setMaximumSize( 32767, 32767 );
	qtarch_fix_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_fix_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_fix_label->setFontPropagation( QWidget::NoChildren );
	qtarch_fix_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_fix_label->setText( i18n("Perhaps a bugfix or workaround ?") );
	qtarch_fix_label->setAlignment( 289 );
	qtarch_fix_label->setMargin( -1 );

	fix_mledit = new QMultiLineEdit( w3, "fix_mledit" );
	fix_mledit->setGeometry( 20, 290, 360, 80 );
	fix_mledit->setMinimumSize( 0, 0 );
	fix_mledit->setMaximumSize( 32767, 32767 );
	fix_mledit->setFocusPolicy( QWidget::StrongFocus );
	fix_mledit->setBackgroundMode( QWidget::PaletteBase );
	fix_mledit->setFontPropagation( QWidget::SameFont );
	fix_mledit->setPalettePropagation( QWidget::SameFont );
	fix_mledit->insertLine( "" );
	fix_mledit->setReadOnly( FALSE );
	fix_mledit->setOverwriteMode( FALSE );

	addTab(w3,i18n("Problem description"));
	
	// **************set the button*********************
	setOkButton(0);
  setDefaultButton(i18n("Send E-Mail"));
  setCancelButton(i18n("Cancel"));
  connect( this, SIGNAL(defaultButtonPressed()), SLOT(ok()) );

  // generate bug-id
  QString s;
  QDate date=QDate::currentDate();
  QTime time=QTime::currentTime();
  s.setNum(time.hour());
  if (s.length()==1) {
    s.insert(0,"0");
  }
  strBugID=s+":";
  s.setNum(time.minute());
  if (s.length()==1) {
    s.insert(0,"0");
  }
  strBugID+=s+",";
  s.setNum(date.day());
  if (s.length()==1) {
    s.insert(0,"0");
  }
  strBugID+=s+".";
  s.setNum(date.month());
  if (s.length()==1) {
    s.insert(0,"0");
  }
  strBugID+=s+".";
  s.setNum(date.year());
  s.remove(0,2);
  if (s.length()==1) {
    s.insert(0,"0");
  }
  strBugID+=s;
 
  // save bugreport-email
  BugEmail=bug_email;

  // cerr << endl << "init dialog";
//   cerr << endl << author;
//   cerr << endl << author_email;
//   cerr << endl << bug_email;
//   cerr << endl << strBugID;

}

CBugReportDlg::~CBugReportDlg(){
}

void CBugReportDlg::ok() {

  if (description_mledit->text() == ""  ||  subject_edit->text() == "") {
    KMsgBox::message(this,i18n("Information"),i18n("Please fill in at least the subject and bug description!"));
    return;
  }
  //  cerr << endl << "slot ok()"
  email_address = email_edit->text();
  name = name_edit->text();
  os = os_edit->text();
  compiler = compiler_edit->text();
  kde_version = kde_version_edit->text();
  qt_version = qt_version_edit->text();
  
  if (generateEmail()) {
    if(sendEmail()){
      KMsgBox::message(this,i18n("Bug Report"),i18n("Bugreport was successfully submitted to the KDevelop Team.\n\t\tThank you!"));
    }
  }
  accept();
}

bool CBugReportDlg::generateEmail() {

  //  cout << endl << "start generateEmail";
  QString text ="\n";
  text.append("Package: kdevelop\n");
  text.append("Version: ");text.append(kdevelop_version_edit->text());text.append("\n");
  text.append("Severity: ");
  if (severity_harmless->isChecked()) text.append("normal\n\n");
  if (severity_serious->isChecked()) text.append("grave\n\n");
  if (severity_critical->isChecked()) text.append("critical\n\n");
  
  text.append("Bugreport ID : "+strBugID+"\n\n");
  text.append("Originator\t: ");text.append(name_edit->text());text.append("\n");
  text.append("E-Mail\t: ");text.append(email_edit->text());text.append("\n\n");
  text.append("Subject : ");text.append(subject_edit->text());text.append("\n\n");
  text.append("Error Class\t: ");text.append(class_combo->currentText());text.append("\n");
  text.append("Error Location: ");text.append(location_combo->currentText());text.append("\n");
  text.append("Priority\t: ");
  if (priority_low->isChecked()) text.append("low\n");
  if (priority_medium->isChecked()) text.append("medium\n");
  if (priority_high->isChecked()) text.append("high\n");
  
  text.append("Bug Description ---------------------------\n\n");
  text.append(description_mledit->text());text.append("\n\n");
  text.append("How to repeat the error -------------------\n\n");
  text.append(repeat_mledit->text());text.append("\n\n");
  text.append("Bugfix or Workaround ----------------------\n\n");
  text.append(fix_mledit->text());text.append("\n\n");
  text.append("System Information ------------------------\n\n");
  text.append("KDevelop version\t: ");text.append(kdevelop_version_edit->text());text.append("\n");
  text.append("KDE version\t\t: ");text.append(kde_version_edit->text());text.append("\n");
  text.append("QT version\t\t: ");text.append(qt_version_edit->text());text.append("\n");
  text.append("OS/Distribution\t: ");text.append(os_edit->text());text.append("\n");
  text.append("Compiler\t\t: ");text.append(compiler_edit->text());text.append("\n\n");

  QDir dir(KApplication::localkdedir()+"/share/apps/");
  dir.mkdir("kdevelop");
  //  cerr << endl << " dir: " << dir.absPath();
  QFile file(KApplication::localkdedir()+"/share/apps/kdevelop/bugreport."+strBugID);
  //  cerr << endl << "file: " << KApplication::localkdedir()+"/share/apps/kdevelop/bugreport."+strBugID;
  if (!file.open(IO_WriteOnly)) {
    return false;
  }
  file.writeBlock(text,text.length());
  //  cerr << endl << "file written";
  file.close();

  return true;
}


bool CBugReportDlg::sendEmail() {

  //  cerr << endl << "start sendEmail";
  QString command("cat ");
  command.append(KApplication::localkdedir()+"/share/apps/kdevelop/bugreport."+strBugID);
  command.append(" | mail -s \x22");
  command.append(subject_edit->text());
  command.append(" ["+strBugID+"]\x22 ");
  command.append(BugEmail);
  //  cerr << endl << command;
  KShellProcess *process2=new KShellProcess;
  *process2 << command;
  process2->start(KProcess::Block,KProcess::NoCommunication);

  return true;
}

