/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
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
#include <kdeversion.h>
#include "../../config.h"
#include "doctreeviewpart.h"
#include "doctreeviewwidget.h"
#include "doctreeviewfactory.h"
#include "adddocitemdlg.h"
#include "misc.h"
#include "domutil.h"
#include "urlutil.h"
#include "librarydocdlg.h"
#include <qfileinfo.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <kurllabel.h>

DocTreeGlobalConfigWidget::DocTreeGlobalConfigWidget(DocTreeViewPart *part, DocTreeViewWidget *widget,
                                                     QWidget *parent, const char *name)
    : DocTreeGlobalConfigWidgetBase(parent, name)
{
    extEnableButton->hide();
    extDisableButton->hide();
    dhEnableButton->hide();
    dhDisableButton->hide();
    m_part = part;
    QDomDocument d;
    if(m_part->projectDom()) d = *m_part->projectDom();
    m_ignoreTocs = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoretocs", "toc");
    m_ignoreDevHelp = DomUtil::readListEntry(d, "/kdevdoctreeview/ignoredevhelp", "toc");
    m_widget = widget;
    readConfig();
    if (!m_part->project())
    {
//        extEditButton->setEnabled(false);
        extEnableButton->setEnabled(false);
        extDisableButton->setEnabled(false);
        dhEnableButton->setEnabled(false);
        dhDisableButton->setEnabled(false);
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
//    extListView->addColumn(i18n("Enabled"));
    extListView->addColumn(i18n("Title"));
    extListView->addColumn(i18n("URL"));
    extListView->setAllColumnsShowFocus(true);

    dhListView->addColumn(i18n("Name"));
//    dhListView->addColumn(i18n("Enabled"));
    dhListView->addColumn(i18n("Title"));
    dhListView->addColumn(i18n("URL"));
    dhListView->addColumn(i18n("Author"));
    dhListView->setAllColumnsShowFocus(true);

    dhURL->setMode((int) KFile::Directory);

    urlDownload->setURL("http://lidn.sourceforge.net");
    connect(urlDownload, SIGNAL(leftClickedURL(const QString&)), kapp,
        SLOT(invokeBrowser(const QString&)));
/*
    connect(extListView, SIGNAL(executed( QListViewItem *)), this,
            SLOT( extEdit()));
    connect(bListView, SIGNAL(executed( QListViewItem *)), this,
            SLOT(  pushEdit_clicked()));
    connect(dhListView, SIGNAL(executed( QListViewItem *)), this,
            SLOT(  dhEditButton_clicked()));
*/
#if QT_VERSION >= 0x030200
    connect(extListView, SIGNAL( doubleClicked( QListViewItem *, const QPoint&, int ) ), this,
            SLOT( extEdit()));
    connect(bListView, SIGNAL( doubleClicked( QListViewItem *, const QPoint&, int ) ), this,
            SLOT(  pushEdit_clicked()));
    connect(dhListView, SIGNAL( doubleClicked( QListViewItem *, const QPoint&, int ) ), this,
            SLOT(  dhEditButton_clicked()));
#else
    connect(extListView, SIGNAL( doubleClicked( QListViewItem * ) ), this,
            SLOT( extEdit()));
    connect(bListView, SIGNAL( doubleClicked( QListViewItem * ) ), this,
            SLOT(  pushEdit_clicked()));
    connect(dhListView, SIGNAL( doubleClicked( QListViewItem * ) ), this,
            SLOT(  dhEditButton_clicked()));
#endif

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
        /*UNUSED! KListViewItem *qtitem = */ new KListViewItem(qtdocs_view, it.key(), config->readPathEntry(it.key()));
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
        /*UNUSED! KListViewItem *qtitem = */ new KListViewItem(doxygen_view, itx.key(), config->readPathEntry(itx.key()));
    }
    if (xmap.empty() && (!QString(KDELIBS_DOXYDIR).isEmpty()))
    {
        /*UNUSED! KListViewItem *qtitem = */ new KListViewItem(doxygen_view, "KDE Libraries (Doxygen)", KDELIBS_DOXYDIR);
    }


    config->setGroup("General KDoc");
    QMap<QString, QString> dmap = config->entryMap("General KDoc");
    QMap<QString, QString>::Iterator itd;
    for (itd = dmap.begin(); itd != dmap.end(); ++itd)
    {
        /*UNUSED! KListViewItem *qtitem = */ new KListViewItem(kdoc_view, itd.key(), config->readPathEntry(itd.key()));
    }
    if (dmap.empty() && (!QString(KDELIBS_DOCDIR).isEmpty()))
    {
        /*UNUSED! KListViewItem *qtitem = */ new KListViewItem(kdoc_view, "KDE Libraries (KDoc)", KDELIBS_DOCDIR);
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
    indexKDevelopBox->setChecked(config->readBoolEntry("IndexKDevelop"));
    indexQtBox->setChecked(config->readBoolEntry("IndexQt"));
    indexKdelibsBox->setChecked(config->readBoolEntry("IndexKdelibs"));
    indexBooksBox->setChecked(config->readBoolEntry("IndexBooks"));
    indexBookmarksBox->setChecked(config->readBoolEntry("IndexBookmarks"));

    config->setGroup("htdig");
    QString exe = kapp->dirs()->findExe("htdig");
    htdigbinEdit->setURL(config->readPathEntry("htdigbin", exe));
    exe = kapp->dirs()->findExe("htmerge");
    htmergebinEdit->setURL(config->readPathEntry("htmergebin", exe));
    exe = kapp->dirs()->findExe("htsearch");
    htsearchbinEdit->setURL(config->readPathEntry("htsearchbin", exe));

    //get bookmarks
    QStringList bookmarksTitle, bookmarksURL;
    DocTreeViewTool::getBookmarks(&bookmarksTitle, &bookmarksURL);
    QStringList::Iterator oit1, oit2;
    for (oit1 = bookmarksTitle.begin(), oit2 = bookmarksURL.begin();
         oit1 != bookmarksTitle.end() && oit2 != bookmarksURL.end();
         ++oit1, ++oit2) {
        new KListViewItem( bListView, *oit1, *oit2);
    }

    config->setGroup("DevHelp");
    dhURL->setURL(config->readPathEntry("DevHelpDir"));

    readTocConfigs();
    DocTreeViewTool::scanDevHelpDirs();
    readDevHelpConfig();
}

void DocTreeGlobalConfigWidget::readTocConfigs()
{
    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        const QString name( QFileInfo(*tit).baseName() );
        const QString location( DocTreeViewTool::tocLocation( *tit ) );
        const QString title (DocTreeViewTool::tocTitle( *tit ));
        new KListViewItem( extListView, name, title, location);
/*        if( m_ignoreTocs.contains( name ) )
            new KListViewItem( extListView, name, "false", title, location);
        else
            new KListViewItem( extListView, name, "true", title, location);*/
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
#if defined(KDE_IS_VERSION)
# if KDE_IS_VERSION(3,1,3)
#  ifndef _KDE_3_1_3_
#   define _KDE_3_1_3_
#  endif
# endif
#endif

#if defined(_KDE_3_1_3_)
    config->writePathEntry("htdigbin", htdigbinEdit->url());
    config->writePathEntry("htmergebin", htmergebinEdit->url());
    config->writePathEntry("htsearchbin", htsearchbinEdit->url());
#else
    config->writeEntry("htdigbin", htdigbinEdit->url());
    config->writeEntry("htmergebin", htmergebinEdit->url());
    config->writeEntry("htsearchbin", htsearchbinEdit->url());
#endif

    QStringList bookmarksTitle, bookmarksURL;
    {
        QListViewItem *item = bListView->firstChild();
        for (; item; item = item->nextSibling()) {
            bookmarksTitle.append(item->text(0));
            bookmarksURL.append(item->text(1));
        }
    }
    DocTreeViewTool::setBookmarks(bookmarksTitle, bookmarksURL);

    if (!dhURL->url().isEmpty())
    {
        config->setGroup("DevHelp");
#if defined(_KDE_3_1_3_)
        config->writePathEntry("DevHelpDir", dhURL->url());
#else
        config->writeEntry("DevHelpDir", dhURL->url());
#endif
    }
    config->sync();
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
        LibraryDocDlg *dlg = new LibraryDocDlg( this, name.latin1(), location, _default, "TocDirs");
        dlg->libName->setEnabled(false);
        dlg->libSource->setEnabled(false);
        dlg->exec();

        delete dlg;
    }
    extListView->clear();
    readTocConfigs();
}

void DocTreeGlobalConfigWidget::extEnable()
{
/*    QListViewItem *item( extListView->selectedItem() );
    if( item && item->text(1) == "false" )
    {
        m_ignoreTocs.remove( item->text( 0 ) );
        DomUtil::writeListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc", m_ignoreTocs );
        item->setText(1, "true");
   }*/
}

void DocTreeGlobalConfigWidget::extDisable()
{
    //kdDebug(9002) << "disable" << endl;
/*    QListViewItem *item( extListView->selectedItem() );
    if( item && item->text(1) == "true" )
    {
        m_ignoreTocs << item->text( 0 );
        DomUtil::writeListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc", m_ignoreTocs );
        item->setText(1, "false");
    }*/
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
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::Directory, "", AddDocItemDialog::None,
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
    AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "*.xml *.dcf", AddDocItemDialog::Qt);
    if (dialog->exec())
        new KListViewItem(qtdocs_view, dialog->title(), dialog->url());
    delete dialog;
}

void DocTreeGlobalConfigWidget::qtdocsedit_button_clicked( )
{
    if (qtdocs_view->currentItem())
    {
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "*.xml *.dcf", AddDocItemDialog::Qt,
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
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::Directory, "", AddDocItemDialog::None,
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
        AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "text/html", AddDocItemDialog::None,
            bListView->currentItem()->text(0), bListView->currentItem()->text(1));
        if (dialog->exec())
        {
            bListView->currentItem()->setText(0, dialog->title());
            bListView->currentItem()->setText(1, dialog->url());
        }
        delete dialog;
    }
}

void DocTreeGlobalConfigWidget::extAddButton_clicked( )
{
    AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "*.toc", AddDocItemDialog::KDevelopTOC);
    if (dialog->exec())
    {
        KListViewItem *item = new KListViewItem(extListView);
        dhListView->setCurrentItem(item);

        QFileInfo fi(dialog->url());
        dhListView->currentItem()->setText(0, fi.baseName());
//        dhListView->currentItem()->setText(1, "true");

//        QString localURL = locateLocal("data", QString("kdevdoctreeview/tocs/") + dialog->title());
        QString localURL = DocTreeViewFactory::instance()->dirs()->saveLocation("doctocs") + fi.baseName() + ".toc";
        KURL src;
        src.setPath(dialog->url());
        KURL dest;
        dest.setPath(localURL);
        KIO::NetAccess::copy(src, dest);

        dhListView->currentItem()->setText(2, DocTreeViewTool::tocLocation( localURL ));
        dhListView->currentItem()->setText(1, DocTreeViewTool::tocTitle( localURL ));
    }
    delete dialog;
}

void DocTreeGlobalConfigWidget::extRemoveButton_clicked( )
{
    if (extListView->currentItem())
    {
        KURL url;
        url.setPath(DocTreeViewFactory::instance()->dirs()->findResource("doctocs",
            extListView->currentItem()->text(0) + QString(".toc")));
        if (! KIO::NetAccess::del(url))
            KMessageBox::error(this, i18n("Could not remove documentation TOC.\nIt may be a part of system-wide KDevelop documentation."));
        else
            delete extListView->currentItem();
    }
}

void DocTreeGlobalConfigWidget::dhAddButton_clicked( )
{
    AddDocItemDialog *dialog = new AddDocItemDialog(KFile::File, "*.devhelp", AddDocItemDialog::DevHelp);
    if (dialog->exec())
    {
        KListViewItem *item = new KListViewItem(dhListView);
        dhListView->setCurrentItem(item);

        QFileInfo fi(dialog->url());

        BookInfo inf = DocTreeViewTool::devhelpInfo(dialog->url());
        dhListView->currentItem()->setText(0, fi.baseName(false));
//        dhListView->currentItem()->setText(1, "true");
        dhListView->currentItem()->setText(1, inf.title);
        if (!inf.defaultLocation.isEmpty())
            dhListView->currentItem()->setText(2, inf.defaultLocation);
        else
            dhListView->currentItem()->setText(2, URLUtil::directory(fi.absFilePath()));
        dhListView->currentItem()->setText(3, inf.author);

//        QString localURL = locateLocal("data", QString("kdevdoctreeview/tocs/") + dialog->title());
        QString localURL = DocTreeViewFactory::instance()->dirs()->saveLocation("docdevhelp") + fi.baseName()  + ".devhelp";
        KURL src;
        src.setPath(dialog->url());
        KURL dest;
        dest.setPath(localURL);
        KIO::NetAccess::copy(src, dest);
    }
    delete dialog;
}

void DocTreeGlobalConfigWidget::dhDisableButton_clicked( )
{
/*    QListViewItem *item( dhListView->selectedItem() );
    if( item && item->text(1) == "true" )
    {
        m_ignoreDevHelp << item->text( 0 );
        DomUtil::writeListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoredevhelp", "toc", m_ignoreDevHelp );
        item->setText(1, "false");
    }*/
}

void DocTreeGlobalConfigWidget::dhEditButton_clicked( )
{
    QListViewItem *item = dhListView->currentItem();
    if( item )
    {
        const QString name( item->text(0) );
        const QString location( item->text(2) );
        KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
        QStringList tocs = dirs->findAllResources("docdevhelp", QString::null, false, true);
        QString filePath;
        for( QStringList::Iterator it = tocs.begin(); it!=tocs.end(); ++it)
        {
            if(QFileInfo(*it).baseName() == name)
                filePath = *it;
        }
        const QString _default( DocTreeViewTool::devhelpInfo(filePath).defaultLocation );
        LibraryDocDlg *dlg = new LibraryDocDlg( this, name.latin1(), location, _default, "TocDevHelp");
        dlg->libName->setEnabled(false);
        dlg->libSource->setEnabled(false);
        dlg->exec();

        delete dlg;
    }
    dhListView->clear();
    readDevHelpConfig();
}

void DocTreeGlobalConfigWidget::dhEnableButton_clicked( )
{
/*    QListViewItem *item( dhListView->selectedItem() );
    if( item && item->text(1) == "false" )
    {
        m_ignoreDevHelp.remove( item->text( 0 ) );
        DomUtil::writeListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoredevhelp", "toc", m_ignoreDevHelp );
        item->setText(1, "true");
    }*/
}

void DocTreeGlobalConfigWidget::dhRemoveButton_clicked( )
{
    if (dhListView->currentItem())
    {
        KURL url;
        url.setPath(DocTreeViewFactory::instance()->dirs()->findResource("docdevhelp",
            dhListView->currentItem()->text(0) + QString(".devhelp")));
        if (! KIO::NetAccess::del(url))
            KMessageBox::error(this, i18n("Could not remove documentation TOC.\nIt may be a part of system-wide KDevelop documentation."));
        else
        {
            KConfig *config = DocTreeViewFactory::instance()->config();
            config->setGroup("TocDevHelp");
            config->deleteEntry(dhListView->currentItem()->text(0));
            delete dhListView->currentItem();
        }
    }
}

void DocTreeGlobalConfigWidget::dhScanButton_clicked( )
{
    dhListView->clear();
    DocTreeViewTool::scanDevHelpDirs(dhURL->url());
    readDevHelpConfig();
}

void DocTreeGlobalConfigWidget::readDevHelpConfig()
{
    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();

    QStringList tocs = dirs->findAllResources("docdevhelp", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        KListViewItem *item = 0;

        QFileInfo fi(*tit);

        BookInfo inf = DocTreeViewTool::devhelpInfo(*tit);
/*        if( m_ignoreDevHelp.contains( fi.baseName() ) )
            item = new KListViewItem( dhListView, "", "false");
        else
            item = new KListViewItem( dhListView, "", "true");*/
        item = new KListViewItem(dhListView);
        item->setText(0, fi.baseName(false));
        item->setText(1, inf.title);
        item->setText(2, DocTreeViewTool::devhelpLocation(fi.baseName() , inf.defaultLocation));
        item->setText(3, inf.author);
    }
}

#include "doctreeglobalconfigwidget.moc"
