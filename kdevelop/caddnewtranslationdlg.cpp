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


#include <kapp.h>
#include <kmsgbox.h>
#include <klocale.h>
#include "caddnewtranslationdlg.h"
#include "cproject.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/ 

CAddNewTranslationDlg::CAddNewTranslationDlg(QWidget *parent, const char *name, CProject* p_prj) 
  : QDialog(parent,name,true) {
  
  prj= p_prj;
  setCaption(i18n("Add new Translation File"));

	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( this, "Label_1" );
	qtarch_Label_1->setGeometry( 20, 30, 90, 30 );
	qtarch_Label_1->setMinimumSize( 0, 0 );
	qtarch_Label_1->setMaximumSize( 32767, 32767 );
	qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_1->setText( i18n("Language:") );
	qtarch_Label_1->setAlignment( 289 );
	qtarch_Label_1->setMargin( -1 );

	lang_combo = new KLanguageCombo(this, "lang_combo" );
	lang_combo->setGeometry( 110, 30, 240, 25 );

	ok_button = new QPushButton( this, "ok_button" );
	ok_button->setGeometry( 60, 100, 100, 25 );
	ok_button->setMinimumSize( 0, 0 );
	ok_button->setMaximumSize( 32767, 32767 );
	ok_button->setFocusPolicy( QWidget::TabFocus );
	ok_button->setBackgroundMode( QWidget::PaletteBackground );
	ok_button->setFontPropagation( QWidget::NoChildren );
	ok_button->setPalettePropagation( QWidget::NoChildren );
	ok_button->setText( i18n("OK") );
	ok_button->setAutoRepeat( FALSE );
	ok_button->setAutoResize( FALSE );
	ok_button->setDefault(true);
	
	cancel_button = new QPushButton( this, "cancel_button" );
	cancel_button->setGeometry( 210, 100, 100, 25 );
	cancel_button->setMinimumSize( 0, 0 );
	cancel_button->setMaximumSize( 32767, 32767 );
	cancel_button->setFocusPolicy( QWidget::TabFocus );
	cancel_button->setBackgroundMode( QWidget::PaletteBackground );
	cancel_button->setFontPropagation( QWidget::NoChildren );
	cancel_button->setPalettePropagation( QWidget::NoChildren );
	cancel_button->setText( i18n("Cancel") );
	cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );

	resize( 370,145 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );


	connect(ok_button,SIGNAL(clicked()),SLOT(slotOkClicked()));
	connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));

	QStrList po_files;
	lang_list.clear();
	if(!p_prj->isQt2Project()) // KDE 1/2 project
	  p_prj->getPOFiles("po/Makefile.am",po_files);
  else // QT 2 Project
	  p_prj->getTSFiles(p_prj->getBinPROGRAM()+"/Makefile.am",po_files);

	if((po_files.contains("br.po") || po_files.contains(prj->getBinPROGRAM()+".br.ts")) == 0){
	  lang_combo->insertLanguage(i18n("br;Breton"));
	  lang_list.append("br");
	}
	if((po_files.contains("ca.po") || po_files.contains(prj->getBinPROGRAM()+".ca.ts")) == 0){
	  lang_combo->insertLanguage(i18n("ca;Catalan"));
	  lang_list.append("ca");
	}
	 if((po_files.contains("cs.po") || po_files.contains(prj->getBinPROGRAM()+".cs.ts")) == 0){
	  lang_combo->insertLanguage(i18n("cs;Czech"));
	  lang_list.append("cs");
	}
	if((po_files.contains("da.po") || po_files.contains(prj->getBinPROGRAM()+".da.ts")) == 0){
	  lang_combo->insertLanguage(i18n("da;Danish"));
	  lang_list.append("da");
	}
	if((po_files.contains("de.po") || po_files.contains(prj->getBinPROGRAM()+".de.ts")) == 0){
	  lang_combo->insertLanguage(i18n("de;German"));
	  lang_list.append("de");
	}
	if((po_files.contains("el.po") || po_files.contains(prj->getBinPROGRAM()+".el.ts")) == 0){
	  lang_combo->insertLanguage(i18n("el;Greek"));
	  lang_list.append("el");
	}
	if((po_files.contains("eo.po") || po_files.contains(prj->getBinPROGRAM()+".eo.ts")) == 0){
	  lang_combo->insertLanguage(i18n("eo;Esperanto"));
	  lang_list.append("eo");
	}
	if((po_files.contains("es.po") || po_files.contains(prj->getBinPROGRAM()+".es.ts")) == 0){
	  lang_combo->insertLanguage(i18n("es;Spanish"));
	  lang_list.append("es");
	}
	if((po_files.contains("et.po") || po_files.contains(prj->getBinPROGRAM()+".et.ts")) == 0){
	  lang_combo->insertLanguage(i18n("et;Estonian"));
	  lang_list.append("et");
	}
	if((po_files.contains("fi.po") || po_files.contains(prj->getBinPROGRAM()+".fi.ts")) == 0){
	  lang_combo->insertLanguage(i18n("fi;Finnish"));
	  lang_list.append("fi");
	}
	if((po_files.contains("fr.po") || po_files.contains(prj->getBinPROGRAM()+".fr.ts")) == 0){
	  lang_combo->insertLanguage(i18n("fr;French"));
	  lang_list.append("fr");
	}
	if((po_files.contains("he.po") || po_files.contains(prj->getBinPROGRAM()+".he.ts")) == 0){
	  lang_combo->insertLanguage(i18n("he;Hebrew"));
	  lang_list.append("he");
	}
	if((po_files.contains("hr.po") || po_files.contains(prj->getBinPROGRAM()+".hr.ts")) == 0){
	  lang_combo->insertLanguage(i18n("hr;Croatian"));
	  lang_list.append("hr");
	}
	if((po_files.contains("hu.po") || po_files.contains(prj->getBinPROGRAM()+".hu.ts")) == 0){
	  lang_combo->insertLanguage(i18n("hu;Hungarian"));
	  lang_list.append("hu");
	}
	if((po_files.contains("is.po") || po_files.contains(prj->getBinPROGRAM()+".is.ts")) == 0){
	  lang_combo->insertLanguage(i18n("is;Islandic"));
	  lang_list.append("po");
	}
	if((po_files.contains("it.po") || po_files.contains(prj->getBinPROGRAM()+".it.ts")) == 0){
	  lang_combo->insertLanguage(i18n("it;Italian"));
	  lang_list.append("it");
	}
	if((po_files.contains("ja.po") || po_files.contains(prj->getBinPROGRAM()+".ja.ts")) == 0){
	  lang_combo->insertLanguage(i18n("ja;Japanese"));
	  lang_list.append("ja");
 	}
	if((po_files.contains("ko.po") || po_files.contains(prj->getBinPROGRAM()+".ko.ts")) == 0){
	  lang_combo->insertLanguage(i18n("ko;Korean"));
	  lang_list.append("ko");
	}
	if((po_files.contains("mk.po") || po_files.contains(prj->getBinPROGRAM()+".mk.ts")) == 0){
	  lang_combo->insertLanguage(i18n("mk;Macedonian"));
	  lang_list.append("mk");
	}
	if((po_files.contains("nl.po") || po_files.contains(prj->getBinPROGRAM()+".nl.ts")) == 0){
	  lang_combo->insertLanguage(i18n("nl;Dutch"));
	  lang_list.append("nl");
	}
	if((po_files.contains("no.po") || po_files.contains(prj->getBinPROGRAM()+".no.ts")) == 0){
	  lang_combo->insertLanguage(i18n("no;Norwegian"));
	  lang_list.append("no");
	}
	if((po_files.contains("pl.po") || po_files.contains(prj->getBinPROGRAM()+".pl.ts")) == 0){
	  lang_combo->insertLanguage(i18n("pl;Polish"));
	  lang_list.append("pl");
	}
	if((po_files.contains("pt.po") || po_files.contains(prj->getBinPROGRAM()+".pt.ts")) == 0){
	  lang_combo->insertLanguage(i18n("pt;Portuguese"));
	  lang_list.append("pt");
	}
	if((po_files.contains("pt_BR.po") || po_files.contains(prj->getBinPROGRAM()+".pt_BR.ts")) == 0){
	  lang_combo->insertLanguage(i18n("pt_BR;Brazil Portuguese"));
	  lang_list.append("pt_BR");
	}
	if((po_files.contains("ro.po") || po_files.contains(prj->getBinPROGRAM()+".ro.ts")) == 0){
	  lang_combo->insertLanguage(i18n("ro;Romanian"));
	  lang_list.append("ro");
	}
	if((po_files.contains("ru.po") || po_files.contains(prj->getBinPROGRAM()+".ru.ts")) == 0){
	  lang_combo->insertLanguage(i18n("ru;Russian"));
	  lang_list.append("ru");
	}
	if((po_files.contains("sk.po") || po_files.contains(prj->getBinPROGRAM()+".sk.ts")) == 0){
	  lang_combo->insertLanguage(i18n("sk;Slovak"));
	  lang_list.append("sk");
	}
	if((po_files.contains("sl.po") || po_files.contains(prj->getBinPROGRAM()+".sl.ts")) == 0){
	  lang_combo->insertLanguage(i18n("sl;Slovenian"));
	  lang_list.append("sl");
	}
	if((po_files.contains("sv.po") || po_files.contains(prj->getBinPROGRAM()+".sv.ts")) == 0){
	  lang_combo->insertLanguage(i18n("sv;Swedish"));
	  lang_list.append("sv");
	}
	if((po_files.contains("tr.po") || po_files.contains(prj->getBinPROGRAM()+".tr.ts")) == 0){
	  lang_combo->insertLanguage(i18n("tr;Turkish"));
	  lang_list.append("tr");
	}
	if((po_files.contains("zh_CN.GB2312.po") || po_files.contains(prj->getBinPROGRAM()+".zh_CN.GB2312.ts")) == 0){
	  lang_combo->insertLanguage(i18n("zh_CN.GB2312;Simplified Chinese"));
	  lang_list.append("zh_CN.GB2312");
	}
	if((po_files.contains("zh_TW.Big5.po") || po_files.contains(prj->getBinPROGRAM()+".zh_TW.Big5.ts")) == 0){
	  lang_combo->insertLanguage(i18n("zh_TW.Big5;Chinese"));
	  lang_list.append("zh_TW.Big5");
	}
	if(lang_list.isEmpty()){
	  KMsgBox::message(0,i18n("Information"),i18n("Your sourcecode is already translated to all supported languages."),KMsgBox::INFORMATION);
	  ok_button->setEnabled(false);
	}
}
CAddNewTranslationDlg::~CAddNewTranslationDlg(){
}
/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/ 

void CAddNewTranslationDlg::slotOkClicked(){
	
	if(!prj->isQt2Project()) // KDE 1/2 project
    langfile = lang_list.at(lang_combo->currentItem()) + QString(".po");
  else // QT 2 Project
    langfile = prj->getBinPROGRAM()+"."+lang_list.at(lang_combo->currentItem());

  accept();  
}
/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/ 
QString CAddNewTranslationDlg::getLangFile(){
  return langfile;
}

