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
    if (!m_part->project())
    {
//        extEditButton->setEnabled(false);
        extEnableButton->setEnabled(false);
        extDisableButton->setEnabled(false);
    }
    
    qtdocs_view->addColumn(i18n("Title"));
    qtdocs_view->addColumn(i18n("URL"));
    qtdocs_view->setAllColumnsShowFocus(true);
    
    doxygen_view->addColumn(i18n("Title"));
    doxygen_view->addColumn(i18n("URL"));
    doxygen_view->setAllColumnsShowFocus(true);

    kdoc_view->addColumn(i18n("Title"));
    kdoc_view->addColumn(i18n("URL"));
    kdoc_view->setAllColumnsShowFocus(true);
        
    bListView->addColumn(i18n("Title"));
    bListView->addColumn(i18n("URL"));
    bListView->setAllColumnsShowFocus(true);
    
    extListView->addColumn(i18n("Name"));
    extListView->addColumn(i18n("Enabled"));
    extListView->addColumn(i18n("Location"));
    extListView->setAllColumnsShowFocus(true);
    
}


DocTreeGlobalConfigWidget::~DocTreeGlobalConfigWidget()
{}


void DocTreeGlobalConfigWidget::readConfig()
{
    KConfig *config = DocTreeViewFactory::instance()->config();
    
    //qt *.xml documentation files
    config->setGroup("General Qt");   
    QMap<QString, QString> emap = config->entryMap("General Qt");
    QMap<QString, QString>::Iterator it;
    for (it = emap.begin(); it != emap.end(); ++it)
    {
        KListViewItem *qtitem = new KListViewItem(qtdocs_view, it.key(), config->readPathEntry(it.key()));
    }
    if (emap.empty())
    {
        KListViewItem *qtitem = new KListViewItem(qtdocs_view, "Qt Reference Documentation", QString(QT_DOCDIR) + QString("/qt.xml"));
        qtitem = new KListViewItem(qtdocs_view, "Qt Assistant Manual", QString(QT_DOCDIR) + QString("/assistant.xml"));
        qtitem = new KListViewItem(qtdocs_view, "Qt Designer Manual", QString(QT_DOCDIR) + QString("/designer.xml"));
        qtitem = new KListViewItem(qtdocs_view, "Guide to the Qt Translation Tools", QString(QT_DOCDIR) + QString("/linguist.xml"));
        qtitem = new KListViewItem(qtdocs_view, "qmake User Guide", QString(QT_DOCDIR) + QString("/qmake.xml"));
    }

    config->setGroup("General Doxygen");
    QMap<QString, QString> xmap = config->entryMap("General Doxygen");
    QMap<QString, QString>::Iterator itx;
    for (itx = xmap.begin(); itx != xmap.end(); ++itx)
    {
        KListViewItem *qtitem = new KListViewItem(doxygen_view, itx.key(), config->readPathEntry(itx.key()));
    }
    if (xmap.empty() && (!QString(KDELIBS_DOXYDIR).isEmpty()))
    {
        KListViewItem *qtitem = new KListViewItem(doxygen_view, "KDE Libraries (Doxygen)", KDELIBS_DOXYDIR);
    }

    
    config->setGroup("General KDoc");
    QMap<QString, QString> dmap = config->entryMap("General KDoc");
    QMap<QString, QString>::Iterator itd;
    for (itd = dmap.begin(); itd != dmap.end(); ++itd)
    {
        KListViewItem *qtitem = new KListViewItem(kdoc_view, itd.key(), config->readPathEntry(itd.key()));
    }
    if (dmap.empty() && (!QString(KDELIBS_DOCDIR).isEmpty()))
    {
        KListViewItem *qtitem = new KListViewItem(kdoc_view, "KDE Libraries (KDoc)", KDELIBS_DOCDIR);
    }
        
/*    qtdocdirEdit->setURL(config->readPathEntry("qtdocdir", QT_DOCDIR));
    qtdocdirEdit->fileDialog()->setMode( KFile::Directory );
    kdelibsdoxydirEdit->setURL(config->readPathEntry("kdelibsdocdir", KDELIBS_DOXYDIR));
    kdelibsdoxydirEdit->fileDialog()->setMode( KFile::Directory );
*/
/*    config->setGroup("General");
    kdocCheck->setChecked( config->readBoolEntry("displayKDELibsKDoc", false) );
  */  
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

    config->deleteGroup("General Qt");
    config->setGroup("General Qt");   
    QListViewItemIterator it( qtdocs_view );
    while ( it.current() )
    {
        config->writePathEntry(it.current()->text(0), it.current()->text(1));
        ++it;
    }

    config->deleteGroup("General Doxygen");
    config->setGroup("General Doxygen");   
    QListViewItemIterator itx( doxygen_view );
    while ( itx.current() ) 
    {
        config->writePathEntry(itx.current()->text(0), itx.current()->text(1));
        ++itx;
    }

    config->deleteGroup("General KDoc");
    config->setGroup("General KDoc");   
    QListViewItemIterator itd( kdoc_view );
    while ( itd.current() ) 
    {
        config->writePathEntry(itd.current()->text(0), itd.current()->text(1));
        ++itd;
    }
    
      
/*    config->setGroup("General");
    config->writeEntry("displayKDELibsKDoc", kdocCheck->isChecked() );
 */
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


void DocTreeGlobalConfigWidget::doxygenadd_button_clicked( )
{
    AddDocItemDialog *dialog = new AddDocItemDialog(KFile::Directory, "");
    if (dialog->exec())
    {
        QString url = dialog->url();
        if (url[url.length()-1] == QChar('/')) url.remove(url.length()-1, 1);
        new KListViewItem(doxygen_view, dialog->title(), url);
    }
    delete dialog;
}

void DocTreeGlobalConfigWidget::doxygenedit_button_clicked( )
{
    if (doxygen_view->currentItem())
    {
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::Directory, "", false, 
            doxygen_view->currentItem()->text(0), doxygen_view->currentItem()->text(1));
        if (dialog->exec())
        {
            QString url = dialog->url();
            if (url[url.length()-1] == QChar('/')) url.remove(url.length()-1, 1);
            doxygen_view->currentItem()->setText(0, dialog->title());
            doxygen_view->currentItem()->setText(1, url);
        }
        delete dialog;
    }
}

void DocTreeGlobalConfigWidget::doxygenremove_button_clicked( )
{
    if (doxygen_view->currentItem())
        delete doxygen_view->currentItem();
}

void DocTreeGlobalConfigWidget::qtdocsadd_button_clicked( )
{
    AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "text/xml", true);
    if (dialog->exec())
        new KListViewItem(qtdocs_view, dialog->title(), dialog->url());
    delete dialog;
}

void DocTreeGlobalConfigWidget::qtdocsedit_button_clicked( )
{
    if (qtdocs_view->currentItem())
    {
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "text/xml", true,
            qtdocs_view->currentItem()->text(0), qtdocs_view->currentItem()->text(1));
        if (dialog->exec())
        {
            qtdocs_view->currentItem()->setText(0, dialog->title());
            qtdocs_view->currentItem()->setText(1, dialog->url());
        }
        delete dialog;
    }
}

void DocTreeGlobalConfigWidget::qtdocsremove_button_clicked( )
{
    if (qtdocs_view->currentItem())
        delete qtdocs_view->currentItem();
}

void DocTreeGlobalConfigWidget::kdocadd_button_clicked( )
{
    AddDocItemDialog *dialog = new AddDocItemDialog(KFile::Directory, "");
    if (dialog->exec())
    {
        QString url = dialog->url();
        if (url[url.length()-1] == QChar('/')) url.remove(url.length()-1, 1);
        new KListViewItem(kdoc_view, dialog->title(), url);
    }
    delete dialog;
}

void DocTreeGlobalConfigWidget::kdocedit_button_clicked( )
{
    if (kdoc_view->currentItem())
    {
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::Directory, "", false,
            kdoc_view->currentItem()->text(0), kdoc_view->currentItem()->text(1));
        if (dialog->exec())
        {
            QString url = dialog->url();
            if (url[url.length()-1] == QChar('/')) url.remove(url.length()-1, 1);
            kdoc_view->currentItem()->setText(0, dialog->title());
            kdoc_view->currentItem()->setText(1, url);
        }
        delete dialog;
    }
}

void DocTreeGlobalConfigWidget::kdocremove_button_clicked( )
{
    if (kdoc_view->currentItem())
        delete kdoc_view->currentItem();
}

void DocTreeGlobalConfigWidget::pushEdit_clicked()
{
    if (bListView->currentItem())
    {
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "text/html", false,
            kdoc_view->currentItem()->text(0), kdoc_view->currentItem()->text(1));
        if (dialog->exec())
        {
            QString url = dialog->url();
            if (url[url.length()-1] == QChar('/')) url.remove(url.length()-1, 1);
            kdoc_view->currentItem()->setText(0, dialog->title());
            kdoc_view->currentItem()->setText(1, url);
        }
        delete dialog;
    }
}


#include "doctreeglobalconfigwidget.moc"
