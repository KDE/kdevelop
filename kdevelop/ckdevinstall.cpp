/***************************************************************************
                          ckdevinstall.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Mar 4 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <qfile.h>
#include <qfileinfo.h>
#include <qprogressdialog.h>

#include <kapp.h>
#include <kmsgbox.h>
#include <kfiledialog.h>

#include "ckdevinstall.h"
#include "ctoolclass.h"


CKDevInstall::CKDevInstall(QWidget *parent, const char *name ) : QDialog(parent,name,true) {

	config = kapp->getConfig();

  setCaption(i18n("KDevelop Installation"));

  main_frame = new QFrame( this, "Frame_2" );
	main_frame->setGeometry( 10, 10, 500, 290 );
	{
		QColorGroup normal( QColor( QRgb(0) ), QColor( QRgb(128) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QColorGroup disabled( QColor( QRgb(8421504) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
		QColorGroup active( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		main_frame->setPalette( palette );
	}
	main_frame->setFrameStyle( 33 );
	main_frame->setLineWidth( 2 );


	welcome_label = new QLabel( this, "Label_2" );
	welcome_label->setGeometry( 40, 30, 440, 40 );
	{
		QColorGroup normal( QColor( QRgb(0) ), QColor( QRgb(8388608) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ) );
		QColorGroup disabled( QColor( QRgb(8421504) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
		QColorGroup active( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		welcome_label->setPalette( palette );
	}
	{
		QFont font( "Lucida", 18, 50, 0 );
		font.setStyleHint( (QFont::StyleHint)0 );
		font.setCharSet( (QFont::CharSet)1 );
		welcome_label->setFont( font );
	}
	welcome_label->setFrameStyle( 33 );
	welcome_label->setText(i18n("Welcome to the KDevelop Setup !"));
	welcome_label->setAlignment( 292 );
	welcome_label->setMargin( -1 );

	qt_label = new QLabel( this, "Label_3" );
	qt_label->setGeometry( 40, 100, 220, 30 );
	{
		QColorGroup normal( QColor( QRgb(0) ), QColor( QRgb(128) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ) );
		QColorGroup disabled( QColor( QRgb(8421504) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
		QColorGroup active( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		qt_label->setPalette( palette );
	}
	{
		QFont font( "LucidaTypewriter", 12, 50, 0 );
		font.setStyleHint( (QFont::StyleHint)0 );
		font.setCharSet( (QFont::CharSet)0 );
		qt_label->setFont( font );
	}
	qt_label->setText(i18n("Qt-Documentation Path:"));
	qt_label->setAlignment( 289 );
	qt_label->setMargin( -1 );

	config->setGroup("Doc_Location");
	QString qt_doc=config->readEntry("doc_qt","");
	if(qt_doc.isEmpty())
	  qt_test=true;
	qt_edit = new QLineEdit( this, "LineEdit_1" );
	qt_edit->setGeometry( 270, 100, 160, 30 );
	qt_edit->setText( qt_doc );
	qt_edit->setMaxLength( 32767 );
	qt_edit->setEchoMode( QLineEdit::Normal );
	qt_edit->setFrame( FALSE );

	qt_button = new QPushButton( this, "PushButton_4" );
	qt_button->setGeometry( 440, 100, 40, 30 );
	connect( qt_button, SIGNAL(pressed()), SLOT(slotQTpressed()) );
	qt_button->setText( "..." );
	qt_button->setAutoRepeat( FALSE );
	qt_button->setAutoResize( FALSE );


	kde_label = new QLabel( this, "Label_4" );
	kde_label->setGeometry( 40, 140, 220, 30 );
	{
		QColorGroup normal( QColor( QRgb(0) ), QColor( QRgb(128) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ) );
		QColorGroup disabled( QColor( QRgb(8421504) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
		QColorGroup active( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		kde_label->setPalette( palette );
	}
	{
		QFont font( "LucidaTypewriter", 12, 50, 0 );
		font.setStyleHint( (QFont::StyleHint)0 );
		font.setCharSet( (QFont::CharSet)0 );
		kde_label->setFont( font );
	}
	kde_label->setText(i18n("KDE-Documentation Path:"));
	kde_label->setAlignment( 289 );
	kde_label->setMargin( -1 );

	config->setGroup("Doc_Location");
	QString kde_doc=config->readEntry("doc_kde","");
	if(kde_doc.isEmpty())
	  kde_test=true;
	kde_edit = new QLineEdit( this, "LineEdit_2" );
	kde_edit->setGeometry( 270, 140, 160, 30 );
	kde_edit->setText( kde_doc );
	kde_edit->setMaxLength( 32767 );
	kde_edit->setEchoMode( QLineEdit::Normal );
	kde_edit->setFrame( TRUE );

	kde_button = new QPushButton( this, "PushButton_5" );
	kde_button->setGeometry( 440, 140, 40, 30 );
	connect( kde_button, SIGNAL(pressed()), SLOT(slotKDEpressed()) );
	kde_button->setText("...");
	kde_button->setAutoRepeat( FALSE );
	kde_button->setAutoResize( FALSE );

	hint_label = new QLabel( this, "Label_5" );
	hint_label->setGeometry( 40, 190, 440, 90 );
	{
		QColorGroup normal( QColor( QRgb(0) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QColorGroup disabled( QColor( QRgb(8421504) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
		QColorGroup active( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		hint_label->setPalette( palette );
	}
	{
		QFont font( "LucidaTypewriter", 12, 50, 0 );
		font.setStyleHint( (QFont::StyleHint)0 );
		font.setCharSet( (QFont::CharSet)0 );
		hint_label->setFont( font );
	}
	hint_label->setFrameStyle( 33 );
	hint_label->setText(i18n("This installation program lets you set the correct path "
	                          "to your documentations and check for needed helper programs."
	                          "  Choose 'Proceed` to invoke the automatic detection and "
	                          "documentation generator. If you press the right mouse button over"
	                          " the buttons or 'Help' you will get more specific information.") );
	hint_label->setAlignment( 1316 );
	hint_label->setMargin( -1 );

	help_button = new QPushButton( this, "PushButton_6" );
	help_button->setGeometry( 50, 310, 110, 30 );
	connect( help_button, SIGNAL(pressed()), SLOT(slotHelp()) );
	help_button->setText(i18n("Help") );
	help_button->setAutoRepeat( FALSE );
	help_button->setAutoResize( FALSE );
	
	auto_button = new QPushButton( this, "PushButton_1" );
	auto_button->setGeometry( 200, 310, 120, 30 );
	connect( auto_button, SIGNAL(pressed()), SLOT(slotAuto()) );
	auto_button->setText(i18n("Proceed >>"));
	auto_button->setAutoRepeat( FALSE );
	auto_button->setAutoResize( FALSE );

	cancel_button = new QPushButton( this, "PushButton_3" );
	cancel_button->setGeometry( 360, 310, 110, 30 );
	connect( cancel_button, SIGNAL(pressed()), SLOT(slotCancel()) );
	cancel_button->setText( "Cancel" );
	cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );

	resize( 520, 360 );

}
CKDevInstall::~CKDevInstall(){
}

void CKDevInstall::slotQTpressed()
{
  QString dir;
  dir = KFileDialog::getDirectory(config->readEntry("doc_qt"));
  if (!dir.isEmpty()){
    qt_edit->setText(dir);
    config->setGroup("Doc_Location");

    QString qt_testfile=dir+"classes.html"; // test if the path really is the qt-doc path
    if(QFileInfo(qt_testfile).exists())
      config->writeEntry("doc_qt",dir);
    else{
      KMsgBox::message(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "Qt-library documentation. Please choose the\n"
                                                              "correct path."),KMsgBox::EXCLAMATION);
    }
  }
}

void CKDevInstall::slotKDEpressed()
{
  QString dir;
  dir = KFileDialog::getDirectory(config->readEntry("doc_kde"));
  if (!dir.isEmpty()){
    kde_edit->setText(dir);
    config->setGroup("Doc_Location");

    QString kde_testfile=dir+"kdecore/index.html"; // test if the path really is the qt-doc path
    if(QFileInfo(kde_testfile).exists())
      config->writeEntry("doc_kde",dir);
    else{
      KMsgBox::message(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "KDE-library documentation. Please choose 'Proceed'\n"
                                                              "in any case. If you don't have a documentation,\n"
                                                              "it will be generated automatically in one of the\n"
                                                              "next steps"),KMsgBox::EXCLAMATION);
    }
  }

}
void CKDevInstall::slotHelp()
{

  kapp->invokeHTMLHelp("kdevelop/index-3.html", "ss3.7.3" );
}


void CKDevInstall::slotAuto() // proceed >>
{
  int numProgs=14; // number of installed Programs
  bool make=false;
  bool gmake=false;
  bool autoconf=false;
  bool autoheader=false;
  bool automake=false;
  bool perl=false;
  bool sgml2html=false;
  bool ksgml2html=false;
  bool kdoc=false;
  bool glimpse=false;
  bool glimpseindex=false;
  bool kdbg=false;
  bool kiconedit=false;
  bool ktranslator=false;

  QProgressDialog progress( i18n("Checking needed programs..."), i18n("Cancel"), numProgs, this );
  for (int i=0; i<numProgs; i++) {
  progress.setProgress( i );
  if ( progress.wasCancelled() )
    break;
    if(CToolClass::searchInstProgram("make")){
      make=true;
    }
  }
  progress.setProgress( numProgs );

  if(CToolClass::searchInstProgram("gmake")){
    gmake=true;
  }
  if(CToolClass::searchInstProgram("autoconf")){
    autoconf=true;
  }
  if(CToolClass::searchInstProgram("autoheader")){
    autoheader=true;
  }
  if(CToolClass::searchInstProgram("automake")){
    automake=true;
  }
  if(CToolClass::searchInstProgram("perl")){
    perl=true;
  }
  if(CToolClass::searchInstProgram("sgml2html")){
    sgml2html=true;
  }
  if(CToolClass::searchInstProgram("ksgml2html")){
    ksgml2html=true;
  }
  if(CToolClass::searchInstProgram("kdoc")){
    kdoc=true;
  }
  if(CToolClass::searchInstProgram("glimpse")){
    glimpse=true;
  }
  if(CToolClass::searchInstProgram("glimpseindex")){
    glimpseindex=true;
  }
  if(CToolClass::searchInstProgram("kdbg")){
    kdbg=true;
  }
  if(CToolClass::searchInstProgram("kiconedit")){
    kiconedit=true;
  }
  if(CToolClass::searchInstProgram("ktranslator")){
    ktranslator=true;
  }

}
void CKDevInstall::slotCancel()
{
  config->setGroup("General Options");
  config->writeEntry("Install",!successful);
  close();
}














