/***************************************************************************
                          wizardcreatekdedocpage.cpp  -  description
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

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kprocess.h>

#include "ckdevinstall.h"
#include "cupdatekdedocdlg.h"

WizardCreateKDEDocPage::WizardCreateKDEDocPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  bool kdoc = true;
  bool perl = true;
	
	QLabel* label;
	m_vbox = new QVBox(this);
	QWidget* wdg = new QWidget(m_vbox);
	QVBoxLayout* vl = new QVBoxLayout(wdg);
  if (!kdoc && !perl) {
    label = new QLabel(wdg);
    label = new QLabel(i18n("The Program KDoc was not found on your system, a library documentation update can not be performed.\n\n"
											 "KDoc is part of the kdesdk package that can be obtained from www.kde.org."), wdg);
    label->setAlignment(WordBreak);
    label = new QLabel(wdg);
  }
  else{
    label = new QLabel(i18n("Now KDevelop will create a new KDE-library documentation.\n\n"
    												"For that, you need the kdelibs package as the source package. "
    												"In most cases it is included in your distribution. "
    												"If you don't have the kdelibs as sources, we advise to obtain them from www.kde.org.\n\n"
    												"Mind that the sources should match your installed kdelibs version."), wdg);
    label->setAlignment(WordBreak);
    vl->addWidget(label);
    QString docDir = locateLocal("appdata", "KDE-Documentation");
    KStandardDirs::makeDir(docDir);

//    QDir* kde_dir=new QDir();
//    kde_dir->setCurrent(QDir::homeDirPath ());
//    kde_dir->mkdir(".kde",false);
//    kde_dir->mkdir(".kde/share",false);
//    kde_dir->mkdir(".kde/share/apps",false);
//    kde_dir->mkdir(".kde/share/apps/kdevelop",false);
//    kde_dir->mkdir(".kde/share/apps/kdevelop/KDE-Documentation",false);

		m_pInstallState->kde = docDir;
    CUpdateKDEDocDlg* dlg = new CUpdateKDEDocDlg(m_pInstallState->shell_process, docDir, m_pInstallState->qt, wdg, "test");
    vl->addWidget(dlg);
		QObject::connect(dlg, SIGNAL(newDocPathIsSetNow(const QString&)), SLOT(slotSetKDEDocPath(const QString&)) );
  }
}

void WizardCreateKDEDocPage::slotSetKDEDocPath(const QString& newKDEDocPath)
{
	m_pInstallState->kde = newKDEDocPath;
	setValid(true);	// enable the Next button again
}
