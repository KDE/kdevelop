/***************************************************************************
                          wizardlastpage.cpp  -  description
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

#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qdir.h>

#include <klocale.h>
#include <kconfig.h>

#include "ckdevinstall.h"

WizardLastPage::WizardLastPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  m_vbox = new QVBox(this);

  QLabel * label = new QLabel("", m_vbox);
  label = new QLabel(i18n("The installation process finished successfully.\n\n"
                           "The KDevelop Team wishes that you will enjoy our program and we would be honoured for any feedback.\n\n"
                           "The KDevelop Team"), m_vbox );
  label->setAlignment(WordBreak);
  label = new QLabel("", m_vbox);

	m_pInstallState->install = false;
	if (m_pInstallState->finish_dir)
		m_pInstallState->finish_dir->setCurrent(QDir::homeDirPath());
}

