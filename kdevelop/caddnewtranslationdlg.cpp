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

#include "cproject.h"

//#include <kapp.h>
#include <kmessagebox.h>
//#include <klangcombo.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kglobal.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgrid.h>
#include <kbuttonbox.h>


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

CAddNewTranslationDlg::CAddNewTranslationDlg(QWidget *parent, const char *name, CProject* p_prj)
  : QDialog(parent,name,true) {

  prj= p_prj;
  setCaption(i18n("Add new Translation File"));
  QGridLayout *grid1 = new QGridLayout(this,2,2,15,7);
  QLabel* qtarch_Label_1;
  qtarch_Label_1 = new QLabel( this, "Label_1" );
  qtarch_Label_1->setText( i18n("Language:") );
  grid1->addWidget(qtarch_Label_1,0,0);

  lang_combo = new QComboBox(this, "lang_combo" );
  grid1->addWidget(lang_combo,0,1);

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  ok_button = bb->addButton( i18n("OK") );
  ok_button->setDefault(true);

  cancel_button = bb->addButton( i18n( "Close" ) );

  bb->layout();
  grid1->addWidget( bb,1,1 );

  connect(ok_button,SIGNAL(clicked()),SLOT(slotOkClicked()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));

  QStrList po_files;
   if(!p_prj->isQt2Project()) // KDE 1/2 project
	  p_prj->getPOFiles("po/Makefile.am",po_files);
   else // QT 2 Project
	  p_prj->getTSFiles(p_prj->getBinPROGRAM()+"/Makefile.am",po_files);

  lang_list.clear();
  lang_list << "af" << "ar" << "bg" << "bo" << "br" << "bs" << "ca" << "cs" << "cy" << "da"
            << "de" << "el" << "en_GB"   << "eo" << "es" << "et" << "eu" << "fi" << "fr"
            << "ga" << "gl" << "gu" << "he" << "hi" << "hu" << "id" << "is" << "it" << "ja";
  lang_list << "km" << "ko" << "lt" << "lv" << "mi" << "mk" << "mr" << "nl" << "no" << "no_NY"
            << "oc" << "pl" << "pt" << "pt_BR" << "ro" << "ru" << "sk" << "sl" << "sr" << "sv"
            << "ta" << "th" << "tr" << "uk" << "wa" << "zh_CN.GB2312" << "zh_TW.Big5";

  QString curlang;

  for ( QStringList::ConstIterator it = lang_list.begin();
    it != lang_list.end(); ++it )
    {
      bool bStillMissing=true;
      if(po_files.isEmpty())
        lang_combo->insertItem((*it));
      else
      {
        for (po_files.first(); bStillMissing && po_files.current()!=0; po_files.next())
        {
          // add only if the po-file wasn't found
          if(po_files.contains(((*it)+QString(".po")).local8Bit()) ||
              po_files.contains((prj->getBinPROGRAM()+QString(".")+(*it)+QString(".ts")).local8Bit()))
           bStillMissing=false;
        }
        if (bStillMissing)
          lang_combo->insertItem((*it));
      }
    }

    if(lang_combo->count()==0)
    {
      KMessageBox::information(0,i18n("Your sourcecode is already translated to all supported languages."));
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
    langfile = lang_combo->currentText() + QString(".po");
  else // QT 2 Project
    langfile = prj->getBinPROGRAM()+"."+lang_combo->currentText();

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

#include "caddnewtranslationdlg.moc"
