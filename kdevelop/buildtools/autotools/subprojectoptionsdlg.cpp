/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "subprojectoptionsdlg.h"

#include <qdom.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <keditlistbox.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kservice.h>

#include "domutil.h"
#include "misc.h"
#include "addprefixdlg.h"

#include "autolistviewitems.h"

#include "autoprojectpart.h"
#include "autoprojectwidget.h"


SubprojectOptionsDialog::SubprojectOptionsDialog(AutoProjectPart *part, AutoProjectWidget *widget,
                                                 SubprojectItem *item, QWidget *parent, const char *name)
    : SubprojectOptionsDialogBase(parent, name, true)
{
    setCaption(i18n("Subproject Options for '%1'").arg(item->subdir));

    subProject = item;
    m_part = part;

    QFontMetrics fm(cflags_edit->fontMetrics());
    int wid = fm.width('X')*35;
    cflags_edit->setMinimumWidth(wid);
    cxxflags_edit->setMinimumWidth(wid);
    fflags_edit->setMinimumWidth(wid);

    QDomDocument &dom = *part->projectDom();
    QString prefix = "/kdevautoproject/configurations/" + m_part->currentBuildConfig() + "/";

    ccompiler = DomUtil::readEntry(dom, prefix + "ccompiler", "kdevgccoptions");
    cxxcompiler = DomUtil::readEntry(dom, prefix + "cxxcompiler", "kdevgppoptions");
    f77compiler = DomUtil::readEntry(dom, prefix + "f77compiler", "kdevg77options");

    if (!KService::serviceByDesktopName(ccompiler))
        cflags_button->setEnabled(false);
    if (!KService::serviceByDesktopName(cxxcompiler))
        cxxflags_button->setEnabled(false);
    if (!KService::serviceByDesktopName(f77compiler))
        fflags_button->setEnabled(false);

    insideinc_listview->header()->hide();
    outsideinc_listview->header()->hide();
    buildorder_listview->header()->hide();

    insideinc_listview->setSorting(-1);
    outsideinc_listview->setSorting(-1);
    prefix_listview->setSorting(-1);
    buildorder_listview->setSorting(-1);

    connect( prefix_listview, SIGNAL( doubleClicked ( QListViewItem *, const QPoint &, int ) ), this, SLOT( editPrefixClicked() ) );

    // Insert all subdirectories as possible include directories
    QStringList l = widget->allSubprojects();
    QCheckListItem *lastItem = 0;
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it) {
	kdDebug(9013) << "----------> subproject = " << (*it) << endl;
	QString subProjectName = *it;

	if( subProjectName.isEmpty() ){
	    subProjectName = QString::fromLatin1( "." );
	}
        QCheckListItem *clitem = new QCheckListItem(insideinc_listview, subProjectName, QCheckListItem::CheckBox);
        if (lastItem)
            clitem->moveItem(lastItem);
        lastItem = clitem;
    }

    setIcon ( SmallIcon ( "configure" ) );

    readConfig();
}


SubprojectOptionsDialog::~SubprojectOptionsDialog()
{}


void SubprojectOptionsDialog::readConfig()
{

    cflags_edit->setText(subProject->variables["AM_CFLAGS"]);
    cxxflags_edit->setText(subProject->variables["AM_CXXFLAGS"]);
    fflags_edit->setText(subProject->variables["AM_FFLAGS"]);

    metasources_checkbox->setChecked(subProject->variables["METASOURCES"].stripWhiteSpace() == "AUTO");

    QString includes = subProject->variables["INCLUDES"];
    QStringList includeslist = QStringList::split(QRegExp("[ \t]"), QString(includes));

    QListViewItem *lastItem = 0;
    QStringList::Iterator it;
    for (it = includeslist.begin(); it != includeslist.end(); ++it) {
        QCheckListItem *clitem = static_cast<QCheckListItem*>(insideinc_listview->firstChild());
        while (clitem) {
            if (*it == ("-I$(top_srcdir)/" + clitem->text())) {
                clitem->setOn(true);
                break;
            }
            clitem = static_cast<QCheckListItem*>(clitem->nextSibling());
        }
        if (!clitem) {
            QListViewItem *item = new QListViewItem(outsideinc_listview, *it);
            if (lastItem)
                item->moveItem(lastItem);
            lastItem = item;
        }
    }

    QMap<QString, QString>::ConstIterator it2;
    for (it2 = subProject->prefixes.begin(); it2 != subProject->prefixes.end(); ++it2)
        new QListViewItem(prefix_listview, it2.key(), it2.data());

    QString subdirs = subProject->variables["SUBDIRS"];
    kdDebug(9020) << "Subdirs variable: " << subdirs << endl;
    QStringList subdirslist = QStringList::split(QRegExp("[ \t]"), QString(subdirs));
    lastItem = 0;
    for (it = subdirslist.begin(); it != subdirslist.end(); ++it) {
        QListViewItem *item = new QListViewItem(buildorder_listview, *it);
        if (lastItem)
            item->moveItem(lastItem);
        lastItem = item;
    }
}


void SubprojectOptionsDialog::storeConfig()
{
    QMap<QString, QString> replaceMap;

    QString old_cflags = subProject->variables["AM_CFLAGS"];
    QString new_cflags = cflags_edit->text();
    if (new_cflags != old_cflags) {
        subProject->variables["AM_CFLAGS"] = new_cflags;
        replaceMap.insert("AM_CFLAGS", new_cflags);
    }

    QString old_cxxflags = subProject->variables["AM_CXXFLAGS"];
    QString new_cxxflags = cxxflags_edit->text();
    if (new_cxxflags != old_cxxflags) {
        subProject->variables["AM_CXXFLAGS"] = new_cxxflags;
        replaceMap.insert("AM_CXXFLAGS", new_cxxflags);
    }

    QString old_fflags = subProject->variables["AM_FFLAGS"];
    QString new_fflags = fflags_edit->text();
    if (new_fflags != old_fflags) {
        subProject->variables["AM_FFLAGS"] = new_fflags;
        replaceMap.insert("AM_FFLAGS", new_fflags);
    }

    QString old_metasources = subProject->variables["METASOURCES"];
    QString new_metasources = metasources_checkbox->isChecked() ? QString::fromLatin1("AUTO") : QString::null;
    if (new_metasources != old_metasources) {
        subProject->variables["METASOURCES"] = new_metasources;
        replaceMap.insert("METASOURCES", new_metasources);
    }

    QStringList includeslist;
    QCheckListItem *clitem = static_cast<QCheckListItem*>(insideinc_listview->firstChild());
    while (clitem) {
        if (clitem->isOn())
            includeslist.append("-I$(top_srcdir)/" + clitem->text());
        clitem = static_cast<QCheckListItem*>(clitem->nextSibling());
    }
    clitem = static_cast<QCheckListItem*>(outsideinc_listview->firstChild());
    while (clitem) {
        includeslist.append(clitem->text());
        clitem = static_cast<QCheckListItem*>(clitem->nextSibling());
    }
    QString includes = includeslist.join(" ");
    subProject->variables["INCLUDES"] = includes;
    replaceMap.insert("INCLUDES", includes);

    subProject->prefixes.clear();
    for (QListViewItem *item = prefix_listview->firstChild();
         item; item = item->nextSibling()) {
        QString key = item->text(0);
        QString data = item->text(1);
        subProject->prefixes[key] = data;
        replaceMap.insert(key + "dir", data);
    }
    /// \FIXME take removed prefixes into account

    QStringList subdirslist;
    for (QListViewItem *item = buildorder_listview->firstChild();
         item; item = item->nextSibling())
        subdirslist.append(item->text(0));
    QString subdirs = subdirslist.join(" ");
    kdDebug() << "New subdirs variable: " << subdirs << endl;
    subProject->variables["SUBDIRS"] = subdirs;
    replaceMap.insert("SUBDIRS", subdirs);

    AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);
}


void SubprojectOptionsDialog::cflagsClicked()
{
    QString new_cflags = AutoProjectTool::execFlagsDialog(ccompiler, cflags_edit->text(), this);
//    if (!new_cflags.isNull())
        cflags_edit->setText(new_cflags);
}


void SubprojectOptionsDialog::cxxFlagsClicked()
{
    QString new_cxxflags = AutoProjectTool::execFlagsDialog(cxxcompiler, cxxflags_edit->text(), this);
//    if (!new_cxxflags.isNull())
        cxxflags_edit->setText(new_cxxflags);
}


void SubprojectOptionsDialog::fflagsClicked()
{
    QString new_fflags = AutoProjectTool::execFlagsDialog(f77compiler, fflags_edit->text(), this);
//    if (!new_fflags.isNull())
        fflags_edit->setText(new_fflags);
}


void SubprojectOptionsDialog::insideMoveUpClicked()
{
    if (insideinc_listview->currentItem() == insideinc_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = insideinc_listview->firstChild();
    while (item->nextSibling() != insideinc_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(insideinc_listview->currentItem());
}


void SubprojectOptionsDialog::insideMoveDownClicked()
{
   if (insideinc_listview->currentItem() == 0 || insideinc_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   insideinc_listview->currentItem()->moveItem(insideinc_listview->currentItem()->nextSibling());
}


void SubprojectOptionsDialog::outsideMoveUpClicked()
{
    if (outsideinc_listview->currentItem() == outsideinc_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = outsideinc_listview->firstChild();
    while (item->nextSibling() != outsideinc_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(outsideinc_listview->currentItem());
}


void SubprojectOptionsDialog::outsideMoveDownClicked()
{
   if (outsideinc_listview->currentItem() == 0 || outsideinc_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   outsideinc_listview->currentItem()->moveItem(outsideinc_listview->currentItem()->nextSibling());
}


void SubprojectOptionsDialog::outsideAddClicked()
{
    bool ok;
    QString dir = KInputDialog::getText(i18n("Add Include Directory"), i18n("Add include directory:"), "-I", &ok, 0);
    if (ok && !dir.isEmpty() && dir != "-I")
        new QListViewItem(outsideinc_listview, dir);
}


void SubprojectOptionsDialog::outsideEditClicked()
{
    if ( (outsideinc_listview->childCount()==0) || (outsideinc_listview->currentItem() == 0) )
        return;
    bool ok;
    QString dir = KInputDialog::getText(i18n("Edit Include Directory"), i18n("Edit include directory:"),
            outsideinc_listview->currentItem()-> text(0), &ok, 0);
    if (ok && !dir.isEmpty())
        outsideinc_listview->currentItem()-> setText(0, dir);
}


void SubprojectOptionsDialog::outsideRemoveClicked()
{
    delete outsideinc_listview->currentItem();
}


void SubprojectOptionsDialog::addPrefixClicked()
{
    AddPrefixDialog dlg;
    if (!dlg.exec() || dlg.name().isEmpty() || dlg.path().isEmpty() )
        return;

    new QListViewItem(prefix_listview, dlg.name(), dlg.path());
}


void SubprojectOptionsDialog::editPrefixClicked()
{
    QListViewItem* lvItem = prefix_listview->currentItem();
    if ( (prefix_listview->childCount()==0) || (lvItem == 0) )
        return;
    AddPrefixDialog dlg(lvItem-> text(0), lvItem-> text(1));
    dlg.setCaption(i18n("Edit Prefix"));
    if (!dlg.exec() || dlg.name().isEmpty() || dlg.path().isEmpty() )
        return;
   lvItem-> setText(0, dlg.name());
   lvItem-> setText(1, dlg.path());
}

void SubprojectOptionsDialog::removePrefixClicked()
{
    delete prefix_listview->currentItem();
}


void SubprojectOptionsDialog::buildorderMoveUpClicked()
{
    if (buildorder_listview->currentItem() == buildorder_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = buildorder_listview->firstChild();
    while (item->nextSibling() != buildorder_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(buildorder_listview->currentItem());
}


void SubprojectOptionsDialog::buildorderMoveDownClicked()
{
   if (buildorder_listview->currentItem() == 0 || buildorder_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   buildorder_listview->currentItem()->moveItem(buildorder_listview->currentItem()->nextSibling());
}


void SubprojectOptionsDialog::accept()
{
    storeConfig();
    QDialog::accept();
}

#include "subprojectoptionsdlg.moc"
