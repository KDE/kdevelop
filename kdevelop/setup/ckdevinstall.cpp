/***************************************************************************
                          ckdevinstall.h  -  description
                             -------------------
    begin                : Thu Mar 4 1999
    copyright            : (C) 1999 by Ralf Nolden
    rewrite              : (C) Jun 2001 by Falk Brettschneider
    email                : Ralf.Nolden@post.rwth-aachen.de
                         : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlined.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qstringlist.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kprocess.h>
#include <ksimpleconfig.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../ctoolclass.h"
#include "wizardwelcomepage.h"
#include "wizardchoosehlpage.h"
#include "wizardcreatekdedocpage.h"
#include "wizardindexdocupage.h"
#include "wizardkdedocpage.h"
#include "wizardlastpage.h"
#include "wizardqtdocpage.h"
#include "wizardtoolscheckpage.h"
#include "wizarduimodepage.h"

#include "ckdevinstall.h"
#include "ckdevinstallstate.h"

CKDevInstall::CKDevInstall(QWidget *parent, const char *name, KConfig* config)
: QWizard(parent,name)
  ,m_config(config)
  ,m_pInstallState(0L)
{
  //  test also autoconfified documentation path
  m_config->setGroup("Doc_Location");
  QString qtDocuPath = m_config->readEntry("doc_qt", QT_DOCDIR);
  QString kdeDocuPath = m_config->readEntry("doc_kde", KDELIBS_DOCDIR);
  m_pInstallState = new CKDevInstallState( qtDocuPath, kdeDocuPath);

  setCaption(i18n("KDevelop Setup"));

  WizardWelcomePage* pWizWelcomePage            = new WizardWelcomePage(this,"welcome_page", i18n("Let's start!"), "pics/test.png");
  WizardSyntaxHlPage* pWizSyntaxHlPage          = new WizardSyntaxHlPage(this, "syntaxhl_page", i18n("Choose the syntax highlighting style for the editor!"), "pics/test.png", m_pInstallState);
  WizardUIModePage* pWizUIModePage              = new WizardUIModePage(this,"uimode_page", i18n("What is your favourite user interface mode?"), "pics/test.png", m_pInstallState);
  WizardToolsCheckPage* pWizToolsCheckPage      = new WizardToolsCheckPage(this, "toolscheck_page", i18n("This is the result of a check for tools used by KDevelop:"), "pics/test.png", m_pInstallState);
  WizardQtDocPage* pWizQtDocPage                = new WizardQtDocPage(this, "qtdoc_page", i18n("Automatic detection of the Qt documentation"), "pics/test.png", m_pInstallState);
  WizardKDEDocPage* pWizKDEDocPage              = new WizardKDEDocPage(this, "kdedoc_page", i18n("Automatic detection of the KDE documentation"), "pics/test.png", m_pInstallState);
  WizardCreateKDEDocPage* pWizCreateKDEDocPage  = new WizardCreateKDEDocPage(this, "createkdedoc_page", i18n("Choose Create to start the parser or skip the documentation by choosing Next!"), "pics/test.png", m_pInstallState);
  WizardIndexDocuPage* pWizIndexDocuPage        = new WizardIndexDocuPage(this, "indexdocu_page", i18n("Enhance your documentation by creating an index database!"), "pics/test.png", m_pInstallState);
  WizardLastPage* pWizLastPage                  = new WizardLastPage(this,"last_page", i18n("The setup has finished."), "pics/test.png", m_pInstallState);

  addPage(pWizWelcomePage, "Step 1 of 9");
  addPage(pWizSyntaxHlPage, "Step 2 of 9");
  addPage(pWizUIModePage, "Step 3 of 9");
  addPage(pWizToolsCheckPage, "Step 4 of 9");
  addPage(pWizQtDocPage, "Step 5 of 9");
  addPage(pWizKDEDocPage, "Step 6 of 9");
  addPage(pWizCreateKDEDocPage, "Step 7 of 9");
  addPage(pWizIndexDocuPage, "Step 8 of 9");
  addPage(pWizLastPage, "Step 9 of 9");

  setFinishEnabled(pWizLastPage, true);

  connect( pWizKDEDocPage, SIGNAL(enableCreateKDEDocPage(bool)), SLOT(slotEnableCreateKDEDocPage(bool)) );
  connect( helpButton(), SIGNAL(clicked()), SLOT(slotHelp()) );
  connect( cancelButton(), SIGNAL(clicked()), SLOT(slotCancel()) );
  connect( finishButton(), SIGNAL(clicked()), SLOT(slotFinished()) );
  connect( pWizCreateKDEDocPage, SIGNAL(validData(QWidget*,bool)), SLOT(setNextEnabled(QWidget*,bool)) );
  connect( pWizCreateKDEDocPage, SIGNAL(enableBackButton(QWidget*,bool)), SLOT(setBackEnabled(QWidget*,bool)) );
  connect( pWizIndexDocuPage, SIGNAL(validData(QWidget*,bool)), SLOT(setNextEnabled(QWidget*,bool)) );
  connect( pWizIndexDocuPage, SIGNAL(enableBackButton(QWidget*,bool)), SLOT(setBackEnabled(QWidget*,bool)) );

	resize( 520, 420 );
}

CKDevInstall::~CKDevInstall(){
}

void CKDevInstall::slotHelp()
{
  kapp->invokeHTMLHelp("kdevelop/index-2.html", "ss2.3" );
}

void CKDevInstall::slotCancel()
{
  qApp->quit();
}

void CKDevInstall::slotFinished()
{
  // 1.) write the results of the syntax highlight mode page
  if (m_pInstallState->highlightStyle == 0) {  // new KDevelop-2.0 style
    m_config->setGroup("KWrite Options");
    m_config->writeEntry("WrapAt","78");
    m_config->writeEntry("ConfigFlags","492569");
    m_config->writeEntry("UndoSteps","5000");
    m_config->writeEntry("Highlight","C++");
    m_config->writeEntry("SearchFlags","32");
    m_config->writeEntry("Color0","255,255,255");
    m_config->writeEntry("Color1","0,0,128");
    m_config->writeEntry("Color2","0,0,0");
    m_config->writeEntry("Color3","0,0,0");
    m_config->writeEntry("TabWidth","2");
    m_config->writeEntry("IndentLength","2");
    m_config->writeEntry("Color4","255,255,255");

    m_config->setGroup("C Highlight");
    m_config->writeEntry("Char","0,FFDE1907,FFFF1E09,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Comment","0,FF067817,FF1FD718,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Data Type","0,FFAD466A,FFE65D8D,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Decimal","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Float","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Hex","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Keyword","0,FF0E23AD,FF1534FF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Mimetypes","text/x-c-src");
    m_config->writeEntry("Normal Text","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Octal","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Prep. Lib","0,FF808000,FF00FFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Preprocessor","0,FF800080,FFFF00FF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("String","0,FFDE1907,FFFF1E09,0,0,1,adobe-courier,12,");
    m_config->writeEntry("String Char","0,FFDE1907,FFFF1E09,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Symbol","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Wildcards","*.c;*.C;*.ec;*.xpm;*.po");

    m_config->setGroup("C++ Highlight");
    m_config->writeEntry("Char","0,FFDE1907,FFFF1E09,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Comment","0,FF067817,FF1FD718,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Data Type","0,FFAD466A,FFE65D8D,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Decimal","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Float","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Hex","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Keyword","0,FF0E23AD,FF1534FF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Mimetypes","text/x-c++-src;text/x-c++-hdr;text/x-c-hdr");
    m_config->writeEntry("Normal Text","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Octal","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Prep. Lib","0,FF808000,FF00FFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Preprocessor","0,FF800080,FFFF00FF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("String","0,FFDE1907,FFFF1E09,0,0,1,adobe-courier,12,");
    m_config->writeEntry("String Char","0,FFDE1907,FFFF1E09,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Symbol","1,FF000000,FFFFFFFF,0,0,1,adobe-courier,12,");
    m_config->writeEntry("Wildcards","*.cpp;*.cc;*.cxx;*.CPP;*.CC;*.CXX;*.h;*.hxx;*.H;*.HXX;*.ecpp;*.moc;*.diff;*.inl;*.tlh");
  }
  else if (m_pInstallState->highlightStyle == 1) {
    m_config->setGroup("Perl Highlight");
    m_config->writeEntry("Mimetypes","application/x-perl");
    m_config->writeEntry("Wildcards","*.pl;*.pm");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Variable","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Operator","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("C++ Highlight");
    m_config->writeEntry("Prep. Lib","0,808000,FFFF,0,0,1,courier,12,");
    m_config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Char","0,CC07,CC07,0,0,1,courier,12,");
    m_config->writeEntry("Data Type","0,AD466A,AD466A,0,0,1,courier,12,");
    m_config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/x-c++-src;text/x-c++-hdr;text/x-c-hdr");
    m_config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Preprocessor","0,800080,800080,0,0,1,courier,12,");
    m_config->writeEntry("Wildcards","*.cpp;*.cc;*.cxx;*.CPP;*.CC;*.CXX;*.h;*.hxx;*.H;*.HXX;*.ecpp;*.moc;*.diff;*.inl;*.tlh");
    m_config->writeEntry("Comment","0,E0,D7,0,0,1,courier,12,");
    m_config->writeEntry("String","0,8000,8000,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","0,A7AD,A7AD,0,0,1,courier,12,");
    m_config->writeEntry("String Char","0,8000,8000,0,0,1,courier,12,");

    m_config->setGroup("Ada Highlight");
    m_config->writeEntry("Base-N","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/x-ada-src");
    m_config->writeEntry("Wildcards","*.a");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("Modula 2 Highlight");
    m_config->writeEntry("Mimetypes","text/x-modula-2-src");
    m_config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Wildcards","*.md;*.mi");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("IDL Highlight");
    m_config->writeEntry("Prep. Lib","0,808000,FFFF,0,0,1,courier,12,");
    m_config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Char","0,CC07,CC07,0,0,1,courier,12,");
    m_config->writeEntry("Data Type","0,AD466A,AD466A,0,0,1,courier,12,");
    m_config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/x-idl-src");
    m_config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Preprocessor","0,800080,800080,0,0,1,courier,12,");
    m_config->writeEntry("Wildcards","*.idl");
    m_config->writeEntry("Comment","0,E0,D7,0,0,1,courier,12,");
    m_config->writeEntry("String","0,8000,8000,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","0,A7AD,A7AD,0,0,1,courier,12,");
    m_config->writeEntry("String Char","0,8000,8000,0,0,1,courier,12,");


    m_config->setGroup("Java Highlight");
    m_config->writeEntry("Prep. Lib","0,8080,FFFF,0,0,1,courier,12,");
    m_config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Data Type","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/x-java-src");
    m_config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Preprocessor","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Wildcards","*.java");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("Normal Highlight");
    m_config->writeEntry("Mimetypes","");
    m_config->writeEntry("Wildcards","");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("C Highlight");
    m_config->writeEntry("Prep. Lib","0,8080,FFFF,0,0,1,courier,12,");
    m_config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Data Type","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/x-c-src");
    m_config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Preprocessor","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Wildcards","*.c;*.C;*.ec;*.xpm;*.po");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("HTML Highlight");
    m_config->writeEntry("Tag Text","0,0,FFFFFF,1,0,1,courier,12,");
    m_config->writeEntry("Char","0,8000,FF00,0,0,1,courier,12,");
    m_config->writeEntry("Tag","0,800080,FF00FF,1,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/html");
    m_config->writeEntry("Wildcards","*.html;*.htm");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Tag Value","0,808000,FFFF00,0,0,1,courier,12,");

    m_config->setGroup("Python Highlight");
    m_config->writeEntry("Octal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Char","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Decimal","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/x-python-src");
    m_config->writeEntry("Hex","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Wildcards","*.py");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Float","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String Char","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("Bash Highlight");
    m_config->writeEntry("Integer","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Mimetypes","text/x-shellscript;application/x-shellscript");
    m_config->writeEntry("Substitution","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Wildcards","*.sh");
    m_config->writeEntry("Comment","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("String","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
    m_config->writeEntry("Keyword","1,0,FFFFFF,0,0,1,courier,12,");

    m_config->setGroup("KWrite Options");
    m_config->writeEntry("WrapAt","78");
    m_config->writeEntry("ConfigFlags","1073");
    m_config->writeEntry("UndoSteps","5000");
    m_config->writeEntry("Highlight","C++");
    m_config->writeEntry("SearchFlags","32");
    m_config->writeEntry("Color0","255,255,255");
    m_config->writeEntry("Color1","0,0,128");
    m_config->writeEntry("Color2","0,0,0");
    m_config->writeEntry("Color3","0,0,0");
    m_config->writeEntry("TabWidth","2");
    m_config->writeEntry("Color4","255,255,255");
  }
  else if (m_pInstallState->highlightStyle == 2) {
    m_config->setGroup("KWrite Options");
    m_config->writeEntry("WrapAt","78");
    m_config->writeEntry("ConfigFlags","1040");
    m_config->writeEntry("UndoSteps","5000");
    m_config->writeEntry("Highlight","C++");
    m_config->writeEntry("SearchFlags","32");
    m_config->writeEntry("Color0","255,255,255");
    m_config->writeEntry("Color1","0,0,128");
    m_config->writeEntry("Color2","0,0,0");
    m_config->writeEntry("Color3","0,0,0");
    m_config->writeEntry("TabWidth","2");
    m_config->writeEntry("Color4","255,255,255");

    m_config->setGroup("C Highlight");
    m_config->writeEntry("Wildcards","*.c;*.C;*.ec;*.xpm;*.po");
    m_config->setGroup("C++ Highlight");
    m_config->writeEntry("Wildcards","*.cpp;*.cc;*.cxx;*.CPP;*.CC;*.CXX;*.h;*.hxx;*.H;*.HXX;*.ecpp;*.moc;*.diff;*.inl;*.tlh");

    m_config->setGroup("Normal Highlight");
    m_config->writeEntry("Mimetypes","");
    m_config->writeEntry("Wildcards","");
    m_config->writeEntry("Normal Text","1,0,FFFFFF,0,0,1,courier,12,");
  }

  // 2.) write the results of the tools-check page
  if(m_pInstallState->make){
    m_config->setGroup("General Options");
    m_config->writeEntry("Make","make");
  }

  if(m_pInstallState->gmake){
    m_config->setGroup("General Options");
    m_config->writeEntry("Make","gmake");
  }

  // 3.) write the results of the Qt-docu page
  m_config->setGroup("Doc_Location");
  m_config->writeEntry("doc_qt",m_pInstallState->qt);

  // 4.) write the results of the KDE-docu page
  m_config->setGroup("Doc_Location");
  m_config->writeEntry("doc_kde",m_pInstallState->kde);

  // 5.) write the results of the KDE-docu-indexing page
  m_config->setGroup("General Options");
  m_config->writeEntry("Install",m_pInstallState->install);

  // 6.) write the results of the doc database page
  m_config->setGroup("Doc_Location");
  m_config->writeEntry("searchengine",m_pInstallState->searchEngine);

  // 7.) write the results of the UI mode page
  m_config->setGroup("General Options");
  m_config->writeEntry("MDI mode", m_pInstallState->userInterfaceMode);

  m_config->sync();

  // get rid of possible old docking states
  KSimpleConfig conf(locate("config", "kdeveloprc"));
  conf.deleteGroup(QString("dock_setting_default"), true);
  conf.deleteGroup(QString("docking_version_2_0"), true);
  conf.sync();

  kapp->startServiceByDesktopName("kdevelop");
}

void CKDevInstall::slotEnableCreateKDEDocPage(bool bEnabled)
{
  setAppropriate( page(6), bEnabled);
}

#include "ckdevinstall.moc"
