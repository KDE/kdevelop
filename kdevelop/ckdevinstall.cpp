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

#include "ckdevinstall.h"
#include "ctoolclass.h"
#include "cupdatekdedocdlg.h"
#include "ccreatedocdatabasedlg.h"

#include <kapp.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstddirs.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlined.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qstringlist.h>
//#include <qtimer.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

void CKDevInstall::slotReceivedStdout(KProcess*,char* /*buffer*/,int){
  
}
void CKDevInstall::slotReceivedStderr(KProcess*,char*,int){
}
void CKDevInstall::slotProcessExited(KProcess*){
  QDir* finish_dir=new QDir();
    if (!finished_glimpse){
	finished_glimpse=true;
	if((!glimpse || !glimpseindex) && (!htdig || !htsearch)){
	    KMessageBox::information(this, i18n("Neither glimpse nor htdig is installed,\n"
							    "therefore KDevelop can not index your\n"
							    "documentation to provide a proper help\n"
							    "functionality. We advise to install glimpse\n"
							    "or htdig and create the searchdatabase later by\n"
							    "choosing KDevelop Setup in the Options menu.\n\n"
							    "As this is the last step of the Installation\n"
							    "process, KDevelop will be started now.\n\n"
							    "We hope you enjoy KDevelop and that it is a useful\n"
							    "help to create new software.\n\nThe KDevelop Team"));
	    
	    config->setGroup("General Options");
	    config->writeEntry("Install",false);
			config->sync();
	    finish_dir->setCurrent(QDir::homeDirPath ());
	    accept();
	}
	else{
	    KMessageBox::information(this,i18n("Now KDevelop will perform the last step\n"
							   "of the installation process with indexing\n"
							   "your documentation. This will provide an extended\n"
							   "help functionality and will give you the information\n"
							   "you need."));
	    
	    CCreateDocDatabaseDlg dlg(this,"DLG",shell_process,config, glimpse | glimpseindex, htdig);
	    if(!dlg.exec()){
		slotProcessExited(shell_process);
	    }
	    hint_label->setText(i18n("                 Creating search database                      "
	                          "                                                             "
	                          "                      Please wait...                           "
	                          "                                                               "
	                          "                                                               ") );
	    return;
	}
	
    }
    
    
    KMessageBox::information(this,i18n("\nThe installation process finished successfully.\n\n"
                                       "The KDevelop Team wishes that you will enjoy our\n"
                                       "program and we would be honoured for any feedback.\n\n"
                                       "The KDevelop Team"),
                                    i18n("Installation successful !"));

  config->setGroup("General Options");
  config->writeEntry("Install",false);
	config->sync();
  finish_dir->setCurrent(QDir::homeDirPath ());
  accept();
}

CKDevInstall::CKDevInstall(QWidget *parent, const char *name ) : QDialog(parent,name,true) {
    // shellprocess
    finished_glimpse=false;

    qt_test=kde_test=true; // assuming worst case... everything must be checked
    till_doc=false; // first we must run through all, maybe the second time only from qt-doc-search

    shell_process = new KShellProcess();
    connect(shell_process,SIGNAL(receivedStdout(KProcess*,char*,int)),
	    this,SLOT(slotReceivedStdout(KProcess*,char*,int)) );
    
    connect(shell_process,SIGNAL(receivedStderr(KProcess*,char*,int)),
	    this,SLOT(slotReceivedStderr(KProcess*,char*,int)) );
    
    connect(shell_process,SIGNAL(processExited(KProcess*)),
	    this,SLOT(slotProcessExited(KProcess*) )) ;
    
    config = KGlobal::config();


	
  setCaption(i18n("KDevelop Installation"));

  main_frame = new QFrame( this, "Frame_2" );
	main_frame->setGeometry( 10, 10, 500, 290 );
	{
		QColorGroup normal( QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(128) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ) );
		QColorGroup disabled( QColor((unsigned int) QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ) );
		QColorGroup active( QColor( (unsigned int)(unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		main_frame->setPalette( palette );
	}
	main_frame->setFrameStyle( 33 );
	main_frame->setLineWidth( 2 );


	welcome_label = new QLabel( this, "Label_2" );
	welcome_label->setGeometry( 40, 30, 440, 40 );
	{
		QColorGroup normal( QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(8388608) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(16777215) ) );
		QColorGroup disabled( QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ) );
		QColorGroup active( QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ) );
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
		QColorGroup normal( QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(128) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)(unsigned int)(unsigned int)QRgb(16777215) ) );
		QColorGroup disabled( QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ) );
		QColorGroup active( QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ) );
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
	QString qt_doc=config->readEntry("doc_qt", QT_DOCDIR);
	qt_edit = new QLineEdit( this, "LineEdit_1" );
	qt_edit->setGeometry( 270, 100, 160, 30 );
	qt_edit->setText( qt_doc );
	qt_edit->setMaxLength( 32767 );
	qt_edit->setEchoMode( QLineEdit::Normal );
	qt_edit->setFrame( FALSE );

	qt_button = new QPushButton( this, "PushButton_4" );
	qt_button->setGeometry( 440, 100, 40, 30 );
	QPixmap pix = BarIcon("open");
	qt_button->setPixmap(pix);
	qt_button->setAutoRepeat( FALSE );
	qt_button->setAutoResize( FALSE );
	connect( qt_button, SIGNAL(clicked()), SLOT(slotQTpressed()) );


/*	kde_label = new QLabel( this, "Label_4" );
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
	QString kde_doc=config->readEntry("doc_kde",KDELIBS_DOCDIR);
	kde_edit = new QLineEdit( this, "LineEdit_2" );
	kde_edit->setGeometry( 270, 140, 160, 30 );
	kde_edit->setText( kde_doc );
	kde_edit->setMaxLength( 32767 );
	kde_edit->setEchoMode( QLineEdit::Normal );
	kde_edit->setFrame( TRUE );

	kde_button = new QPushButton( this, "PushButton_5" );
	kde_button->setGeometry( 440, 140, 40, 30 );
	kde_button->setText("...");
	kde_button->setAutoRepeat( FALSE );
	kde_button->setAutoResize( FALSE );
	connect( kde_button, SIGNAL(clicked()), SLOT(slotKDEpressed()) );
*/
	hint_label = new QLabel( this, "Label_5" );
	hint_label->setGeometry( 40, 100, 440, 170 );
	{
	  QColorGroup normal( QColor( (unsigned int)QRgb(0) ), QColor((unsigned int) QRgb(16777215) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ) );
	  QColorGroup disabled( QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(12632256) ) );
	  QColorGroup active( QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(12632256) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(10789024) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ) );
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
	hint_label->setText(i18n("This installation program will check your system for helper programs needed by KDevelop. "
				"Also your documentation will be set up automatically."
				 "  Choose 'Proceed` to invoke the automatic detection and the "
				 "documentation generator. 'Help' will give more information about the installation."
				 " Choosing 'Cancel' will stop the installation and KDevelop will be started without setting the initial"
				 "values needed for proper functionality.") );
	hint_label->setAlignment( 1316 );
	hint_label->setMargin( -1 );
	
	help_button = new QPushButton( this, "PushButton_6" );
	help_button->setGeometry( 50, 310, 110, 30 );
	help_button->setText(i18n("Help") );
	help_button->setAutoRepeat( FALSE );
	help_button->setAutoResize( FALSE );
	connect( help_button, SIGNAL(clicked()), SLOT(slotHelp()) );
	
	auto_button = new QPushButton( this, "PushButton_1" );
	auto_button->setGeometry( 200, 310, 120, 30 );
	auto_button->setText(i18n("Proceed >>"));
	auto_button->setAutoRepeat( false );
	auto_button->setAutoResize( FALSE );
	auto_button->setDefault( TRUE );
	connect( auto_button, SIGNAL(clicked()), SLOT(slotAuto()) );

	cancel_button = new QPushButton( this, "PushButton_3" );
	cancel_button->setGeometry( 360, 310, 110, 30 );
	connect( cancel_button, SIGNAL(clicked()), SLOT(slotCancel()) );
	cancel_button->setText(i18n("Cancel") );
	cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );
//	  setGeometry(QApplication::desktop()->width()/2-260, QApplication::desktop()->height()/2-180, 520,360);

	resize( 520, 360 );

}
CKDevInstall::~CKDevInstall(){
}

void CKDevInstall::slotQTpressed()
{
  QString dir;
  dir = KFileDialog::getExistingDirectory(config->readEntry("doc_qt", QT_DOCDIR));
  if (!dir.isEmpty()){
    qt_edit->setText(dir);
    config->setGroup("Doc_Location");

    QString qt_testfile=dir+"/classes.html"; // test if the path really is the qt-doc path
    if(QFileInfo(qt_testfile).exists()){
      config->writeEntry("doc_qt",dir);
      qt_test=false;
      auto_button->setEnabled(true);
    }
    else{
      KMessageBox::error(this,i18n("\nThe chosen path does not lead to the\n"
                                   "Qt-library documentation. Please choose the\n"
                                   "correct path."),
                                i18n("The selected path is not correct!"));
    }
  }
}

void CKDevInstall::slotKDEpressed()
{
  QString dir;
  dir = KFileDialog::getExistingDirectory(config->readEntry("doc_kde", KDELIBS_DOCDIR));
  if (!dir.isEmpty()){
    kde_edit->setText(dir);
    config->setGroup("Doc_Location");

    QString kde_testfile=dir+"/kdecore/index.html"; // test if the path really is the kde-doc path
    if(QFileInfo(kde_testfile).exists()){
      config->writeEntry("doc_kde",dir);
      kde_test=false;
    }
    else{
      KMessageBox::error(this,i18n("\nThe chosen path does not lead to the\n"
                                   "KDE-library documentation. Please choose 'Proceed'\n"
                                   "in any case. If you don't have a documentation,\n"
                                   "it will be generated automatically in one of the\n"
                                   "next steps"),i18n("The selected path is not correct!"));
    }
  }

}
void CKDevInstall::slotHelp()
{

  kapp->invokeHTMLHelp("kdevelop/index-2.html", "ss2.3" );
}


void CKDevInstall::slotAuto() // proceed >>
{
  hint_label->setText(i18n("        Automatic installation in progress... "));
  int numProgs=100; // number of installed Programs
  bool make=false;
  bool gmake=false;
  bool autoconf=false;
  bool autoheader=false;
  bool automake=false;
  bool perl=false;
  bool sgml2html=false;
  bool ksgml2html=false;
  bool kdoc=false;
  glimpse=false;
  glimpseindex=false;
  htdig=false;
  htsearch=false;
  bool a2ps=false;
  bool enscript=false;
  bool dbg=false;
  bool kiconedit=false;
  bool ktranslator=false;
	bool kbabel=false;
	bool gimp=false;
	bool kpaint=false;
	bool designer=false;
	bool linguist=false;
	
  auto_button->setEnabled(false);

  if (!till_doc) // jump to the new entry point if qt-doc first wasn't found
  {

  int highl_style=KMessageBox::questionYesNo(this,i18n("\nNow you can choose the Syntax-Highlighting style\n"
                                                       "KDevelop will use. The options are to set\n"
                                                       "the highlighting to Emacs style or to the default\n"
                                                       "settings of KWrite (k.a. 'Extended Editor')\n\n"
                                                       "Which one do you want to use ?"),
                                                        i18n("Syntax-Highlighting"),
                                                        i18n("Emacs style"),
                                                        i18n("KWrite default"));
  if(highl_style==KMessageBox::Yes){
    config->setGroup("Perl Highlight");
    config->writeEntry("Mimetypes","application/x-perl");
    config->writeEntry("Wildcards","");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Variable","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Operator","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,èñf@");

    config->setGroup("C++ Highlight");
    config->writeEntry("Prep. Lib","0,808000,FFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Char","0,CC07,CC07,0,0,1,courier,12,Èñf@");
    config->writeEntry("Data Type","0,AD466A,AD466A,0,0,1,courier,12,Èñf@");
    config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Mimetypes","text/x-c++-src;text/x-c++-hdr;text/x-c-hdr");
    config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Preprocessor","0,800080,800080,0,0,1,courier,12,Èñf@");
    config->writeEntry("Wildcards","*.cpp;*.cc;*.cxx;*.CPP;*.CC;*.CXX;*.h;*.hxx;*.H;*.HXX;*.ecpp");
    config->writeEntry("Comment","0,E0,D7,0,0,1,courier,12,Èñf@");
    config->writeEntry("String","0,8000,8000,0,0,1,courier,12,Èñf@");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Keyword","0,A7AD,A7AD,0,0,1,courier,12,Èñf@");
    config->writeEntry("String Char","0,8000,8000,0,0,1,courier,12,Èñf@");

    config->setGroup("Ada Highlight");
    config->writeEntry("Base-N","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Mimetypes","text/x-ada-src");
    config->writeEntry("Wildcards","*.a");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,èñf@");

    config->setGroup("Modula 2 Highlight");
    config->writeEntry("Mimetypes","text/x-modula-2-src");
    config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Wildcards","*.md;*.mi");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,èñf@");
	
		config->setGroup("IDL Highlight");
    config->writeEntry("Prep. Lib","0,808000,FFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Char","0,CC07,CC07,0,0,1,courier,12,Èñf@");
    config->writeEntry("Data Type","0,AD466A,AD466A,0,0,1,courier,12,Èñf@");
    config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Mimetypes","text/x-idl-src");
    config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Preprocessor","0,800080,800080,0,0,1,courier,12,Èñf@");
    config->writeEntry("Wildcards","*.idl");
    config->writeEntry("Comment","0,E0,D7,0,0,1,courier,12,Èñf@");
    config->writeEntry("String","0,8000,8000,0,0,1,courier,12,Èñf@");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,Èñf@");
    config->writeEntry("Keyword","0,A7AD,A7AD,0,0,1,courier,12,Èñf@");
    config->writeEntry("String Char","0,8000,8000,0,0,1,courier,12,Èñf@");


    config->setGroup("Java Highlight");
    config->writeEntry("Prep. Lib","0,8080,FFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Data Type","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Mimetypes","text/x-java-src");
    config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Preprocessor","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Wildcards","*.java");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,èñf@");

    config->setGroup("Normal Highlight");
    config->writeEntry("Mimetypes","");
    config->writeEntry("Wildcards","");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");

    config->setGroup("C Highlight");
    config->writeEntry("Prep. Lib","0,8080,FFFF,0,0,1,courier,12,");
    config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Data Type","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Mimetypes","text/x-c-src");
    config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Preprocessor","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Wildcards","*.c;*.C;*.ec");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,");

    config->setGroup("HTML Highlight");
    config->writeEntry("Tag Text","0,0,FFFFFF,1,0,1,courier,12,");
    config->writeEntry("Char","0,8000,FF00,0,0,1,courier,12,");
    config->writeEntry("Tag","0,800080,FF00FF,1,0,1,courier,12,");
    config->writeEntry("Mimetypes","text/html");
    config->writeEntry("Wildcards","*.html;*.htm");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Tag Value","0,808000,FFFF00,0,0,1,courier,12,");

    config->setGroup("Python Highlight");
    config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Mimetypes","text/x-python-src");
    config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Wildcards","*.py");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,èñf@");
    config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,èñf@");

    config->setGroup("Bash Highlight");
    config->writeEntry("Integer","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Mimetypes","text/x-shellscript");
    config->writeEntry("Substitution","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Wildcards","");
    config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");

    config->setGroup("KWrite Options");
    config->writeEntry("WrapAt","78");
    config->writeEntry("ConfigFlags","1073");
    config->writeEntry("UndoSteps","50");
    config->writeEntry("Highlight","C++");
    config->writeEntry("SearchFlags","32");
    config->writeEntry("Color0","255,255,255");
    config->writeEntry("Color1","0,0,128");
    config->writeEntry("Color2","0,0,0");
    config->writeEntry("Color3","0,0,0");
    config->writeEntry("TabWidth","2");
    config->writeEntry("Color4","255,255,255");
  }
  if(highl_style==KMessageBox::No){
    config->setGroup("KWrite Options");
    config->writeEntry("WrapAt","78");
    config->writeEntry("ConfigFlags","1040");
    config->writeEntry("UndoSteps","50");
    config->writeEntry("Highlight","C++");
    config->writeEntry("SearchFlags","32");
    config->writeEntry("Color0","255,255,255");
    config->writeEntry("Color1","0,0,128");
    config->writeEntry("Color2","0,0,0");
    config->writeEntry("Color3","0,0,0");
    config->writeEntry("TabWidth","2");
    config->writeEntry("Color4","255,255,255");

    config->setGroup("C Highlight");
    config->writeEntry("Wildcards","*.c;*.C;*.ec");
    config->setGroup("C++ Highlight");
    config->writeEntry("Wildcards","*.cpp;*.cc;*.cxx;*.CPP;*.CC;*.CXX;*.h;*.hxx;*.H;*.HXX;*.ecpp");
  }
  config->sync();
  QProgressDialog progress( i18n("Checking needed programs..."), i18n("Cancel"), numProgs, this );
  progress.setMinimumDuration ( 0 );


  for (int i=0; i<numProgs; i++) {
  progress.setProgress( i );
  if ( progress.wasCancelled() )
    break;
    if(CToolClass::searchInstProgram("make")){

      make=true;
    }
  }
  progress.setProgress( numProgs );

  } // end of till_doc

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
  if(CToolClass::searchInstProgram("htdig")){
    htdig=true;
  }
  if(CToolClass::searchInstProgram("htsearch")){
    htsearch=true;
  }
  if(CToolClass::searchInstProgram("a2ps")){
    a2ps=true;
  }
  if(CToolClass::searchInstProgram("enscript")){
    enscript=true;
  }
  if(CToolClass::searchInstProgram("kdbg")){
    dbg=true;
  }
  if(CToolClass::searchInstProgram("kiconedit")){
    kiconedit=true;
  }
  if(CToolClass::searchInstProgram("ktranslator")){
    ktranslator=true;
  }
  if(CToolClass::searchInstProgram("kbabel")){
    kbabel=true;
  }
  if(CToolClass::searchInstProgram("gimp")){
    kpaint=true;
  }
  if(CToolClass::searchInstProgram("kpaint")){
    kpaint=true;
  }
  if(CToolClass::searchInstProgram("designer")){
    designer=true;
  }
  if(CToolClass::searchInstProgram("linguist")){
    linguist=true;
  }
	QStrList tools_exe;
	QStrList tools_entry;
	QStrList tools_argument;

  QString found=i18n(" was found.");
  QString not_found=i18n(" was not found.");

  QString make_str;
  if(make){
    make_str="make"+found+"\n";
    config->setGroup("General Options");
    config->writeEntry("Make","make");
  }
  else
    make_str="make"+not_found+"\n";
  QString gmake_str;
  if(gmake){
    gmake_str="gmake"+found+i18n(" -- setting make-command to gmake\n");
    config->setGroup("General Options");
    config->writeEntry("Make","gmake");
  }
  else
    gmake_str="gmake"+not_found+"\n";
  QString autoconf_str;
  if(autoconf)
    autoconf_str="autoconf"+found+"\n";
  else
    autoconf_str="autoconf"+not_found+"\n";
  QString autoheader_str;
  if(autoheader)
    autoheader_str="autoheader"+found+"\n";
  else
    autoheader_str="autoheader"+not_found+"\n";
  QString automake_str;
  if(automake)
    automake_str="automake"+found+"\n";
  else
    automake_str="automake"+not_found+"\n";
  QString sgml2html_str;
  if(sgml2html)
    sgml2html_str="sgml2html"+found+"\n";
  else
    sgml2html_str="sgml2html"+not_found+i18n(" -- generating application handbooks will not be possible\n");
  QString kdoc_str;
  if(kdoc)
    kdoc_str="kdoc"+found+"\n";
  else
    kdoc_str="kdoc"+not_found+i18n(" -- generating API-documentations will not be possible\n");
  QString dbg_str;
  if(dbg){
    dbg_str="kdbg"+found+"\n";
		tools_exe.append("kdbg");
		tools_entry.append("K&Dbg");
		tools_argument.append(" ");
	}
  QString kiconedit_str;
  if(kiconedit){
    kiconedit_str="KIconedit"+found+"\n";
		tools_exe.append("kiconedit");
		tools_entry.append("K&IconEdit");
		tools_argument.append(" ");
	}
  else
    kiconedit_str="KIconedit"+not_found+i18n(" -- editing icons will not be possible\n");
	
	QString kpaint_str;
	if(gimp){
    kpaint_str="GIMP"+found+"\n";
		tools_exe.append("gimp");
		tools_entry.append("&GIMP");
		tools_argument.append(" ");
	}
  else if(kpaint){
      kpaint_str="KPaint"+found+"\n";
  		tools_exe.append("kpaint");
  		tools_entry.append("K&Paint");
  		tools_argument.append(" ");
  }
  else
      kpaint_str=i18n("GIMP/KPaint ")+not_found+"\n";
	

	QString designer_str;
	if(designer){
    designer_str="Qt Designer"+found+"\n";
		tools_exe.append("designer");
		tools_entry.append("&Qt Designer");
		tools_argument.append(" -client ");
	}
  else
    designer_str=i18n("Qt Designer")+not_found+"\n";
	
	QString linguist_str;
	if(linguist){
    linguist_str="Qt Linguist"+found+"\n";
		tools_exe.append("linguist");
		tools_entry.append("&Qt linguist");
		tools_argument.append(" ");
	}
  else
    linguist_str=i18n("Qt Linguist")+not_found+"\n";

  QString ktranslator_str;
  if(ktranslator){
    ktranslator_str="KTranslator"+found+"\n";
		tools_exe.append("ktranslator");
		tools_entry.append("K&Translator");
		tools_argument.append(" ");
	}
  else
    ktranslator_str="KTranslator"+not_found+"\n";

  QString kbabel_str;
  if(kbabel){
    kbabel_str="KBabel"+found+"\n";
		tools_exe.append("kbabel");
		tools_entry.append("K&Babel");
		tools_argument.append(" ");
	}
  else
    kbabel_str="KBabel"+not_found+"\n";

  QString glimpse_str;
  if(glimpse)
    glimpse_str="Glimpse"+found+"\n";
  else
    glimpse_str="Glimpse"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  QString glimpseindex_str;
  if(glimpseindex)
    glimpseindex_str="Glimpseindex"+found+"\n";
  else
    glimpseindex_str="Glimpseindex"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  QString htdig_str;
  if(htdig)
    htdig_str="htdig"+found+"\n";
  else
    htdig_str="htdig"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  QString htsearch_str;
  if(htsearch)
    htsearch_str="htsearch"+found+"\n";
  else
    htsearch_str="htsearch"+not_found; //+ i18n(" -- search functionality will not be provided\n");
  if (!glimpse || !glimpseindex)
  {
    if (!htsearch)
    {
      htsearch_str += i18n(" -- search functionality will not be provided\n");
      if (!htdig)
	htdig_str += "\n";
    }
    else if (!htdig)
    {
      htdig_str += i18n(" -- search functionality will not be provided\n");
      if (!htsearch)
	htsearch_str += "\n";
    }
    if (!glimpse)
      glimpse_str += "\n";
    if (!glimpseindex)
      glimpseindex_str += "\n";
  }
  else
  {
    if (!htdig)
      htdig_str += "\n";
    if (!htsearch)
      htsearch_str += "\n";
  }
  QString perl_str;
  if(perl)
    perl_str="Perl"+found+"\n";
  else
    perl_str="Perl"+not_found+ i18n(" -- generation of new frame applications will not be possible\n");
  QString print_str;
  if(a2ps || enscript)
    print_str="a2ps / enscript"+found+ i18n("  -- printing available\n");
  else
    print_str="a2ps / enscript"+not_found+ i18n(" -- printing can only use lpr\n");

  if (!till_doc)
  {
  KMessageBox::information(this,i18n("The following results have been determined for your system:\n\n ")
                  +make_str+gmake_str+autoconf_str+autoheader_str+automake_str+perl_str+sgml2html_str+kdoc_str+glimpse_str+glimpseindex_str+htdig_str+htsearch_str
                  +print_str+dbg_str+kiconedit_str+kpaint_str+ktranslator_str+kbabel_str+linguist_str+designer_str,
                  i18n("Program test results"));

	config->setGroup("ToolsMenuEntries");
	config->writeEntry("Tools_exe",tools_exe);
	config->writeEntry("Tools_entry",tools_entry);
	config->writeEntry("Tools_argument",tools_argument);
	
  }  // end of till_doc

  int i;
  //  test also autoconfified documentation path
  config->setGroup("Doc_Location");
  QString qt=config->readEntry("doc_qt", QT_DOCDIR);
  qt_test=true;

  QString qt_testfile; // for tests if the path really is the qt-doc path
  const char *qt_dirs[]={
    "/usr/local/qt/html",
		"/usr/local/lib/qt/html",
		"/usr/lib/qt/html",
		"/usr/lib/qt/doc/html",
		"/usr/X11/lib/qt/html",
		"/usr/X11/lib/qt/doc/html",
		"/usr/doc/qt-doc/html",
		
    "/usr/local/qt2/html",
		"/usr/local/lib/qt2/html",
		"/usr/lib/qt2/html",
		"/usr/lib/qt2/doc/html",
		
    "/usr/local/qt-2.2.1/html",
		"/usr/local/lib/qt-2.2.1/html",
		"/usr/lib/qt-2.2.1/html",
		"/usr/lib/qt-2.2.1/doc/html",
		0l };

  // first check the autoconfified path
  if(qt_test && !qt.isEmpty())
  {
    qt_testfile=qt+"/classes.html";

    if(QFileInfo(qt_testfile).exists())
      qt_test=false;
  }

  for (i=0; qt_dirs[i]!=0l && qt_test; i++)
  {
    qt = qt_dirs[i];
    qt_testfile=qt+"/classes.html";

    if(QFileInfo(qt_testfile).exists())
      qt_test=false;
  };

  if(!qt_test){
	  config->writeEntry("doc_qt",qt);
	  KMessageBox::information(this,i18n("\nThe Qt-Documentation has been found at:\n\n")+qt
								     +i18n("\n\nThe correct path has been set.\n "), i18n("Qt Documentation found"));
	}
  else
  {  // return to the setup to set it manually ?
	  int result=KMessageBox::questionYesNo(this,i18n("\nThe Qt-library documentation could not\n"
								  "be detected. Please insert the correct path\n"
								  "to your Qt-documentation manually. Do you want\n"
								  "to set it now ?\n "),i18n("Information"));
	  if(result==KMessageBox::Yes)
	  {
	    hint_label->setGeometry( 40, 150, 440, 120 );
	    hint_label->setText(i18n("    Please choose your Qt-Documentation path by pushing the selection button above."));
            till_doc=true; // till qt-doc search all is done
	    return;
    }
	  else
	  {
	    slotProcessExited(0);
      return;
    }
  }
  
  //  test also autoconfified documentation path
  config->setGroup("Doc_Location");
  QString dir=config->readEntry("doc_kde", KDELIBS_DOCDIR);
  kde_test=true;

  QStringList kde_dirs;
  kde_dirs  << "/opt/kde/share/doc/HTML/en/kdelibs"     // normal dist
            << "/usr/share/doc/kdelibs"                 // Redhat 6.0
            << "/usr/local/kde/share/doc/kdelibs";      // other locations

  // first check the autoconfified path
  if(kde_test && !dir.isEmpty())
  {
    if(QFileInfo(dir+"/kdecore/index.html").exists())
      kde_test=false;
  }

  for ( QStringList::Iterator it = kde_dirs.begin(); it != kde_dirs.end(); ++it )
  {
    if(QFileInfo((*it)+"/kdecore/index.html").exists())
      kde_test=false;
  };

  if (!kde_test) {
    config->writeEntry("doc_kde",dir);
    KMessageBox::information(this,
                                i18n("\nThe KDE-Library-Documentation has been found at:\n\n")+dir
								     +i18n("\n\nThe correct path has been set.\n "),
				                i18n("KDE-Library Documentation found"));
     slotProcessExited(0);
     return; //ok, nothing more to do, we are leaving	
  }
  

  QDir* kde_dir=new QDir();

  if(!kdoc && !perl)
    KMessageBox::information(this,i18n("\nThe Program KDoc was not found on your system,\n"
                                                "a library documentation update can not be performed.\n"
                                                "KDoc is part of the kdesdk package that can be obtained\n"
                                                "from www.kde.org.\n\n "));
  else{
    KMessageBox::information(this, i18n("\nNow KDevelop will create a new KDE-library\n"
                                        "documentation. For that, you need the kdelibs\n"
                                        "package as the source package. In most cases it is\n"
                                        "included in your distribution.If you don't have\n"
                                        "the kdelibs as sources, we advise to obtain them\n"
                                        "from www.kde.org. Mind that the sources should match\n"
                                        "your installed kdelibs version.\n\n "));

    kde_dir->setCurrent(QDir::homeDirPath ());
    kde_dir->mkdir(".kde",false);
    kde_dir->mkdir(".kde/share",false);
    kde_dir->mkdir(".kde/share/apps",false);
    kde_dir->mkdir(".kde/share/apps/kdevelop",false);
    kde_dir->mkdir(".kde/share/apps/kdevelop/KDE-Documentation",false);
    config->setGroup("Doc_Location");
    config->writeEntry("doc_kde",QDir::homeDirPath ()+"/.kde/share/apps/kdevelop/KDE-Documentation/");
    config->sync();
    CUpdateKDEDocDlg dlg(shell_process, config,this,"test");
    if(!dlg.exec()){
      slotProcessExited(shell_process);
    }

    if (dlg.isUpdated())
        config->writeEntry("doc_kde",dlg.getDocPath());

    auto_button->setEnabled(false);
    hint_label->setText(i18n("                Creating KDE documentation                           "
			     "                                                             "
			     "               Please wait...                           "
			     "                                                               "
			     "                                                               ") );
    
  }
  
}
void CKDevInstall::slotCancel()
{
  int result=KMessageBox::questionYesNo(this,
            i18n("\n\nThis will exit the automatic installation process\n"
			        "and start KDevelop with the default values !\n\n"
					"If you choose 'Continue', you will have to set all\n"
					"installation values manually in the KDevelop Setup\n"
					"dialog available in the options menu.\n\n "),
            i18n("Warning"),i18n("Continue"),i18n("Back"));
  if(result==KMessageBox::Yes)
  {
    config->setGroup("General Options");
    config->writeEntry("Install",false);
	  config->sync();
    close();
  }
  else
    return;
}


