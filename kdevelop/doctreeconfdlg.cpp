/***************************************************************************
                             doctreeconfdlg.cpp
                             -------------------

    copyright            : (C) 1999 by The KDevelop Team
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
#include <qlabel.h>
#include <qlistview.h>
#include <qmap.h>
#include <kbuttonbox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#include "klistview.h"
#include "doctreeconfdlg.h"


#include <iostream>
DocTreeConfigDialog::DocTreeConfigDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Library documentation"));

    QVBoxLayout *layout = new QVBoxLayout(this, 10);

    QLabel *label = new QLabel(i18n("Show the following libraries in the documentation tree:"), this);
    layout->addWidget(label);

    QListView *view = new QListView(this);
    QString path = locate("appdata", "tools/libraries");
    cout << "path " << path << endl;
    KSimpleConfig libconfig(path);
    libconfig.setGroup("Libraries");
    QStringList liblist = libconfig.readListEntry("Entries");
    QStringList::Iterator it;
    for (it = liblist.begin(); it != liblist.end(); ++it)
	{
	    QString title = libconfig.readEntry(*it);
            (void) new QCheckListItem(view, *it, QCheckListItem::CheckBox);
            //	    view->insertItem(new KCheckListItem(view, title, *it));
	    cout << "insert " << (*it) << endl;
	}
   
    readConfig();
    layout->addWidget(view);

    KButtonBox *box = new KButtonBox(this);
    box->addStretch();
    QPushButton *okbutton = box->addButton(i18n("OK"));
    QPushButton *cancelbutton = box->addButton(i18n("Cancel"));
    box->addStretch();
    box->layout();
    layout->addWidget(box);

    connect(okbutton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
}


DocTreeConfigDialog::~DocTreeConfigDialog()
{}


void DocTreeConfigDialog::readConfig()
{  
    KConfig *config = KGlobal::config();
    QMap<QString,QString> entries = config->entryMap("DocTree_KDELibs");
}


void DocTreeConfigDialog::storeConfig()
{
    KConfig *config = KGlobal::config();
}


void DocTreeConfigDialog::accept()
{
    storeConfig();
    QDialog::accept();
}
