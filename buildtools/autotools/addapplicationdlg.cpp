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

#include "addapplicationdlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kicondialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>

#include "autolistviewitems.h"

#include "misc.h"
#include "autoprojectwidget.h"
#include "autoprojectpart.h"


AddApplicationDialog::AddApplicationDialog(AutoProjectWidget *widget, SubprojectItem *spitem,
                                           QWidget *parent, const char *name)
    : AddApplicationDialogBase(parent, name, true)
{
    filename_edit->setText(".desktop");
    filename_edit->home(false);
    filename_edit->setFocus();
    chosentypes_listview->header()->hide();
    availtypes_listview->header()->hide();
    QString addApplication = add_button->text();
    QString removeApplication = remove_button->text();

    add_button->setText( QApplication::reverseLayout() ? removeApplication : addApplication );
    remove_button->setText( QApplication::reverseLayout() ? addApplication : removeApplication );

    m_widget = widget;
    subProject = spitem;

    // Fill the combo box with program names in the directory
    QPtrListIterator<TargetItem> tit(spitem->targets);
    for (; tit.current(); ++tit) {
        if ((*tit)->primary == "PROGRAMS")
            executable_combo->insertItem(QString((*tit)->name));
    }

    // Fill the list of available mime types
    KMimeType::List l = KMimeType::allMimeTypes();
    KMimeType::List::Iterator it;
    for (it = l.begin(); it != l.end(); ++it)
        new QListViewItem(availtypes_listview, (*it)->name());

    setIcon ( SmallIcon ( "window_new" ) );
}


AddApplicationDialog::~AddApplicationDialog()
{}


void AddApplicationDialog::iconClicked()
{
    KIconLoader *loader = AutoProjectFactory::instance()->iconLoader();
    QString name = KIconDialog::getIcon(KIcon::Desktop);
    if (!name.isNull()) {
        iconName = name;
        icon_button->setPixmap(loader->loadIcon(name, KIcon::Desktop));
    }
}


void AddApplicationDialog::addTypeClicked()
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
}


void AddApplicationDialog::removeTypeClicked()
{
    delete chosentypes_listview->currentItem();
}


void AddApplicationDialog::accept()
{
    // Create list of mime types
    QStringList mimeTypes;
    QListViewItem *item = chosentypes_listview->firstChild();
    while (item) {
        mimeTypes.append(item->text(0));
        item = item->nextSibling();
    }

    // Some plausibility tests
    QString fileName = filename_edit->text();
    if (fileName.isEmpty() || fileName == ".desktop") {
        KMessageBox::sorry(this, i18n("You have to enter a file name."));
        filename_edit->setFocus();
        return;
    }

    QString executable = executable_combo->currentText();
    if (executable.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to enter the file name of an executable program."));
        executable_combo->setFocus();
        return;
    }

    QString name = name_edit->text();
    if (name.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to enter an application name."));
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
    stream << "Type=Application" << endl;
    stream << "Name=" << name << endl;
    stream << "Exec=" << (executable + " -caption \"%c\" %i %m %u") << endl;
    stream << "Comment=" << comment_edit->text() << endl;
    if (!iconName.isNull())
        stream << "Icon=" << iconName << endl;
    stream << "MimeTypes=" << mimeTypes.join(";") << endl;
    stream << "Terminal=" << (terminal_box->isChecked()? "true" : "false") << endl;
    f.close();

    // Find a prefix that points to the applnk directory.
    // If there is none, use appslnksection
    QString section = section_combo->currentText();
    QString appsdir = "$(kde_appsdir)/" + section;
    QMap<QString,QString>::ConstIterator it;
    for (it = subProject->prefixes.begin(); it != subProject->prefixes.end(); ++it)
        if (it.data() == appsdir)
            break;

    QMap<QString, QString> replaceMap;
    QString prefix;
    if (it == subProject->prefixes.end()) {
        prefix = "applnk" + section;
        replaceMap.insert(prefix + "dir", appsdir);
        subProject->prefixes.insert(prefix, appsdir);
    } else {
        prefix = it.key();
    }
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
    replaceMap.insert(varname, subProject->variables[varname]);
    AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);

    QDialog::accept();
}

#include "addapplicationdlg.moc"
