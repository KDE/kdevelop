/***************************************************************************
                          welcomepage.cpp  -  description
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
#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpixmap.h>

#include <klocale.h>

#include "ckdevinstallstate.h"
#include "wizardwelcomepage.h"

WizardWelcomePage::WizardWelcomePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename)
{
   m_vbox = new QVBox(this);

   QLabel * label = new QLabel("", m_vbox);
   label = new QLabel(i18n("Welcome to the KDevelop Setup!"), m_vbox);
   label->setAlignment(AlignHCenter);
   label->setFont( QFont("Helvetica", 18, QFont::Bold));
   label->setFixedSize(label->sizeHint());
   label = new QLabel("", m_vbox);
   label = new QLabel(i18n("This installation program will:") + "\n" +
                      i18n("1.) let you adjust some settings for the editor and user interface.") + "\n" +
                      i18n("2.) check your system for helper programs needed by KDevelop.") + "\n" +
                      i18n("3.) set up your documentation, automatically.") + "\n\n" +
                      i18n("'Next' will invoke the automatic detection and the documentation generator.") + "\n" +
                      i18n("'Help' will give more information about the installation.") + "\n" +
                      i18n("'Cancel' will stop the installation and KDevelop will be started without setting the initial values needed for proper functionality."), m_vbox );
   label->setAlignment(WordBreak);
   label = new QLabel("", m_vbox);

   // allow to click on Next
   setValid(true);
}
