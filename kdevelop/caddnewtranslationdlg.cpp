/***************************************************************************
                          caddnewtranslationdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Apr 8 1999                                           
    copyright            : (C) 1999 by Sandy Meier                         
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


#include "caddnewtranslationdlg.h"
#include <kapp.h>


CAddNewTranslationDlg::CAddNewTranslationDlg(QWidget *parent, const char *name, CProject* p_prj) 
  : QDialog(parent,name,true) {
  

	
	ok_button = new QPushButton( this, "ok_button" );
	ok_button->setGeometry( 60, 100, 100, 30 );
	ok_button->setMinimumSize( 0, 0 );
	ok_button->setMaximumSize( 32767, 32767 );
	ok_button->setFocusPolicy( QWidget::TabFocus );
	ok_button->setBackgroundMode( QWidget::PaletteBackground );
	ok_button->setFontPropagation( QWidget::NoChildren );
	ok_button->setPalettePropagation( QWidget::NoChildren );
	ok_button->setText( "OK" );
	ok_button->setAutoRepeat( FALSE );
	ok_button->setAutoResize( FALSE );

	cancel_button = new QPushButton( this, "cancel_button" );
	cancel_button->setGeometry( 210, 100, 100, 30 );
	cancel_button->setMinimumSize( 0, 0 );
	cancel_button->setMaximumSize( 32767, 32767 );
	cancel_button->setFocusPolicy( QWidget::TabFocus );
	cancel_button->setBackgroundMode( QWidget::PaletteBackground );
	cancel_button->setFontPropagation( QWidget::NoChildren );
	cancel_button->setPalettePropagation( QWidget::NoChildren );
	cancel_button->setText( "Cancel" );
	cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );

	lang_combo = new KLanguageCombo(this, "lang_combo" );
	lang_combo->setGeometry( 110, 30, 240, 30 );
	// lang_combo->setMinimumSize( 0, 0 );
// 	lang_combo->setMaximumSize( 32767, 32767 );
// 	lang_combo->setFocusPolicy( QWidget::StrongFocus );
// 	lang_combo->setBackgroundMode( QWidget::PaletteBackground );
// 	lang_combo->setFontPropagation( QWidget::AllChildren );
// 	lang_combo->setPalettePropagation( QWidget::AllChildren );
// 	lang_combo->setSizeLimit( 10 );
// 	lang_combo->setAutoResize( FALSE );

	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( this, "Label_1" );
	qtarch_Label_1->setGeometry( 20, 30, 90, 30 );
	qtarch_Label_1->setMinimumSize( 0, 0 );
	qtarch_Label_1->setMaximumSize( 32767, 32767 );
	qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_1->setText( "Language:" );
	qtarch_Label_1->setAlignment( 289 );
	qtarch_Label_1->setMargin( -1 );

	resize( 370,150 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );

	setCaption(i18n("Add new Translation File..."));
	connect(ok_button,SIGNAL(clicked()),SLOT(OK()));
	connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));

	QStrList po_files;
	p_prj->getPOFiles("po/Makefile.am",po_files);

	if(po_files.contains("br.po") == 0){
	  lang_combo->insertLanguage("br;Breton");
	}
	if(po_files.contains("ca.po") == 0){
	  lang_combo->insertLanguage("ca;Catalan");
	}
	if(po_files.contains("cs.po") == 0){
	lang_combo->insertLanguage("cs;Czech");
	}
	if(po_files.contains("da.po") == 0){
	  lang_combo->insertLanguage("da;Danish");
	}
	if(po_files.contains("de.po") == 0){
	  lang_combo->insertLanguage("de;German");
	}
	if(po_files.contains("el.po") == 0){
	  lang_combo->insertLanguage("el;Greek");
	}
	if(po_files.contains("eo.po") == 0){
	  lang_combo->insertLanguage("eo;Esperanto");
	}
	if(po_files.contains("es.po") == 0){
	  lang_combo->insertLanguage("es;Spanish");
	}
	if(po_files.contains("fi.po") == 0){
	  lang_combo->insertLanguage("fi;Finnish");
	}
	if(po_files.contains("fr.po") == 0){
	  lang_combo->insertLanguage("fr;French");
	}
	if(po_files.contains("he.po") == 0){
	  lang_combo->insertLanguage("he;Hebrew");
	}
	if(po_files.contains("hr.po") == 0){
	  lang_combo->insertLanguage("hr;Croatian");
	}
	if(po_files.contains("hu.po") == 0){
	  lang_combo->insertLanguage("hu;Hungarian");
	}
	if(po_files.contains("is.po") == 0){
	  lang_combo->insertLanguage("is;Islandic");
	}
	if(po_files.contains("it.po") == 0){
	  lang_combo->insertLanguage("it;Italian");
	}
	if(po_files.contains("hr.po") == 0){
	  lang_combo->insertLanguage("hr;Croatian");
	}
	if(po_files.contains("ko.po") == 0){
	  lang_combo->insertLanguage("ko;Korean");
	}
	if(po_files.contains("mk.po") == 0){
	  lang_combo->insertLanguage("mk;Macedonian");
	}
	if(po_files.contains("nl.po") == 0){
	  lang_combo->insertLanguage("nl;Dutch");
	}
	if(po_files.contains("no.po") == 0){
	  lang_combo->insertLanguage("no;Norwegian");
	}
	if(po_files.contains("pl.po") == 0){
	  lang_combo->insertLanguage("pl;Polish");
	}
	if(po_files.contains("pt.po") == 0){
	  lang_combo->insertLanguage("pt;Portuguese");
	}
	if(po_files.contains("pt_BR.po") == 0){
	  lang_combo->insertLanguage("pt_BR;Brazil Portuguese");
	}
	if(po_files.contains("ro.po") == 0){
	  lang_combo->insertLanguage("ro;Romanian");
	}
	if(po_files.contains("ru.po") == 0){
	  lang_combo->insertLanguage("ru;Russian");
	}
	if(po_files.contains("sk.po") == 0){
	  lang_combo->insertLanguage("sk;Slovak");
	}
	if(po_files.contains("sv.po") == 0){
	  lang_combo->insertLanguage("sv;Swedish");
	}
	if(po_files.contains("tr.po") == 0){
	  lang_combo->insertLanguage("tr;Turkish");
	}
	if(po_files.contains("zh_CN.GB2312.po") == 0){
	  lang_combo->insertLanguage("zh_CN.GB2312;Simplified Chinese");
	}
	if(po_files.contains("zh_TW.Big5.po") == 0){
	  lang_combo->insertLanguage("zh_TW.Big5;Chinese");
	}
	if(po_files.contains("et.po") == 0){
	  lang_combo->insertLanguage("et;Estonian");
	}
}
CAddNewTranslationDlg::~CAddNewTranslationDlg(){
}

void CAddNewTranslationDlg::OK(){
  accept();
  
}
