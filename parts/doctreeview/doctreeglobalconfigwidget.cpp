/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doctreeglobalconfigwidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <kconfig.h>
#include <kprocess.h>
//#include <kdebug.h>
#include <iostream>
#include "../../config.h"
#include "domutil.h"
#include "doctreeviewpart.h"
#include "doctreeviewfactory.h"

DocTreeGlobalConfigWidget::DocTreeGlobalConfigWidget(DocTreeViewWidget *widget,
                                                     QWidget *parent, const char *name)
    : DocTreeGlobalConfigWidgetBase(parent, name)
{
    m_widget = widget;
    
    readConfig();
}


DocTreeGlobalConfigWidget::~DocTreeGlobalConfigWidget()
{}


void DocTreeGlobalConfigWidget::readConfig()
{
    KConfig *config = DocTreeViewFactory::instance()->config();

    config->setGroup("General");
    qtdocdirEdit->setText(config->readEntry("qtdocdir", QT_DOCDIR));

    kdelibsdoxydirEdit->setText(config->readEntry("kdelibsdocdir", KDELIBS_DOXYDIR));
    
    config->setGroup("Index");
    indexKDevelopBox->setChecked(config->readEntry("IndexKDevelop"));
    indexQtBox->setChecked(config->readEntry("IndexQt"));
    indexKdelibsBox->setChecked(config->readEntry("IndexKdelibs"));
    indexBooksBox->setChecked(config->readEntry("IndexBooks"));
    indexBookmarksBox->setChecked(config->readEntry("IndexBookmarks"));

    config->setGroup("htdig");
    htdigbinEdit->setText(config->readEntry("htdigbin"));
    htmergebinEdit->setText(config->readEntry("htmergebin"));
    htsearchbinEdit->setText(config->readEntry("htsearchbin"));
}


void DocTreeGlobalConfigWidget::storeConfig()
{
    KConfig *config = DocTreeViewFactory::instance()->config();

    config->setGroup("General");
    config->writeEntry("qtdocdir", qtdocdirEdit->text());
    config->writeEntry("kdelibsdocdir", kdelibsdoxydirEdit->text());

    config->setGroup("Index");
    config->writeEntry("IndexKDevelop", indexKDevelopBox->isChecked());
    config->writeEntry("IndexQt", indexQtBox->isChecked());
    config->writeEntry("IndexKdelibs", indexKdelibsBox->isChecked());
    config->writeEntry("IndexBooks", indexBooksBox->isChecked());
    config->writeEntry("IndexBookmarks", indexBookmarksBox->isChecked());

    config->setGroup("htdig");
    config->writeEntry("htdigbin", htdigbinEdit->text());
    config->writeEntry("htmergebin", htmergebinEdit->text());
    config->writeEntry("htsearchbin", htsearchbinEdit->text());
}


void DocTreeGlobalConfigWidget::updateIndexClicked()
{
    // I'm not sure if storing the configuration here is compliant
    // with user interface guides, but I see no easy way around
    storeConfig();
    
    DocTreeViewFactory::instance()->config()->sync();
    KProcess proc;
    proc << "kdevelop-htdig";
    proc.start(KProcess::DontCare);
}


void DocTreeGlobalConfigWidget::accept()
{
    storeConfig();
}

#include "doctreeglobalconfigwidget.moc"
