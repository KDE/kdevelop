/***************************************************************************
                             doctreeconfdlg.cpp
                             -------------------
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
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
#include <qpushbutton.h>
#include <qlistview.h>
#include <qheader.h>
#include <qhbox.h>
#include <qvbox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#include "klistview.h"
#include "cdoctreepropdlg.h"
#include "doctreeviewconfigwidget.h"
#include "doctreeconfdlg.h"


// This should become a thin wrapper around DocTreeViewConfigWidget

DocTreeConfigDialog::DocTreeConfigDialog(Page page, QWidget *parent, const char *name)
    : QTabDialog(parent, name, true)
{
    setCaption(i18n("Configure Documentation Tree"));

    // KDevelop tab
    QHBox *kw = new QHBox(this);
    kw->setMargin(15);
    kw->setSpacing(15);
    (void) new QLabel(i18n("Items shown in the \n"
                           "KDevelop section \n"
                           "of the documentation tree:"), kw);
    kdevelopView = new KListView(kw);
    kdevelopView->addColumn("");
    kdevelopView->header()->hide();

    // Libraries tab
    QHBox *lw = new QHBox(this);
    lw->setMargin(15);
    lw->setSpacing(15);
    (void) new QLabel(i18n("Items shown in the \n"
                           "Qt/KDE Libraries section \n"
                           "of the documentation tree:"), lw);
    librariesView = new KListView(lw);
    librariesView->addColumn("");
    librariesView->header()->hide();

    // Others tab
    QHBox *ow = new QHBox(this);
    ow->setMargin(15);
    ow->setSpacing(15);
    (void) new QLabel(i18n("Items shown in the \n"
                           "Others section of \n"
                           "the documentation tree:"), ow);
    othersView = new KListView(ow);
    othersView->setAllColumnsShowFocus(true);
    othersView->setColumnWidth(0, 30);
    othersView->setColumnWidth(1, 30);
    othersView->addColumn(i18n("Title"));
    othersView->addColumn(i18n("URL"));
    QVBox *buttonbox = new QVBox(ow);
    buttonbox->setMargin(KDialog::spacingHint());
    connect( new QPushButton(i18n("Add..."), buttonbox), SIGNAL(clicked()), this, SLOT(addClicked()) );
    connect( new QPushButton(i18n("Edit..."), buttonbox), SIGNAL(clicked()), this, SLOT(editClicked()) );
    connect( new QPushButton(i18n("Remove"), buttonbox), SIGNAL(clicked()), this, SLOT(removeClicked()) );

    addTab(kw, i18n("KDevelop"));
    addTab(lw, i18n("Qt/KDE libraries"));
    addTab(ow, i18n("Others section"));

    showPage((page==KDevelop)? kw : (page==Libraries)? lw : ow);

    readConfig();
}


DocTreeConfigDialog::~DocTreeConfigDialog()
{}

void DocTreeConfigDialog::readConfig()
{
#if 0
    QString path = locate("appdata", "tools/documentation");
    KSimpleConfig docconfig(path);
    // Read in possible items for the KDevelop tree
    docconfig.setGroup("Contents");
    QStringList kdevelopEntries = docconfig.readListEntry("KDevelopEntries");
    for (QStringList::Iterator it = kdevelopEntries.begin();
         it != kdevelopEntries.end();
         ++it)
	{
            docconfig.setGroup("KDevelop-" + (*it));
            QString name = docconfig.readEntry("Name"); if (name.isEmpty()) name = (*it);
            //            (void) new DocTreeConfigCheckListItem(kdevelopView, name, (*it));
	    qDebug( "Insert %s", name.ascii() );
	}
    // Read in possible items for the Libraries tree
    docconfig.setGroup("Contents");
    QStringList librariesEntries = docconfig.readListEntry("LibrariesEntries");
    for (QStringList::Iterator it = librariesEntries.begin();
         it != librariesEntries.end();
         ++it)
	{
            docconfig.setGroup("Libraries-" + (*it));
            QString name = docconfig.readEntry("Name"); if (name.isEmpty()) name = (*it);
            //            (void) new DocTreeConfigCheckListItem(librariesView, name, (*it));
	    qDebug( "Insert %s", name.ascii() );
	}

    KConfig *config = KGlobal::config();
    config->setGroup("DocTree");
    // Enable/disable items in the KDevelop tree
    QStringList kdevelopNotShown = config->readListEntry("KDevelopNotShown");
    for (QStringList::Iterator it = kdevelopNotShown.begin();
         it != kdevelopNotShown.end();
         ++it)
	{
            QListViewItem *item = kdevelopView->firstChild();
            for (; item; item = item->nextSibling())
                {
                    DocTreeConfigCheckListItem *citem = static_cast<DocTreeConfigCheckListItem*>(item);
                    qDebug( "Checking %s with %s", citem->ident().ascii(), (*it).ascii() );
                    if (citem->ident() == (*it))
                        citem->setOn(false);
                }
        }
    // Enable/disable items in the Libraries tree
    QStringList librariesNotShown = config->readListEntry("LibrariesNotShown");
    for (QStringList::Iterator it = librariesNotShown.begin();
         it != librariesNotShown.end();
         ++it)
	{
            QListViewItem *item = librariesView->firstChild();
            for (; item; item = item->nextSibling())
                {
                    DocTreeConfigCheckListItem *citem = static_cast<DocTreeConfigCheckListItem*>(item);
                    qDebug( "Checking %s with %s", citem->ident().ascii(), (*it).ascii() );
                    if (citem->ident() == (*it))
                        citem->setOn(false);
                }
        }

    // Read in configuration of the Others tree
    // Here we store what we _want_ to see, in contrast to the KDevelop/Libraries
    // sections where we store what  we _don't_ want to see 
    QStringList othersShownTitle = config->readListEntry("OthersShownTitle");
    QStringList othersShownURL = config->readListEntry("OthersShownURL");
    QStringList::Iterator it1 = othersShownTitle.begin();
    QStringList::Iterator it2 = othersShownURL.begin();
    for (; it1 != othersShownTitle.end() && it2 != othersShownURL.end(); ++it1, ++it2)
        new QListViewItem(othersView, *it1, *it2);
#endif
}


void DocTreeConfigDialog::storeConfig()
{
#if 0
    KConfig *config = KGlobal::config();
    config->setGroup("DocTree");
    // Save KDevelop section
    QStringList kdevelopNotShown;
    {
        QListViewItem *item = kdevelopView->firstChild();
        for (; item; item = item->nextSibling())
            {
                DocTreeConfigCheckListItem *citem = static_cast<DocTreeConfigCheckListItem*>(item);
                if (!citem->isOn())
                    kdevelopNotShown.append(citem->ident());
            }
    }
    // Save Libraries section
    config->writeEntry("KDevelopNotShown", kdevelopNotShown);
    QStringList librariesNotShown;
    {
        QListViewItem *item = librariesView->firstChild();
        for (; item; item = item->nextSibling())
            {
                DocTreeConfigCheckListItem *citem = static_cast<DocTreeConfigCheckListItem*>(item);
                if (!citem->isOn())
                    librariesNotShown.append(citem->ident());
            }
    }
    config->writeEntry("LibrariesNotShown", librariesNotShown);
    // Save Others section
    QStringList othersShownTitle, othersShownURL;
    {
        QListViewItem *item = othersView->firstChild();
        for (; item; item = item->nextSibling())
            {
                othersShownTitle.append(item->text(0));
                othersShownURL.append(item->text(1));
            }
    }
    config->writeEntry("OthersShownTitle", othersShownTitle);
    config->writeEntry("OthersShownURL", othersShownURL);
#endif
}


void DocTreeConfigDialog::addClicked()
{
    CDocTreePropDlg dlg;
    dlg.setCaption(i18n("Add documentation entry"));
    if (!dlg.exec())
        return;

    (void) new QListViewItem(othersView, dlg.name_edit->text(), dlg.file_edit->text());
}


void DocTreeConfigDialog::editClicked()
{
    CDocTreePropDlg dlg;
    dlg.setCaption(i18n("Edit documentation entry"));
    if (!dlg.exec())
        return;

    (void) new QListViewItem(othersView, othersView->currentItem(),
                             dlg.name_edit->text(), dlg.file_edit->text());
    delete othersView->currentItem();
}


void DocTreeConfigDialog::removeClicked()
{
    delete othersView->currentItem();
}


void DocTreeConfigDialog::accept()
{
    storeConfig();
    QTabDialog::accept();
}
