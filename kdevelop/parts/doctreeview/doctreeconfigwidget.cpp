/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qvbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kdialog.h>
#include <klistview.h>

#include "cdoctreepropdlg.h"
#include "doctreeview.h"
#include "doctreeconfigwidget.h"


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



DocTreeConfigWidget::DocTreeConfigWidget(DocTreeView *docpart, QWidget *parent, const char *name)
    : QTabWidget(parent, name)
{
    part = docpart;
    
    // KDevelop tab
    kdevelopTab = new QHBox(this);
    kdevelopTab->setMargin(15);
    kdevelopTab->setSpacing(15);
    (void) new QLabel(i18n("Items shown in the \n"
                           "KDevelop section \n"
                           "of the documentation tree:"), kdevelopTab);
    kdevelopView = new KListView(kdevelopTab);
    kdevelopView->addColumn("");
    kdevelopView->header()->hide();

    // Libraries tab
    librariesTab = new QHBox(this);
    librariesTab->setMargin(15);
    librariesTab->setSpacing(15);
    (void) new QLabel(i18n("Items shown in the \n"
                           "Qt/KDE Libraries section \n"
                           "of the documentation tree:"), librariesTab);
    librariesView = new KListView(librariesTab);
    librariesView->addColumn("");
    librariesView->header()->hide();

    // Others tab
    othersTab = new QHBox(this);
    othersTab->setMargin(15);
    othersTab->setSpacing(15);
    (void) new QLabel(i18n("Items shown in the \n"
                           "Others section of \n"
                           "the documentation tree:"), othersTab);
    othersView = new KListView(othersTab);
    othersView->setAllColumnsShowFocus(true);
    othersView->setColumnWidth(0, 70);
    othersView->setColumnWidth(1, 70);
    othersView->addColumn(i18n("Title"));
    othersView->addColumn(i18n("URL"));
    QVBox *buttonbox = new QVBox(othersTab);
    buttonbox->setMargin(KDialog::spacingHint());
    connect( new QPushButton(i18n("Add..."), buttonbox), SIGNAL(clicked()), this, SLOT(addClicked()) );
    connect( new QPushButton(i18n("Edit..."), buttonbox), SIGNAL(clicked()), this, SLOT(editClicked()) );
    connect( new QPushButton(i18n("Remove"), buttonbox), SIGNAL(clicked()), this, SLOT(removeClicked()) );

    addTab(kdevelopTab,  i18n("KDevelop"));
    addTab(librariesTab, i18n("Qt/KDE libraries"));
    addTab(othersTab,    i18n("Others section"));

    readConfig();
}


DocTreeConfigWidget::~DocTreeConfigWidget()
{}


void DocTreeConfigWidget::showPage(DocTreeConfigWidget::Page page)
{
    QTabWidget::showPage((page==KDevelop)? kdevelopTab :
                         (page==Libraries)? librariesTab : othersTab);
}


void DocTreeConfigWidget::readConfig()
{
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
            (void) new DocTreeConfigListItem(kdevelopView, name, (*it), kdevelopPos);
	    kdDebug(9002) << "Inserting " << name << endl;
            ++kdevelopPos;
	}
    // Read in possible items for the Libraries tree
    docconfig.setGroup("Contents");
    QStringList librariesEntries = docconfig.readListEntry("LibrariesEntries");
    int librariesPos = 0;
    for (QStringList::Iterator it = librariesEntries.begin();
         it != librariesEntries.end();
         ++it)
	{
            docconfig.setGroup("Libraries-" + (*it));
            QString name = docconfig.readEntry("Name"); 
            (void) new DocTreeConfigListItem(librariesView, name, (*it), librariesPos);
	    kdDebug(9002) << "Inserting " << name << endl;
            ++librariesPos;
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
                    DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
                    kdDebug(9002) << "Checking " << citem->ident() << " with " << (*it) << endl;
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
                    DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
                    kdDebug(9002) << "Checking " << citem->ident() << " with " << (*it) << endl;
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
}


void DocTreeConfigWidget::storeConfig()
{
    KConfig *config = KGlobal::config();
    config->setGroup("DocTree");
    // Save KDevelop section
    QStringList kdevelopNotShown;
    {
        QListViewItem *item = kdevelopView->firstChild();
        for (; item; item = item->nextSibling())
            {
                DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
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
                DocTreeConfigListItem *citem = static_cast<DocTreeConfigListItem*>(item);
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
}


void DocTreeConfigWidget::addClicked()
{
    CDocTreePropDlg dlg;
    dlg.setCaption(i18n("Add documentation entry"));
    if (!dlg.exec())
        return;

    (void) new QListViewItem(othersView, dlg.name_edit->text(), dlg.file_edit->text());
}


void DocTreeConfigWidget::editClicked()
{
    CDocTreePropDlg dlg;
    dlg.setCaption(i18n("Edit documentation entry"));
    if (!dlg.exec())
        return;

    (void) new QListViewItem(othersView, othersView->currentItem(),
                             dlg.name_edit->text(), dlg.file_edit->text());
    delete othersView->currentItem();
}


void DocTreeConfigWidget::removeClicked()
{
    delete othersView->currentItem();
}


void DocTreeConfigWidget::accept()
{
    storeConfig();
    part->configurationChanged();
}
#include "doctreeconfigwidget.moc"
