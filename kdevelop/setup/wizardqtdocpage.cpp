/***************************************************************************
                          wizardqtdocpage.cpp  -  description
                             -------------------
    begin                : Sat Jun 2 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qstring.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "ckdevinstallstate.h"
#include "wizardqtdocpage.h"

WizardQtDocPage::WizardQtDocPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
  ,qt_edit(0L)
{
  // nothing to be done here
  // we create this page when it will be chosen (because it's widget contents depends on the current state at that time)
}

void WizardQtDocPage::showEvent(QShowEvent*)
{
  int i;
  m_pInstallState->qt_test = true;

  // try to get the path from the QTDIR environment variable
  char* qt_dir_env = getenv( "QTDIR" );
  QCString qt_dir;
  if (qt_dir_env) {
    qt_dir = QCString(qt_dir_env);
  }
  qt_dir += "/doc/html";

  QString qt_testfile; // for tests if the path really is the qt-doc path
  const char *qt_dirs[]={
    qt_dir,
    "/usr/local/qt-2.2.3/html",
    "/usr/local/lib/qt-2.2.3/html",
    "/usr/lib/qt-2.2.3/html",
    "/usr/lib/qt-2.2.3/doc/html",

    "/usr/lib/qt2/html",
    "/usr/lib/qt2/doc/html",
    "/usr/local/qt2/html",
    "/usr/local/lib/qt2/html",
    "/usr/lib/qt/html",
    "/usr/lib/qt/doc/html",
    "/usr/local/qt/html",
    "/usr/local/qt/doc/html",
    "/usr/local/lib/qt/html",
    "/usr/local/lib/qt/doc/html",
    "/usr/X11/lib/qt/html",
    "/usr/X11/lib/qt/doc/html",
    "/usr/doc/qt-doc/html",

    0l };

  // first check the autoconfified path
  if(m_pInstallState->qt_test && !m_pInstallState->qt.isEmpty())
  {
    qt_testfile=m_pInstallState->qt+"/classes.html";

    if(QFileInfo(qt_testfile).exists())
      m_pInstallState->qt_test=false;
  }

  for (i=0; qt_dirs[i]!=0l && m_pInstallState->qt_test; i++)
  {
    m_pInstallState->qt = qt_dirs[i];
    qt_testfile=m_pInstallState->qt+"/classes.html";

    if(QFileInfo(qt_testfile).exists()) {
      m_pInstallState->qt_test=false;
    }
  };

  // create the widgets of this page
  m_vbox = new QVBox(this);
  m_vbox->show();
  QLabel* label = new QLabel(m_vbox);
  label->show();

  if(!m_pInstallState->qt_test){
    label = new QLabel(i18n("The Qt-Documentation has been found at:\n\n") + m_pInstallState->qt + "\n\n" +
                       i18n("The correct path has been set."), m_vbox);
    label->show();
    label->setAlignment(WordBreak);
    label = new QLabel(m_vbox);
    label->show();
    setPageTitle(i18n("Qt Documentation found"));
  }
  else
  {  // return to the setup to set it manually ?
    label = new QLabel(i18n("The Qt-library documentation could not be detected.\n\n"
                            "Please insert the correct path to your Qt-documentation manually."), m_vbox);
    label->show();
    label->setAlignment(WordBreak);
    label = new QLabel(m_vbox);
    label->show();

    QString qt_doc = m_pInstallState->qt;

    QWidget* w1 = new QWidget(m_vbox);
    QVBoxLayout* vl = new QVBoxLayout(w1, 15, 7);
    QLabel* qt_label = new QLabel( i18n("Qt-Documentation Path:"), w1 );

    QWidget* w2 = new QWidget(w1);
    vl->addWidget(qt_label);
    vl->addWidget(w2);

    QHBoxLayout* hl = new QHBoxLayout(w2, 15, 7);

    qt_edit = new QLineEdit( w2, "LineEdit" );
    qt_edit->setText( qt_doc );
    qt_edit->setEchoMode( QLineEdit::Normal );
    qt_edit->setMinimumWidth(200);

    QPushButton* qt_button = new QPushButton( w2, "PushButton" );
    QPixmap pix = SmallIcon("fileopen");
    qt_button->setPixmap(pix);
    connect( qt_button, SIGNAL(clicked()), SLOT(slotQTpressed()) );

    hl->addWidget(qt_edit);
    hl->addWidget(qt_button);

    label = new QLabel(m_vbox);

    w1->show();
    qt_label->show();
    w2->show();
    qt_edit->show();
    qt_button->show();
    label->show();
  }
}

void WizardQtDocPage::hideEvent(QHideEvent*)
{
  if (qt_edit)
    m_pInstallState->qt = qt_edit->text();
  delete m_vbox;
  qt_edit = 0;
}

void WizardQtDocPage::slotQTpressed()
{
  QString dir;
  dir = KFileDialog::getExistingDirectory(m_pInstallState->qt);
  if (!dir.isEmpty()){
    qt_edit->setText(dir);
    QString qt_testfile=dir+"/classes.html"; // test if the path really is the qt-doc path
    if(QFileInfo(qt_testfile).exists()){
      m_pInstallState->qt = dir;
    }
    else{
      KMessageBox::error(this,i18n("\nThe chosen path does not lead to the\n"
                                   "Qt-library documentation. Please choose the\n"
                                   "correct path."),
                                i18n("The Selected Path is not Correct!"));
    }
  }
}

#include "wizardqtdocpage.moc"
