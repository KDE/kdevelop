/***************************************************************************
                          wizardkdedocpage.cpp  -  description
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

#include <qlabel.h>
#include <qvbox.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "ckdevinstallstate.h"
#include "wizardkdedocpage.h"
#include <config.h>

WizardKDEDocPage::WizardKDEDocPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  // nothing to be done here
  // we create this page when it will be chosen (because it's widget contents depends on the current state at that time)
}

void WizardKDEDocPage::showEvent(QShowEvent*)
{
  bool kde_docs_found=false;

  QStringList kde_dirs;
  kde_dirs  << "/opt/kde2/share/doc/HTML/en/kdelibs"     // normal dist
//!? conflict with KDE-1.x            << "/opt/kde/share/doc/HTML/en/kdelibs"     // normal dist
            << "/usr/share/doc/kdelibs"                 // Redhat 6.0
            << "/usr/local/kde/share/doc/kdelibs"       // other locations
            << KDELIBS_DOCDIR;    // autoconf said

  // first check the autoconfified path
  if(!m_pInstallState->kde.isEmpty())
  {
    if(QFileInfo(m_pInstallState->kde+"/kdecore/index.html").exists())
      kde_docs_found=true;
  }

  for ( QStringList::Iterator it = kde_dirs.begin();
	!kde_docs_found && it != kde_dirs.end(); ++it )
  {
    if(QFileInfo((*it)+"/kdecore/index.html").exists())
    {
      m_pInstallState->kde = (*it);
      kde_docs_found=true;
    }
  };

  m_vbox = new QVBox(this);
  m_vbox->show();
  QLabel* label;
  if (kde_docs_found) {
    label = new QLabel(m_vbox);
    label->show();
    label = new QLabel(i18n("The KDE-Library-Documentation has been found at:\n\n") + m_pInstallState->kde + "\n\n" +
                       i18n("The correct path has been set."), m_vbox);
    label->show();
    label->setAlignment(WordBreak);
    label = new QLabel(m_vbox);
    label->show();
    setPageTitle(i18n("KDE-Library Documentation found"));

    emit enableCreateKDEDocPage(false);
  }
  else {
    label = new QLabel(m_vbox);
    label->show();
    label = new QLabel(i18n("The documentation of the KDE-library could not be found.\n\n"
                            "It will be automatically regenerated in the next step."), m_vbox);
    label->show();
    label->setAlignment(WordBreak);
    label = new QLabel(m_vbox);
    label->show();
    
    emit enableCreateKDEDocPage(true);
  }
}

void WizardKDEDocPage::hideEvent(QHideEvent*)
{
	delete m_vbox;
}

void WizardKDEDocPage::slotKDEpressed()
{
  QString dir;
  dir = KFileDialog::getExistingDirectory(m_pInstallState->kde);
  if (!dir.isEmpty()){
    QLineEdit* kde_edit = new QLineEdit(this);
    kde_edit->setText(dir);

    QString kde_testfile=dir+"/kdecore/index.html"; // test if the path really is the kde-doc path
    if(QFileInfo(kde_testfile).exists()){
      m_pInstallState->kde = dir;
//      kde_test=false;
    }
    else{
      KMessageBox::error(this,i18n("\nThe chosen path does not lead to the\n"
                                   "KDE-library documentation. Please choose 'Proceed'\n"
                                   "in any case. If you don't have a documentation,\n"
                                   "it will be generated automatically in one of the\n"
                                   "next steps"),i18n("The Selected Path is not Correct!"));
    }
  }
}

#include "wizardkdedocpage.moc"
