/***************************************************************************
                          wizardindexdocupage.cpp  -  description
                             -------------------
    begin                : Sun Jun 3 2001
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
#include <qlayout.h>
#include <qdir.h>

#include <klocale.h>
#include <kconfig.h>
#include <kprocess.h>

#include "ckdevinstall.h"
#include "ccreatedocdatabasedlg.h"

WizardIndexDocuPage::WizardIndexDocuPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  pInstallState->finish_dir = new QDir();

  QLabel* label;
  m_vbox = new QVBox(this);

  if (!m_pInstallState->finished_glimpse)
  {
    m_pInstallState->finished_glimpse=true;
    if((!m_pInstallState->glimpse || !m_pInstallState->glimpseindex) && (!m_pInstallState->htdig || !m_pInstallState->htsearch))
    {
		  label = new QLabel("", m_vbox);
      label = new QLabel(i18n("Neither glimpse nor htdig is installed, therefore KDevelop can not index your documentation to provide a proper help functionality.\n\n"
                              "We advise to install glimpse or htdig and create the searchdatabase later by choosing KDevelop Setup in the Options menu.\n\n"), m_vbox);
      label->setAlignment(WordBreak);
		  label = new QLabel("", m_vbox);

      m_pInstallState->install = false;
      if (m_pInstallState->finish_dir)
        m_pInstallState->finish_dir->setCurrent(QDir::homeDirPath ());
    }
    else
    {
		  setValid(false);	// disable the Next button, will be enabled again when indexing has finished
		
		  QWidget* wdg = new QWidget(m_vbox);
  		QVBoxLayout* vl = new QVBoxLayout(wdg);
      label = new QLabel(i18n("Now KDevelop will perform the last step of the installation process with indexing your documentation."
                              "This will provide an extended help functionality and will give you the information you need."), wdg);
      label->setAlignment(WordBreak);

      CCreateDocDatabaseDlg* dlg = new CCreateDocDatabaseDlg(wdg,"DLG", m_pInstallState->shell_process, m_pInstallState->kde, m_pInstallState->qt, m_pInstallState->glimpse | m_pInstallState->glimpseindex, m_pInstallState->htdig);
      vl->addWidget(label);
      vl->addWidget(dlg);

      QObject::connect(dlg, SIGNAL(indexingFinished(const QString&)), SLOT(indexingFinished(const QString&)) );
    }
  }
}

void WizardIndexDocuPage::indexingFinished(const QString& searchEngine)
{
  m_pInstallState->searchEngine = searchEngine;
  setValid(true);	// enable the Next button again
}
