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
#include <kdebug.h>
#include "../../config.h"
#include "doctreeviewpart.h"
#include "doctreeviewwidget.h"
#include "doctreeviewfactory.h"
#include "adddocitemdlg.h"
#include "misc.h"
#include "domutil.h"
#include "librarydocdlg.h"
#include <qfileinfo.h>
#include <kapp.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

DocTreeGlobalConfigWidget::DocTreeGlobalConfigWidget(DocTreeViewPart *part, DocTreeViewWidget *widget,
                                                     QWidget *parent, const char *name)
    : DocTreeGlobalConfigWidgetBase(parent, name)
{
    m_part = part;
    QDomDocument d;
    if(m_part->projectDom()) d = *m_part->projectDom();
    m_ignoreTocs = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoretocs", "toc");
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
    
    readTocConfigs();
}

void DocTreeGlobalConfigWidget::readTocConfigs()
{
    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit) 
    {
        const QString name( QFileInfo(*tit).baseName() );
        const QString location( DocTreeViewTool::tocLocation( *tit ) );
        if( m_ignoreTocs.contains( name ) )
            new KListViewItem( extListView, name, "false", location);
        else
            new KListViewItem( extListView, name, "true", location);
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

void DocTreeGlobalConfigWidget::extEdit()
{
    QListViewItem *item = extListView->currentItem();
    if( item )
    {
        const QString name( item->text(0) );
        const QString location( item->text(2) );
        KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
        QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
        QString filePath;
        for( QStringList::Iterator it = tocs.begin(); it!=tocs.end(); ++it)
        {
            if(QFileInfo(*it).baseName() == name)
                filePath = *it;
        }
        const QString _default( DocTreeViewTool::tocDocDefaultLocation( filePath ) );
        LibraryDocDlg *dlg = new LibraryDocDlg( this, name, location, _default);
        dlg->exec();
        
        delete dlg;
    }
    extListView->clear();
    readTocConfigs();
}

void DocTreeGlobalConfigWidget::extEnable()
{
    QListViewItem *item( extListView->selectedItem() );
    if( item && item->text(1) == "false" ) 
    {
        m_ignoreTocs.remove( item->text( 0 ) );
        DomUtil::writeListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc", m_ignoreTocs );
        item->setText(1, "true");
    }

}
void DocTreeGlobalConfigWidget::extDisable()
{
    //kdDebug(9002) << "disable" << endl;
    QListViewItem *item( extListView->selectedItem() );
    if( item && item->text(1) == "true" ) 
    {
        m_ignoreTocs << item->text( 0 );
        DomUtil::writeListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc", m_ignoreTocs );
        item->setText(1, "false");
    }
}


#include "doctreeglobalconfigwidget.moc"
