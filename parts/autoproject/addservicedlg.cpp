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

#include <qcombobox.h>
#include <qfile.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qtextstream.h>
#include <kdebug.h>
#include <kicondialog.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservicetype.h>

#include "autoprojectwidget.h"
#include "autoprojectfactory.h"
#include "addservicedlg.h"



AddServiceDialog::AddServiceDialog(AutoProjectWidget *widget, SubprojectItem *spitem,
                                   QWidget *parent, const char *name)
    : AddServiceDialogBase(parent, name, true)
{
    filename_edit->setFocus();
    filename_edit->home(false);
    chosentypes_listview->header()->hide();
    availtypes_listview->header()->hide();
    
    m_widget = widget;
    subProject = spitem;

    // Fill the combo box with library names in the directory
    QListIterator<TargetItem> tit(spitem->targets);
    for (; tit.current(); ++tit) {
        if ((*tit)->primary == "LTLIBRARIES")
            library_combo->insertItem(QString((*tit)->name));
    }
    
    // Fill the list of available service types
    KServiceType::List l = KServiceType::allServiceTypes();
    KServiceType::List::Iterator it;
    for (it = l.begin(); it != l.end(); ++it)
        if (!(*it)->isType(KST_KMimeType))
            new QListViewItem(availtypes_listview, (*it)->name());
}


AddServiceDialog::~AddServiceDialog()
{}


void AddServiceDialog::updateProperties()
{
    QStringList props;
    
    QListViewItem *item = static_cast<QCheckListItem*>(chosentypes_listview->firstChild());
    while (item) {
        KServiceType::Ptr type = KServiceType::serviceType(item->text(0));
        if (type) {
            QStringList stprops = type->propertyDefNames();
            QStringList::ConstIterator stit;
            for (stit = stprops.begin(); stit != stprops.end(); ++stit)
                if (props.find(*stit) == props.end() && (*stit) != "Name" && (*stit) != "Comment"
                    && (*stit) != "Icon")
                    props.append(*stit);
        }
        item = item->nextSibling();
    }

    properties_listview->clear();
    QStringList::ConstIterator it;
    for (it = props.begin(); it != props.end(); ++it)
        new QListViewItem(properties_listview, *it);
}


void AddServiceDialog::iconClicked()
{
    KIconLoader *loader = AutoProjectFactory::instance()->iconLoader();
    KIconDialog dlg(loader, this);
    QString name = dlg.selectIcon(0, 0, 0);
    if (!name.isNull()) {
        iconName = name;
        icon_button->setPixmap(loader->loadIcon(name, 0));
    }
}


void AddServiceDialog::addTypeClicked()
{
    QListViewItem *selitem = availtypes_listview->selectedItem();
    if (!selitem)
        return;
    
    QListViewItem *olditem = chosentypes_listview->firstChild();
    while (olditem) {
        if (selitem->text(0) == olditem->text(0))
            return;
        olditem = olditem->nextSibling();
    }
    new QListViewItem(chosentypes_listview, selitem->text(0));

    updateProperties();
}


void AddServiceDialog::removeTypeClicked()
{
    delete chosentypes_listview->currentItem();

    updateProperties();
}


void AddServiceDialog::propertyExecuted(QListViewItem *item)
{
    if (!item)
        return;

    QString prop = item->text(0);
    QString value = item->text(1);
    bool ok;
    value = KLineEditDlg::getText(i18n("Property %1:").arg(prop), value, &ok, this);
    if (!ok)
        return;

    item->setText(1, value);
}


void AddServiceDialog::accept()
{
    QStringList serviceTypes;
    QListViewItem *item = chosentypes_listview->firstChild();
    while (item) {
        serviceTypes.append(item->text(0));
        item = item->nextSibling();
    }
    QFile f(subProject->path + "/" + filename_edit->text());
    if (f.exists()) {
        KMessageBox::sorry(this, i18n("A file with this name exists already."));
        return;
    }
    if (!f.open(IO_WriteOnly)) {
        KMessageBox::sorry(this, i18n("Could not open file for writing."));
        return;
    }
    
    QTextStream stream(&f);
    stream << "[Desktop Entry]" << endl;
    stream << "Type=Service" << endl;
    stream << "Name=" << name_edit->text() << endl;
    stream << "Comment=" << comment_edit->text() << endl;
    if (!iconName.isNull())
        stream << "Icon=" << iconName << endl;
    stream << "ServiceTypes=" << serviceTypes.join(",") << endl;
    item = properties_listview->firstChild();
    while (item) {
        stream << item->text(0) << "=" << item->text(1) << endl;
        item = item->nextSibling();
    }
    f.close();
        
    QDialog::accept();
}

#include "addservicedlg.moc"
