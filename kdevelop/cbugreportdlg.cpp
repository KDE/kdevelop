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

#include <kapp.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include <qdir.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qfile.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qmultilinedit.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombo.h>
#include <qwhatsthis.h>
#include <qstring.h>
#include <qlayout.h>
#include <qgrid.h>

#include <iostream.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

CBugReportDlg::CBugReportDlg(QWidget *parent,
                                const char *name,TBugReportInfo buginfo,
                                const QString& bug_email) :
  QTabDialog(parent,name,true)
{


  setCaption(i18n("Bug Report"));
  setFixedSize(466,460);

  //+++++++++++++ TAB: Attention +++++++++++++++++++++++++++++++++++++++++

  QWidget* w0=new QWidget(this,"Attention");
   QGridLayout *grid1 = new QGridLayout(w0,8,2,15,7);
  QMultiLineEdit* info_mledit = new QMultiLineEdit( w0, "info_mledit" );
                grid1->addMultiCellWidget(info_mledit,0,6,0,1);
	info_mledit->setFocusPolicy( QWidget::StrongFocus );
	info_mledit->setBackgroundMode( QWidget::PaletteBase );
	info_mledit->setFontPropagation( QWidget::SameFont );
	info_mledit->setPalettePropagation( QWidget::SameFont );
	info_mledit->setReadOnly( TRUE );
	info_mledit->setOverwriteMode( FALSE );
	info_mledit->setText( i18n( "IMPORTANT !\n\n"\
	"Please read these instructions before you send in your bugreport.\n\n"\
	"1. Fill in the command to run sendmail into the textfield below.\n"\
	"  It will be saved in your local kdeveloprc,so you don't have to do\n"
	"  this all the time.\n"\
	"  ( We hope, you don't have to write bugreports all the time.:) )\n"\
	"  Make sure, that you use absolute pathnames.\n"\
	"  It's possible, that sendmail isn't located in your search path.\n"\
	"         Example: /usr/sbin/sendmail\n"\
	"  If you can't use sendmail, just leave the field empty.\n"\
	"  KDevelop will write the bugreport into a file named 'bugreport.XXX'\n"\
	"  located at HOME/.kde/share/apps/kdevelop.\n"\
	"  Load it into your mail program, cut out the first 3 lines\n"
	"  ( the mail header for sendmail ) and send it.\n\n"\
	"2. Fill in your name and e-mail address, so we know who you are\n"\
	"  and can contact you for further questions.\n\n"\
	"3. Fill in the subject and the basic informations.\n\n"\
	"4. Describe your system. Don' forget to fill in information\n"\
	"  about programs related to your problem. ( your automake version\n"\
	"  for build problems etc. )\n\n"\
	"5. Describe the bug you encountered and how we can reproduce it.\n"\
	"  Describe it exactly. Something like 'KDevelop crashes' is useless.\n\n"\
	"6. Please write in ENGLISH if possible. If not, use a language,\n"\
	"  that at least one developer understand.\n\n"\
	"7. Push 'Send E-Mail'. :)\n"\
	"  Currently, the mail will go to the KDE Bug tracking system.\n"\
	"  It offers a web interface and will inform you,\n"\
	"  when your bug is cleaned.\n"\
	"  Visit <http://bugs.kde.org>." ) );

	QLabel* qtarch_sendmail_label;
	qtarch_sendmail_label = new QLabel( w0, "sendmail_label" );
                grid1->addWidget(qtarch_sendmail_label,7,0);
	qtarch_sendmail_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_sendmail_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_sendmail_label->setFontPropagation( QWidget::NoChildren );
	qtarch_sendmail_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_sendmail_label->setText( i18n("sendmail command :") );
	qtarch_sendmail_label->setAlignment( 289 );
	qtarch_sendmail_label->setMargin( -1 );

	sendmail_edit = new QLineEdit( w0, "sendmail_edit" );
                grid1->addWidget(sendmail_edit,7,1);
	sendmail_edit->setFocusPolicy( QWidget::StrongFocus );
	sendmail_edit->setBackgroundMode( QWidget::PaletteBase );
	sendmail_edit->setFontPropagation( QWidget::NoChildren );
	sendmail_edit->setPalettePropagation( QWidget::NoChildren );
	sendmail_edit->setText( buginfo.sendmail_command );
	sendmail_edit->setMaxLength( 32767 );
	sendmail_edit->setEchoMode( QLineEdit::Normal );
	sendmail_edit->setFrame( TRUE );

  addTab(w0,i18n("Attention"));

  //+++++++++++++ TAB: General inforamtion +++++++++++++++++++++++++++++++++++++++++

  QWidget* w=new QWidget(this,"General");
  QWhatsThis::add(w, i18n("Fill in all information,\nwe need to help you."));

  QButtonGroup* qtarch_severity_group;
  grid1 = new QGridLayout(w,8,2,15,7);
  qtarch_severity_group = new QButtonGroup( w, "severity_group" );
  qtarch_severity_group->setFocusPolicy( QWidget::NoFocus );
  qtarch_severity_group->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_severity_group->setFontPropagation( QWidget::NoChildren );
  qtarch_severity_group->setPalettePropagation( QWidget::NoChildren );
  qtarch_severity_group->setFrameStyle( 49 );
  qtarch_severity_group->setTitle( i18n("Severity") );
  qtarch_severity_group->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_severity_group,5,7,1,1);

  QButtonGroup* qtarch_priority_group;
  qtarch_priority_group = new QButtonGroup( w, "priority_group" );
  qtarch_priority_group->setFocusPolicy( QWidget::NoFocus );
  qtarch_priority_group->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_priority_group->setFontPropagation( QWidget::NoChildren );
  qtarch_priority_group->setPalettePropagation( QWidget::NoChildren );
  qtarch_priority_group->setFrameStyle( 49 );
  qtarch_priority_group->setTitle( i18n("Priority") );
  qtarch_priority_group->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_priority_group,5,7,0,0);

  name_edit = new QLineEdit( w, "name_edit" );
  grid1->addWidget(name_edit,0,1);
  name_edit->setFocusPolicy( QWidget::StrongFocus );
  name_edit->setBackgroundMode( QWidget::PaletteBase );
  name_edit->setFontPropagation( QWidget::NoChildren );
  name_edit->setPalettePropagation( QWidget::NoChildren );
  name_edit->setText(buginfo.author);
  name_edit->setMaxLength( 32767 );
  name_edit->setEchoMode( QLineEdit::Normal );
  name_edit->setFrame( TRUE );

  email_edit = new QLineEdit( w, "email_edit" );
  grid1->addWidget(email_edit,1,1);
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
  grid1->addWidget(qtarch_name_label,0,0);
  qtarch_name_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_name_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_name_label->setFontPropagation( QWidget::NoChildren );
  qtarch_name_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_name_label->setText( i18n("Your Name :") );
  qtarch_name_label->setAlignment( 289 );
  qtarch_name_label->setMargin( -1 );

  QLabel* qtarch_email_label;
  qtarch_email_label = new QLabel( w, "email_label" );
  grid1->addWidget(qtarch_email_label,1,0);
  qtarch_email_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_email_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_email_label->setFontPropagation( QWidget::NoChildren );
  qtarch_email_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_email_label->setText( i18n("Your E-mail Address :") );
  qtarch_email_label->setAlignment( 289 );
  qtarch_email_label->setMargin( -1 );

  QLabel* qtarch_subject_label;
  qtarch_subject_label = new QLabel( w, "subject_label" );
  grid1->addWidget(qtarch_subject_label,2,0);
  qtarch_subject_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_subject_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_subject_label->setFontPropagation( QWidget::NoChildren );
  qtarch_subject_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_subject_label->setText( i18n("Subject :") );
  qtarch_subject_label->setAlignment( 289 );
  qtarch_subject_label->setMargin( -1 );

  subject_edit = new QLineEdit( w, "subject_edit" );
  grid1->addWidget(subject_edit,2,1);
  subject_edit->setFocusPolicy( QWidget::StrongFocus );
  subject_edit->setBackgroundMode( QWidget::PaletteBase );
  subject_edit->setFontPropagation( QWidget::NoChildren );
  subject_edit->setPalettePropagation( QWidget::NoChildren );
  subject_edit->setMaxLength( 32767 );
  subject_edit->setEchoMode( QLineEdit::Normal );
  subject_edit->setFrame( TRUE );

  QLabel* qtarch_class_label;
  qtarch_class_label = new QLabel( w, "class_label" );
  grid1->addWidget(qtarch_class_label,3,0);
  qtarch_class_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_class_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_class_label->setFontPropagation( QWidget::NoChildren );
  qtarch_class_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_class_label->setText( i18n("Error Class :") );
  qtarch_class_label->setAlignment( 289 );
  qtarch_class_label->setMargin( -1 );

  class_combo = new QComboBox( FALSE, w, "class_combo" );
  grid1->addWidget(class_combo,3,1);
  class_combo->setFocusPolicy( QWidget::StrongFocus );
  class_combo->setBackgroundMode( QWidget::PaletteBackground );
  class_combo->setFontPropagation( QWidget::AllChildren );
  class_combo->setPalettePropagation( QWidget::AllChildren );
  class_combo->setSizeLimit( 10 );
  class_combo->setAutoResize( FALSE );
  // please do not add i18n() to the following 4 lines
  class_combo->insertItem("software bug" );
  class_combo->insertItem( "documentation bug" );
  class_combo->insertItem( "change-request" );
  class_combo->insertItem( "how do I ..." );


  QGridLayout *grid2 = new QGridLayout(qtarch_priority_group,3,1,15,7);
  priority_low = new QRadioButton( /*w*/qtarch_priority_group, "priority_low" );
  grid2->addWidget(priority_low,0,0);
  priority_low->setFocusPolicy( QWidget::TabFocus );
  priority_low->setBackgroundMode( QWidget::PaletteBackground );
  priority_low->setFontPropagation( QWidget::NoChildren );
  priority_low->setPalettePropagation( QWidget::NoChildren );
  priority_low->setText( i18n("low") );
  priority_low->setAutoRepeat( FALSE );
  priority_low->setAutoResize( FALSE );
  priority_low->setChecked( TRUE );

  priority_medium = new QRadioButton( qtarch_priority_group, "priority_medium" );
  grid2->addWidget(priority_medium,1,0);
  priority_medium->setFocusPolicy( QWidget::TabFocus );
  priority_medium->setBackgroundMode( QWidget::PaletteBackground );
  priority_medium->setFontPropagation( QWidget::NoChildren );
  priority_medium->setPalettePropagation( QWidget::NoChildren );
  priority_medium->setText( i18n("medium") );
  priority_medium->setAutoRepeat( FALSE );
  priority_medium->setAutoResize( FALSE );

  priority_high = new QRadioButton( qtarch_priority_group, "priority_high" );
  grid2->addWidget(priority_high,2,0);
  priority_high->setFocusPolicy( QWidget::TabFocus );
  priority_high->setBackgroundMode( QWidget::PaletteBackground );
  priority_high->setFontPropagation( QWidget::NoChildren );
  priority_high->setPalettePropagation( QWidget::NoChildren );
  priority_high->setText( i18n("high") );
  priority_high->setAutoRepeat( FALSE );
  priority_high->setAutoResize( FALSE );


  grid2 = new QGridLayout(qtarch_severity_group,3,1,15,7);
  severity_harmless = new QRadioButton( qtarch_severity_group, "severity_harmless" );
  grid2->addWidget(severity_harmless,0,0);
  severity_harmless->setFocusPolicy( QWidget::TabFocus );
  severity_harmless->setBackgroundMode( QWidget::PaletteBackground );
  severity_harmless->setFontPropagation( QWidget::NoChildren );
  severity_harmless->setPalettePropagation( QWidget::NoChildren );
  severity_harmless->setText( i18n("normal") );
  severity_harmless->setAutoRepeat( FALSE );
  severity_harmless->setAutoResize( FALSE );
  severity_harmless->setChecked( TRUE );

  severity_serious = new QRadioButton( qtarch_severity_group, "severity_serious" );
  grid2->addWidget(severity_serious,1,0);
  severity_serious->setFocusPolicy( QWidget::TabFocus );
  severity_serious->setBackgroundMode( QWidget::PaletteBackground );
  severity_serious->setFontPropagation( QWidget::NoChildren );
  severity_serious->setPalettePropagation( QWidget::NoChildren );
  severity_serious->setText( i18n("grave") );
  severity_serious->setAutoRepeat( FALSE );
  severity_serious->setAutoResize( FALSE );

  severity_critical = new QRadioButton( qtarch_severity_group, "severity_critical" );
  grid2->addWidget(severity_critical,2,0);
  severity_critical->setFocusPolicy( QWidget::TabFocus );
  severity_critical->setBackgroundMode( QWidget::PaletteBackground );
  severity_critical->setFontPropagation( QWidget::NoChildren );
  severity_critical->setPalettePropagation( QWidget::NoChildren );
  severity_critical->setText( i18n("critical") );
  severity_critical->setAutoRepeat( FALSE );
  severity_critical->setAutoResize( FALSE );

  location_combo = new QComboBox( TRUE, w, "location_combo" );
  grid1->addWidget(location_combo,4,1);
  location_combo->setFocusPolicy( QWidget::StrongFocus );
  location_combo->setBackgroundMode( QWidget::PaletteBackground );
  location_combo->setFontPropagation( QWidget::AllChildren );
  location_combo->setPalettePropagation( QWidget::AllChildren );
  location_combo->setSizeLimit( 10 );
  location_combo->setAutoResize( FALSE );
  // please do not add i18n() to the following 13 lines
  location_combo->insertItem("I don´t know" );
  location_combo->insertItem("class tree" );
  location_combo->insertItem( "logical file tree" );
  location_combo->insertItem( "documentation tree" );
  location_combo->insertItem( "editor" );
  location_combo->insertItem( "dialog editor" );
  location_combo->insertItem( "documentation browser" );
  location_combo->insertItem( "app-wizard" );
  location_combo->insertItem( "build process" );
  location_combo->insertItem( "output window" );
  location_combo->insertItem( "configuration" );
  location_combo->insertItem( "project management" );
  location_combo->insertItem( "installation process" );

  QLabel* qtarch_location_label;
  qtarch_location_label = new QLabel( w, "location_label" );
  grid1->addWidget(qtarch_location_label,4,0);
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

  addTab(w,i18n("General"));

  //+++++++++ TAB: System information ++++++++++++++++++++++++++++++++

  QWidget* w2=new QWidget(this,"System");
  grid1 = new QGridLayout(w2,2,2,15,7);
  QButtonGroup* qtarch_environment_group;
  qtarch_environment_group = new QButtonGroup( w2, "environment_group" );
  grid1->addMultiCellWidget(qtarch_environment_group,1,1,0,1);
	qtarch_environment_group->setFocusPolicy( QWidget::NoFocus );
	qtarch_environment_group->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_environment_group->setFontPropagation( QWidget::NoChildren );
	qtarch_environment_group->setPalettePropagation( QWidget::NoChildren );
	qtarch_environment_group->setFrameStyle( 49 );
	qtarch_environment_group->setTitle( i18n("Environment") );
	qtarch_environment_group->setAlignment( 1 );


	QLabel* qtarch_kdevelop_version_label;
	qtarch_kdevelop_version_label = new QLabel( w2, "kdevelop_version_label" );
                grid1->addWidget(qtarch_kdevelop_version_label,0,0);
	qtarch_kdevelop_version_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_kdevelop_version_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_kdevelop_version_label->setFontPropagation( QWidget::NoChildren );
	qtarch_kdevelop_version_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_kdevelop_version_label->setText( i18n("KDevelop version :") );
	qtarch_kdevelop_version_label->setAlignment( 289 );
	qtarch_kdevelop_version_label->setMargin( -1 );

	kdevelop_version_edit = new QLineEdit(w2, "kdevelop_version_edit" );
                grid1->addWidget(kdevelop_version_edit,0,1);
	kdevelop_version_edit->setFocusPolicy( QWidget::StrongFocus );
	kdevelop_version_edit->setBackgroundMode( QWidget::PaletteBase );
	kdevelop_version_edit->setFontPropagation( QWidget::NoChildren );
	kdevelop_version_edit->setPalettePropagation( QWidget::NoChildren );
	kdevelop_version_edit->setText(VERSION); // takes the version number from the config.h macro
	kdevelop_version_edit->setMaxLength( 32767 );
	kdevelop_version_edit->setEchoMode( QLineEdit::Normal );
	kdevelop_version_edit->setFrame( TRUE );

                grid2 = new QGridLayout(qtarch_environment_group,2,7,15,7);
	kde_version_edit = new QLineEdit( qtarch_environment_group, "kde_version_edit" );
                grid2->addWidget(kde_version_edit,0,1);
	kde_version_edit->setFocusPolicy( QWidget::StrongFocus );
	kde_version_edit->setBackgroundMode( QWidget::PaletteBase );
	kde_version_edit->setFontPropagation( QWidget::NoChildren );
	kde_version_edit->setPalettePropagation( QWidget::NoChildren );
	kde_version_edit->setText( buginfo.kde_version);
	kde_version_edit->setMaxLength( 32767 );
	kde_version_edit->setEchoMode( QLineEdit::Normal );
	kde_version_edit->setFrame( TRUE );

	QLabel* qtarch_kde_version_label;
	qtarch_kde_version_label = new QLabel( qtarch_environment_group, "kde_version_label" );
                grid2->addWidget(qtarch_kde_version_label,0,0);
	qtarch_kde_version_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_kde_version_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_kde_version_label->setFontPropagation( QWidget::NoChildren );
	qtarch_kde_version_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_kde_version_label->setText( i18n("KDE version :") );
	qtarch_kde_version_label->setAlignment( 289 );
	qtarch_kde_version_label->setMargin( -1 );

	qt_version_edit = new QLineEdit( qtarch_environment_group, "qt_version_edit" );
                grid2->addWidget(qt_version_edit,1,1);
	qt_version_edit->setFocusPolicy( QWidget::StrongFocus );
	qt_version_edit->setBackgroundMode( QWidget::PaletteBase );
	qt_version_edit->setFontPropagation( QWidget::NoChildren );
	qt_version_edit->setPalettePropagation( QWidget::NoChildren );
	qt_version_edit->setText(buginfo.qt_version );
	qt_version_edit->setMaxLength( 32767 );
	qt_version_edit->setEchoMode( QLineEdit::Normal );
	qt_version_edit->setFrame( TRUE );

	QLabel* qtarch_qt_version_label;
	qtarch_qt_version_label = new QLabel( qtarch_environment_group, "qt_version_label" );
                grid2->addWidget(qtarch_qt_version_label,1,0);
	qtarch_qt_version_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_qt_version_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_qt_version_label->setFontPropagation( QWidget::NoChildren );
	qtarch_qt_version_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_qt_version_label->setText( i18n("QT version :") );
	qtarch_qt_version_label->setAlignment( 289 );
	qtarch_qt_version_label->setMargin( -1 );

	os_edit = new QLineEdit( qtarch_environment_group, "os_edit" );
                grid2->addWidget(os_edit,2,1);
	os_edit->setFocusPolicy( QWidget::StrongFocus );
	os_edit->setBackgroundMode( QWidget::PaletteBase );
	os_edit->setFontPropagation( QWidget::NoChildren );
	os_edit->setPalettePropagation( QWidget::NoChildren );
	os_edit->setText( buginfo.os );
	os_edit->setMaxLength( 32767 );
	os_edit->setEchoMode( QLineEdit::Normal );
	os_edit->setFrame( TRUE );

	QLabel* qtarch_os_label;
	qtarch_os_label = new QLabel( qtarch_environment_group, "os_label" );
                grid2->addWidget(qtarch_os_label,2,0);
	qtarch_os_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_os_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_os_label->setFontPropagation( QWidget::NoChildren );
	qtarch_os_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_os_label->setText( i18n("OS / distribution :") );
	qtarch_os_label->setAlignment( 289 );
	qtarch_os_label->setMargin( -1 );

	QLabel* qtarch_compiler_label;
	qtarch_compiler_label = new QLabel( qtarch_environment_group, "compiler_label" );
                grid2->addWidget(qtarch_compiler_label,3,0);
	qtarch_compiler_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_compiler_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_compiler_label->setFontPropagation( QWidget::NoChildren );
	qtarch_compiler_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_compiler_label->setText( i18n("Compiler :") );
	qtarch_compiler_label->setAlignment( 289 );
	qtarch_compiler_label->setMargin( -1 );

	compiler_edit = new QLineEdit( qtarch_environment_group, "compiler_edit" );
                grid2->addWidget(compiler_edit,3,1);
	compiler_edit->setFocusPolicy( QWidget::StrongFocus );
	compiler_edit->setBackgroundMode( QWidget::PaletteBase );
	compiler_edit->setFontPropagation( QWidget::NoChildren );
	compiler_edit->setPalettePropagation( QWidget::NoChildren );
	compiler_edit->setText( buginfo.compiler );
	compiler_edit->setMaxLength( 32767 );
	compiler_edit->setEchoMode( QLineEdit::Normal );
	compiler_edit->setFrame( TRUE );

	QLabel* qtarch_misc_label;
	qtarch_misc_label = new QLabel( qtarch_environment_group, "misc_label" );
                grid2->addWidget(qtarch_misc_label,4,0);
	qtarch_misc_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_misc_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_misc_label->setFontPropagation( QWidget::NoChildren );
	qtarch_misc_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_misc_label->setText( i18n("Miscalleneous :") );
	qtarch_misc_label->setAlignment( 289 );
	qtarch_misc_label->setMargin( -1 );

	misc_mledit = new QMultiLineEdit( qtarch_environment_group, "misc_edit" );
                grid2->addMultiCellWidget(misc_mledit,4,6,1,1);
	misc_mledit->setFocusPolicy( QWidget::StrongFocus );
	misc_mledit->setBackgroundMode( QWidget::PaletteBase );
	misc_mledit->setFontPropagation( QWidget::SameFont );
	misc_mledit->setPalettePropagation( QWidget::SameFont );
	misc_mledit->insertLine( "" );
	misc_mledit->setReadOnly( FALSE );
	misc_mledit->setOverwriteMode( FALSE );

	addTab(w2,i18n("System"));


  //+++++++++ TAB: Problem description+++++++++++++++++++++++++++++++++++++
  QWidget* w3=new QWidget(this,"Problem description");
  QWhatsThis::add(w3, i18n("Insert as much information about your\nproblem, so that we are able to help by\nyour description."));
   grid1 = new QGridLayout(w3,15,1,15,7);
	description_mledit = new QMultiLineEdit( w3, "description_mledit" );
                grid1->addMultiCellWidget(description_mledit,1,5,0,0);
	description_mledit->setFocusPolicy( QWidget::StrongFocus );
	description_mledit->setBackgroundMode( QWidget::PaletteBase );
	description_mledit->setFontPropagation( QWidget::SameFont );
	description_mledit->setPalettePropagation( QWidget::SameFont );
	description_mledit->insertLine( "" );
	description_mledit->setReadOnly( FALSE );
	description_mledit->setOverwriteMode( FALSE );

	QLabel* qtarch_description_label;
	qtarch_description_label = new QLabel( w3, "description_label" );
                grid1->addWidget(qtarch_description_label,0,0);
	qtarch_description_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_description_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_description_label->setFontPropagation( QWidget::NoChildren );
	qtarch_description_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_description_label->setText( i18n("Bug Description :") );
	qtarch_description_label->setAlignment( 289 );
	qtarch_description_label->setMargin( -1 );

	QLabel* qtarch_repeat_label;
	qtarch_repeat_label = new QLabel( w3, "repeat_label" );
                grid1->addWidget(qtarch_repeat_label,6,0);
	qtarch_repeat_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_repeat_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_repeat_label->setFontPropagation( QWidget::NoChildren );
	qtarch_repeat_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_repeat_label->setText( i18n("How to repeat the error :") );
	qtarch_repeat_label->setAlignment( 289 );
	qtarch_repeat_label->setMargin( -1 );

	repeat_mledit = new QMultiLineEdit( w3, "repeat_mledit" );
                grid1->addMultiCellWidget(repeat_mledit,7,10,0,0);

	repeat_mledit->setFocusPolicy( QWidget::StrongFocus );
	repeat_mledit->setBackgroundMode( QWidget::PaletteBase );
	repeat_mledit->setFontPropagation( QWidget::SameFont );
	repeat_mledit->setPalettePropagation( QWidget::SameFont );
	repeat_mledit->insertLine( "" );
	repeat_mledit->setReadOnly( FALSE );
	repeat_mledit->setOverwriteMode( FALSE );

	QLabel* qtarch_fix_label;
	qtarch_fix_label = new QLabel( w3, "fix_label" );
                grid1->addWidget(qtarch_fix_label,11,0);

	qtarch_fix_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_fix_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_fix_label->setFontPropagation( QWidget::NoChildren );
	qtarch_fix_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_fix_label->setText( i18n("Perhaps a bugfix or workaround ?") );
	qtarch_fix_label->setAlignment( 289 );
	qtarch_fix_label->setMargin( -1 );

	fix_mledit = new QMultiLineEdit( w3, "fix_mledit" );
                grid1->addMultiCellWidget(fix_mledit,12,14,0,0);
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
}

CBugReportDlg::~CBugReportDlg(){
}

void CBugReportDlg::ok() {

  if (description_mledit->text() == ""  ||  subject_edit->text() == "") {
    KMessageBox::information(this,i18n("Please fill in at least the subject and bug description!"));
    return;
  }
  QFile f( sendmail_edit->text() );
  if ( !f.exists() ) {
  	if ( KMessageBox::No == KMessageBox::questionYesNo(this,
  	                            i18n("KDevelop couldn't find sendmail at the given location.\n"\
                                        "If you just want to generate the bugreport and send it\n"\
                                        "by hand later, you can continue.\n"\
                                        "Otherwise press Cancel and retype the sendmail command."),
                                        i18n("Continue"),i18n("Cancel")) )
  	{
  		return;
  	}
  }
  email_address = email_edit->text();
  name = name_edit->text();
  os = os_edit->text();
  compiler = compiler_edit->text();
  kde_version = kde_version_edit->text();
  qt_version = qt_version_edit->text();
  sendmail_command = sendmail_edit->text();
  
  if (generateEmail()) {
    if(sendEmail()){
      KMessageBox::information(this,
                    i18n("Bugreport was successfully submitted to the KDevelop Team.\n\t\tThank you!"),
                    i18n("Bug Report"));
    }
  }
  accept();
}

bool CBugReportDlg::generateEmail() {

  QString text;
  // header
  text+="FROM: ";text+=name_edit->text();text+=" <";text+=email_edit->text();text+=">\n";
	text+="TO: ";text+=BugEmail;text+="\n";
  text+="SUBJECT: ";text+=subject_edit->text();text+="\n";
  text+="\n";
  // body
  text.append("Package: kdevelop\n");
  text.append("Version: ");text.append(kdevelop_version_edit->text());text.append("\n");
  text.append("Severity: ");
  cerr << endl << "if";
  cerr << endl << ":" << class_combo->currentText() << ":";
  if ( QString(class_combo->currentText())==QString("change-request") ) {
  cerr << "yes";
  	text.append("wishlist\n\n");
  } else {
  	if (severity_harmless->isChecked()) text.append("normal\n\n");
  	if (severity_serious->isChecked()) text.append("grave\n\n");
  	if (severity_critical->isChecked()) text.append("critical\n\n");
  }
  text.append("Bugreport ID : "+strBugID+"\n\n");
  text.append("Originator\t: ");text.append(name_edit->text());text.append("\n");
  text.append("E-Mail\t\t: ");text.append(email_edit->text());text.append("\n\n");
  text.append("Subject : ");text.append(subject_edit->text());text.append("\n\n");
  text.append("Error Class\t: ");text.append(class_combo->currentText());text.append("\n");
  text.append("Error Location\t: ");text.append(location_combo->currentText());text.append("\n");
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
  text.append("OS/Distribution\t\t: ");text.append(os_edit->text());text.append("\n");
  text.append("Compiler\t\t: ");text.append(compiler_edit->text());text.append("\n\n");
  text+="misc :\n";
  text+=misc_mledit->text();
  QDir dir(locateLocal("data",""));
  dir.mkdir("kdevelop");
  QFile file(locateLocal("appdata", "bugreport."+strBugID));
  if (!file.open(IO_WriteOnly)) {
    return false;
  }
  file.writeBlock(text,text.length());
  file.close();

  return true;
}


bool CBugReportDlg::sendEmail() {

	QString command("cat ");
	command.append(locateLocal("appdata", "bugreport."+strBugID));
  command+=" | ";
  command+=sendmail_edit->text();
  command+=" -t";
  cerr << command;
  KShellProcess *process2=new KShellProcess;
  *process2 << command;
  process2->start(KProcess::Block,KProcess::NoCommunication);

  return true;
}
