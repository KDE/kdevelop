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

#include "addservicedlg.h"

#include <qcombobox.h>
#include <qfile.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qtextstream.h>
#include <kdebug.h>
#include <kicondialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservicetype.h>

#include "autolistviewitems.h"

#include "misc.h"
#include "autoprojectwidget.h"
#include "autoprojectpart.h"


AddServiceDialog::AddServiceDialog(AutoProjectWidget *widget, SubprojectItem *spitem,
                                   QWidget *parent, const char *name)
    : AddServiceDialogBase(parent, name, true)
{
    filename_edit->setText(".desktop");
    filename_edit->home(false);
    filename_edit->setFocus();
    chosentypes_listview->header()->hide();
    availtypes_listview->header()->hide();
    
    m_widget = widget;
    subProject = spitem;

    // Fill the combo box with library names in the directory
    QPtrListIterator<TargetItem> tit(spitem->targets);
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

    setIcon ( SmallIcon ( "servicenew_kdevelop.png" ) );
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
    QString name = KIconDialog::getIcon(KIcon::Desktop);
    if (!name.isNull()) {
        iconName = name;
        icon_button->setPixmap(loader->loadIcon(name, KIcon::Desktop));
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
    value = KInputDialog::getText(i18n("Enter Value"), i18n("Property %1:").arg(prop), value, &ok, this);
    if (!ok)
        return;

    item->setText(1, value);
}


void AddServiceDialog::accept()
{
    // Create list of service types
    QStringList serviceTypes;
    QListViewItem *item = chosentypes_listview->firstChild();
    while (item) {
        serviceTypes.append(item->text(0));
        item = item->nextSibling();
    }

    // Some plausibility tests
    QString fileName = filename_edit->text();
    if (fileName.isEmpty() || fileName == ".desktop") {
        KMessageBox::sorry(this, i18n("You have to enter a file name."));
        filename_edit->setFocus();
        return;
    }
        
    QString name = name_edit->text();
    if (name.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to enter a service name."));
        name_edit->setFocus();
        return;
    }

    QFile f(subProject->path + "/" + fileName);
    if (f.exists()) {
        KMessageBox::sorry(this, i18n("A file with this name exists already."));
        filename_edit->setFocus();
        return;
    }
    if (!f.open(IO_WriteOnly)) {
        KMessageBox::sorry(this, i18n("Could not open file for writing."));
        return;
    }
    
    QTextStream stream(&f);
    stream << "[Desktop Entry]" << endl;
    stream << "Type=Service" << endl;
    stream << "Name=" << name << endl;
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

    // Find a prefix that points to the services directory.
    // If there is none, use kde_services
    QMap<QString,QString>::ConstIterator it;
    for (it = subProject->prefixes.begin(); it != subProject->prefixes.end(); ++it)
        if (it.data() == "$(kde_servicesdir)")
            break;
    QString prefix = (it == subProject->prefixes.end())? QString("kde_services") : it.key();
    QString varname = prefix + "_DATA";

    // Look if a list view item for this prefix exists already.
    // Create a new one otherwise
    TargetItem *titem = 0;
    for (uint i=0; i < subProject->targets.count(); ++i) {
        TargetItem *tmptitem = subProject->targets.at(i);
        if ("DATA" == tmptitem->primary && prefix == tmptitem->prefix) {
            titem = tmptitem;
            break;
        }
    }
    if (!titem) {
        titem = m_widget->createTargetItem("", prefix, "DATA", false);
        subProject->targets.append(titem);
    }
    // Add this file to the target
    FileItem *fitem = m_widget->createFileItem(fileName, subProject);
    titem->sources.append(fitem);
    
    subProject->variables[varname] += (" " + fileName);
    QMap<QString, QString> replaceMap;
    replaceMap.insert(varname, subProject->variables[varname]);
    AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);
    
    QDialog::accept();
}

#include "addservicedlg.moc"
