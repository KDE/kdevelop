/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doctreeconfigwidget.h"

#include <qcheckbox.h>
#include <qdom.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qlayout.h>
#include <qvbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kinstance.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kprocess.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kfiledialog.h>

#include "librarydocdlgbase.h"
#include "adddocitemdlg.h"
#include "misc.h"
#include "doctreeviewfactory.h"
#include "doctreeviewwidget.h"

class LibraryDocDlg : public LibraryDocDlgBase
{
public:
    LibraryDocDlg(QString name = QString::null,
                  QString doc = QString::null,
                  QString source = QString::null) 
   : LibraryDocDlgBase(0, "library doc", true) {
      libName->setText(name);
      docURL->setURL(doc);
      sourceURL->setURL(source);
      docURL->fileDialog()->setMode(KFile::Directory);
      sourceURL->fileDialog()->setMode(KFile::Directory);
    }
    QString getLibName() { return libName->text(); }
    QString getDocPath() { return docURL->url(); }
    QString getSourcePath() { return sourceURL->url(); }
};

class DocTreeConfigListItem : public QCheckListItem
{
public:
    DocTreeConfigListItem(QListView *parent, const QString &text,
                          const QString &id, int number)
        : QCheckListItem(parent, text, CheckBox)
        { setOn(true); idnt = id; no = number; }
    QString ident()
        { return idnt; }
    virtual QString key(int, bool) const
        { return QString().setNum(no); }
private:
    QString idnt;
    int no;
};


DocTreeConfigWidget::DocTreeConfigWidget(DocTreeViewWidget *widget,
                                         QWidget *parent, const char *name)
    : QTabWidget(parent, name)
{
    m_widget = widget;

    addTab(librariesTab=createLibrariesTab(), i18n("&Qt/KDE libraries (kdoc)"));
    addTab(bookmarksTab=createBookmarksTab(), i18n("&Bookmarks"));
    
    readConfig();
}


DocTreeConfigWidget::~DocTreeConfigWidget()
{}


QWidget *DocTreeConfigWidget::createLibrariesTab()
{
    QWidget *w = new QWidget(this, "libraries tab");
    
    QLabel *libraries_label = new QLabel(i18n("Items shown in the Qt/KDE Libraries section "
                                              "of the documentation tree:"), w);
    
    libraries_view = new QListView(w);
    QFontMetrics fm(libraries_view->fontMetrics());
    libraries_view->setMinimumWidth(fm.width('X')*35);
    libraries_view->setAllColumnsShowFocus(true);
    libraries_view->setResizeMode(QListView::AllColumns);
    libraries_view->setColumnWidth(0, 64);
    libraries_view->setColumnWidth(1, 64);
    libraries_view->setColumnWidth(2, 64);
    libraries_view->addColumn(i18n("Library"));
    libraries_view->addColumn(i18n("Doc Path"));
    libraries_view->addColumn(i18n("Source Path"));

    QVBox *buttonbox = new QVBox(w);
    buttonbox->setMargin(KDialog::spacingHint());
    connect( new QPushButton(i18n("&Update"), buttonbox), SIGNAL(clicked()),
             this, SLOT(updateLibrary()) );
    connect( new QPushButton(i18n("&Edit"), buttonbox), SIGNAL(clicked()),
             this, SLOT(editLibrary()) );
    connect( new QPushButton(i18n("&Add"), buttonbox), SIGNAL(clicked()),
             this, SLOT(addLibrary()) );
    connect( new QPushButton(i18n("&Remove"), buttonbox), SIGNAL(clicked()),
             this, SLOT(removeLibrary()) );
    
    QGridLayout *layout = new QGridLayout(w, 2, 2, KDialog::marginHint(), KDialog::spacingHint());
    layout->addMultiCellWidget(libraries_label, 0, 0, 0, 1);
    layout->addWidget(libraries_view, 1, 0);
    layout->addWidget(buttonbox, 1, 1);
    
    return w;
}

void DocTreeConfigWidget::updateLibrary()
{
    kdDebug() << "update libraries"<< endl;
}

void DocTreeConfigWidget::editLibrary()
{
    QListViewItem* item = libraries_view->currentItem();
    if (item) {
        LibraryDocDlg dlg(item->text(0),item->text(1),item->text(2));
        if (dlg.exec()) {
            item->setText(0, dlg.getLibName());
            item->setText(1, dlg.getDocPath());
            item->setText(2, dlg.getSourcePath());
        }        
    }
}

void DocTreeConfigWidget::addLibrary()
{
    LibraryDocDlg dlg;
    if (dlg.exec()) {
        new QListViewItem(libraries_view, dlg.getLibName(), dlg.getDocPath(), dlg.getSourcePath());
    }            
}

void DocTreeConfigWidget::removeLibrary()
{
    QListViewItem* item = libraries_view->currentItem();
    if (item) {
        libraries_view->removeItem(item);
    }
}


QWidget *DocTreeConfigWidget::createBookmarksTab()
{
    QWidget *w = new QWidget(this, "bookmarks tab");
    
    QLabel *bookmarks_label = new QLabel(i18n("Items shown in the Bookmarks section of the documentation tree:"), w);
    
    bookmarks_view = new QListView(w);
    QFontMetrics fm(bookmarks_view->fontMetrics());
    bookmarks_view->setMinimumWidth(fm.width('X')*35);
    bookmarks_view->setAllColumnsShowFocus(true);
    bookmarks_view->setResizeMode(QListView::AllColumns);
    bookmarks_view->setColumnWidth(0, 70);
    bookmarks_view->setColumnWidth(1, 170);
    bookmarks_view->addColumn(i18n("Title"));
    bookmarks_view->addColumn(i18n("URL"));
    
    QVBox *buttonbox = new QVBox(w);
    buttonbox->setMargin(KDialog::spacingHint());
    connect( new QPushButton(i18n("&Add..."), buttonbox), SIGNAL(clicked()),
             this, SLOT(addBookmarkClicked()) );
    connect( new QPushButton(i18n("&Remove"), buttonbox), SIGNAL(clicked()),
             this, SLOT(removeBookmarkClicked()) );

    QGridLayout *layout = new QGridLayout(w, 2, 2, KDialog::marginHint(), KDialog::spacingHint());
    layout->addMultiCellWidget(bookmarks_label, 0, 0, 0, 1);
    layout->addWidget(bookmarks_view, 1, 0);
    layout->addWidget(buttonbox, 1, 1);
    
    return w;
}


void DocTreeConfigWidget::showPage(DocTreeConfigWidget::Page page)
{
    QTabWidget::showPage((page==KDevelop)? kdevelopTab :
                         (page==Libraries)? librariesTab : bookmarksTab);
}


void DocTreeConfigWidget::readConfig()
{
#if 0
    QString path = locate("appdata", "tools/documentation");
    KSimpleConfig docconfig(path);
    // Read in possible items for the KDevelop tree
    docconfig.setGroup("Contents");
    QStringList kdevelopEntries = docconfig.readListEntry("KDevelopEntries");
    int kdevelopPos = 0;
    for (QStringList::Iterator it = kdevelopEntries.begin();
         it != kdevelopEntries.end();
         ++it)
	{
            docconfig.setGroup("KDevelop-" + (*it));
            QString name = docconfig.readEntry("Name"); 
            (void) new DocTreeConfigListItem(kdevelop_view, name, (*it), kdevelopPos);
	    kdDebug(9002) << "Inserting " << name << endl;
            ++kdevelopPos;
	}
#endif
    // Read in possible items for the Libraries tree
    QStringList libNames, docDirs, sourceDirs;
    DocTreeViewTool::getLibraries(&libNames, &docDirs, &sourceDirs);
    // TODO:will merge this stuff with the generalised view -- exa
    QStringList librariesTitle, librariesURL, librariesHidden;
    DocTreeViewTool::getAllLibraries(&librariesTitle, &librariesURL);
    DocTreeViewTool::getHiddenLibraries(&librariesHidden);
        
    QStringList::Iterator libName, docDir, sourceDir;
    for (libName = libNames.begin(),
         docDir = docDirs.begin(),
         sourceDir = sourceDirs.begin() ;
         libName!=libNames.end() && docDir!=docDirs.end() && sourceDir!=sourceDirs.end();
         ++libName, ++docDir, ++sourceDir) {
         new QListViewItem(libraries_view, *libName, *docDir, *sourceDir);
    }
// load the docs
//    QStringList librariesTitle, librariesURL, librariesHidden;
//    DocTreeViewTool::readLibraryDocs(*docDir,&librariesTitle, &librariesURL);    
//    int librariesPos = 0;
//    QStringList::Iterator lit1, lit2;
//    for (lit1 = librariesTitle.begin(), lit2 = librariesURL.begin();
//         lit1 != librariesTitle.end() && lit2 != librariesURL.end();
//         ++lit1, ++lit2) {
//        (void) new DocTreeConfigListItem(libraries_view, (*lit1), (*lit2), librariesPos);
//      kdDebug(9002) << "Insert " << (*lit2) << endl;
//      ++librariesPos;
//  }

#if 0
    QStringList kdevelopHidden;
    // Enable/disable items in the KDevelop tree
    DocTreeViewTool::getHiddenKDevelop(&kdevelopHidden);
    for (QStringList::Iterator it = kdevelopHidden.begin(); it != kdevelopHidden.end();
         ++it)
	{
            QListViewItem *item = kdevelop_view->firstChild();
            for (; item; item = item->nextSibling())
                {
                    DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
                    kdDebug(9002) << "Checking " << citem->ident() << " with " << (*it) << endl;
                    if (citem->ident() == (*it))
                        citem->setOn(false);
                }
        }
#endif
    
    // Enable/disable items in the Libraries tree
    DocTreeViewTool::getHiddenLibraries(&librariesHidden);
    for (QStringList::Iterator it = librariesHidden.begin(); it != librariesHidden.end();
         ++it) {
        QListViewItem *item = libraries_view->firstChild();
        for (; item; item = item->nextSibling()) {
            DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
            // kdDebug(9002) << "Checking " << citem->ident() << " with " << (*it) << endl;
            if (citem->ident() == (*it))
                citem->setOn(false);
        }
    }
    
    // Read in configuration of the Bookmarks tree
    // Here we store what we _want_ to see, in contrast to the KDevelop/Libraries
    // sections where we store what  we _don't_ want to see
    QStringList bookmarksTitle, bookmarksURL;
    DocTreeViewTool::getBookmarks(&bookmarksTitle, &bookmarksURL);
    QStringList::Iterator oit1, oit2;
    for (oit1 = bookmarksTitle.begin(), oit2 = bookmarksURL.begin();
         oit1 != bookmarksTitle.end() && oit2 != bookmarksURL.end();
         ++oit1, ++oit2) {
        new QListViewItem(bookmarks_view, *oit1, *oit2);
    }
}


void DocTreeConfigWidget::storeConfig()
{
    kdDebug() << "DocTreeConfigWidget::storeConfig()" << endl;

#if 0
    // Save KDevelop section
    QStringList kdevelopHidden;
    {
        QListViewItem *item = kdevelop_view->firstChild();
        for (; item; item = item->nextSibling()) {
            DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
            if (!citem->isOn())
                kdevelopHidden.append(citem->ident());
        }
    }
    DocTreeViewTool::setHiddenKDevelop(kdevelopHidden);
#endif
    
    // Save Libraries section
//    QStringList librariesHidden;
    QStringList libNames, docDirs, sourceDirs;
    {
        QListViewItem *item = libraries_view->firstChild();
        for (; item; item = item->nextSibling()) {
            libNames.append(item->text(0));
            docDirs.append(item->text(1));
            sourceDirs.append(item->text(2));
//            DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
//            if (!citem->isOn())
//                librariesHidden.append(citem->ident());
        }
    }
    DocTreeViewTool::setLibraries(&libNames, &docDirs, &sourceDirs);
//    DocTreeViewTool::setHiddenLibraries(librariesHidden);
    
    // Save Bookmarks section
    QStringList bookmarksTitle, bookmarksURL;
    {
        QListViewItem *item = bookmarks_view->firstChild();
        for (; item; item = item->nextSibling()) {
            bookmarksTitle.append(item->text(0));
            bookmarksURL.append(item->text(1));
        }
    }
    DocTreeViewTool::setBookmarks(bookmarksTitle, bookmarksURL);
}


void DocTreeConfigWidget::addBookmarkClicked()
{
    AddDocItemDialog dlg;
    if (!dlg.exec())
        return;

    (void) new QListViewItem(bookmarks_view, dlg.title(), dlg.url());
}


void DocTreeConfigWidget::removeBookmarkClicked()
{
    delete bookmarks_view->currentItem();
}


void DocTreeConfigWidget::accept()
{
    storeConfig();
    m_widget->configurationChanged();
}

#include "doctreeconfigwidget.moc"
