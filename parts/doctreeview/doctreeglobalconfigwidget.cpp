/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doctreeglobalconfigwidget.h"

#include <qcheckbox.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kurlrequester.h>
//#include <kdebug.h>
#include "../../config.h"
#include "doctreeviewpart.h"
#include "doctreeviewwidget.h"
#include "doctreeviewfactory.h"
#include "kdevproject.h"
#include "adddocitemdlg.h"
#include "misc.h"
#include <qfileinfo.h>
#include <kapp.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

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
    qtdocdirEdit->setURL(config->readPathEntry("qtdocdir", QT_DOCDIR));
    qtdocdirEdit->fileDialog()->setMode( KFile::Directory );
    kdelibsdoxydirEdit->setURL(config->readPathEntry("kdelibsdocdir", KDELIBS_DOXYDIR));
    kdelibsdoxydirEdit->fileDialog()->setMode( KFile::Directory );
    kdocCheck->setChecked( config->readBoolEntry("displayKDELibsKDoc", false) );
    
    config->setGroup("Index");
    indexKDevelopBox->setChecked(config->readEntry("IndexKDevelop"));
    indexQtBox->setChecked(config->readEntry("IndexQt"));
    indexKdelibsBox->setChecked(config->readEntry("IndexKdelibs"));
    indexBooksBox->setChecked(config->readEntry("IndexBooks"));
    indexBookmarksBox->setChecked(config->readEntry("IndexBookmarks"));

    config->setGroup("htdig");
    QString exe = kapp->dirs()->findExe("htdig");
    htdigbinEdit->setURL(config->readEntry("htdigbin", exe));
    exe = kapp->dirs()->findExe("htmerge");
    htmergebinEdit->setURL(config->readEntry("htmergebin", exe));
    exe = kapp->dirs()->findExe("htsearch");
    htsearchbinEdit->setURL(config->readEntry("htsearchbin", exe));
    
    //get bookmarks
    QStringList bookmarksTitle, bookmarksURL;
    DocTreeViewTool::getBookmarks(&bookmarksTitle, &bookmarksURL);
    QStringList::Iterator oit1, oit2;
    for (oit1 = bookmarksTitle.begin(), oit2 = bookmarksURL.begin();
         oit1 != bookmarksTitle.end() && oit2 != bookmarksURL.end();
         ++oit1, ++oit2) {
        new KListViewItem( bListView, *oit1, *oit2);
    }
}


void DocTreeGlobalConfigWidget::storeConfig()
{
    KConfig *config = DocTreeViewFactory::instance()->config();

    config->setGroup("General");
    config->writePathEntry("qtdocdir", qtdocdirEdit->url());
    config->writePathEntry("kdelibsdocdir", kdelibsdoxydirEdit->url());
    config->writeEntry("displayKDELibsKDoc", kdocCheck->isChecked() );
 
    config->setGroup("Index");
    config->writeEntry("IndexKDevelop", indexKDevelopBox->isChecked());
    config->writeEntry("IndexQt", indexQtBox->isChecked());
    config->writeEntry("IndexKdelibs", indexKdelibsBox->isChecked());
    config->writeEntry("IndexBooks", indexBooksBox->isChecked());
    config->writeEntry("IndexBookmarks", indexBookmarksBox->isChecked());

    config->setGroup("htdig");
    config->writeEntry("htdigbin", htdigbinEdit->url());
    config->writeEntry("htmergebin", htmergebinEdit->url());
    config->writeEntry("htsearchbin", htsearchbinEdit->url());
    
    QStringList bookmarksTitle, bookmarksURL;
    {
        QListViewItem *item = bListView->firstChild();
        for (; item; item = item->nextSibling()) {
            bookmarksTitle.append(item->text(0));
            bookmarksURL.append(item->text(1));
        }
    }
    DocTreeViewTool::setBookmarks(bookmarksTitle, bookmarksURL);

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

void DocTreeGlobalConfigWidget::addBookmarkClicked()
{
    AddDocItemDialog dlg;
    if (!dlg.exec())
        return;

    (void) new KListViewItem(bListView, dlg.title(), dlg.url());

}

void DocTreeGlobalConfigWidget::removeBookmarkClicked()
{
    QListViewItem* item = bListView->currentItem();
    if (item) {
        bListView->removeItem(item);
    }
}

void DocTreeGlobalConfigWidget::accept()
{
    storeConfig();
    m_widget->configurationChanged();
}

#include "doctreeglobalconfigwidget.moc"
