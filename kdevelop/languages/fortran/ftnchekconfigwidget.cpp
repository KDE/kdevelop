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

#include "ftnchekconfigwidget.h"

#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qheader.h>
#include <qlistview.h>
#include <qradiobutton.h>
#include <qtooltip.h>
#include <klocale.h>

#include "domutil.h"


class FtnchekItem : public QCheckListItem
{
public:
    FtnchekItem(QListView *parent, const QString &flagstr,
                const QString &description)
        : QCheckListItem(parent, flagstr, QCheckListItem::CheckBox),
          flag(flagstr), desc(description)
    {
        setText(1, desc);
    }

    static void readFlagsToListView(QListView *listview, QStringList *list);
    static void writeFlagsFromListView(QListView *listview, QStringList *list);

private:
    QString flag;
    QString desc;
    friend class FtnchekToolTip;
};


void FtnchekItem::readFlagsToListView(QListView *listview, QStringList *list)
{
    QListViewItem *item = listview->firstChild();
    for (; item; item = item->nextSibling()) {
        FtnchekItem *flitem = static_cast<FtnchekItem*>(item);
        QStringList::Iterator sli = list->find(flitem->flag);
        if (sli != list->end()) {
            flitem->setOn(true);
            list->remove(sli);
        }
    }
}


void FtnchekItem::writeFlagsFromListView(QListView *listview, QStringList *list)
{
    (*list).clear();

    QListViewItem *item = listview->firstChild();
    for (; item; item = item->nextSibling()) {
        FtnchekItem *flitem = static_cast<FtnchekItem*>(item);
        if (flitem->isOn())
            (*list) << flitem->flag;
    }
}


class FtnchekToolTip : public QToolTip
{
public:
    FtnchekToolTip(QWidget *parent)
        : QToolTip(parent)
    {}
protected:
    void maybeTip(const QPoint &pos)
    {
        QListView *listview = static_cast<QListView*>(parentWidget());
        QListViewItem *item = listview->itemAt(pos);
        FtnchekItem *flitem = static_cast<FtnchekItem*>(item);

        if (item)
            tip(listview->itemRect(item), flitem->desc);
    }
};


const char *arguments_flags[] = {
    "arrayness",     I18N_NOOP("Warn about inconsistent use of arguments that use arrays"),
    "type",          I18N_NOOP("Warn about dummy arguments of a data type different from "
                               "the actual arguments"),
    "function-type", I18N_NOOP("Warn if the invocation assumes a different data type for the return type, "
                               "different from the actual return type"),
    "number",        I18N_NOOP("Warn about invoking a subprogram with an incorrect number of arguments"),
    0,               0
};


const char *common_flags[] = {
    "dimension", I18N_NOOP("Corresponding arrays in each declaration of a block must agree in size "
                           "and number of dimensions"),
    "exact",     I18N_NOOP("The comparison of two blocks is done variable-by-variable"),
    "length",    I18N_NOOP("Warn if different declarations of the same block are not equal in total length"),
    "type",      I18N_NOOP("In each declaration of a block, corresponding memory locations "
                           "must agree in data type"),
    0,           0
};


const char *truncation_flags[] = {
    "int-div-exponent", I18N_NOOP("Use of the result of integer division as an exponent"),
    "int-div-real",     I18N_NOOP("Conversion of an expression involving an integer division to real"),
    "int-div-zero",     I18N_NOOP("Division in an integer constant expression that yields a result of zero"),
    0,                  0
};


const char *usage_flags[] = {
    "arg-alias",        I18N_NOOP("A scalar dummy argument is actually the same as another "
                                  "and is (or may be) modified"),
    "arg-array-alias",  I18N_NOOP("A dummy argument which is an array or array element "
                                  "is the same array as another and is modified"),
    "arg-common-alias", I18N_NOOP("A scalar dummy argument is the same as a common variable in "
                                  "the subprogram, and either is modified"),
    0,                  0
};


const char *f77_flags[] = {
    "accept-type",     I18N_NOOP("ACCEPT and TYPE I/O statements"),
    "array-bounds",    I18N_NOOP("Expressions defining array bounds that contain array "
                                 "elements or function references"),
    "assignment-stmt", I18N_NOOP("Assignment statements involving arrays"),
    0,                 0
};


const char *portability_flags[] = {
    "backslash",        I18N_NOOP("Backslash characters in strings"),
    "common-alignment", I18N_NOOP("COMMON block variables not in descending order of storage sizes"),
    "hollerith",        I18N_NOOP("Hollerith constants"),
    0,                  0
};


FtnchekConfigWidget::FtnchekConfigWidget(QDomDocument &projectDom, QWidget *parent, const char *name)
    : FtnchekConfigWidgetBase(parent, name), dom(projectDom)
{
    arguments_group = new QButtonGroup;
    arguments_group->insert(argumentsall_button);
    arguments_group->insert(argumentsonly_button);
    common_group = new QButtonGroup;
    common_group->insert(commonall_button);
    common_group->insert(commononly_button);
    truncation_group = new QButtonGroup;
    truncation_group->insert(truncationall_button);
    truncation_group->insert(truncationonly_button);
    usage_group = new QButtonGroup;
    usage_group->insert(usageall_button);
    usage_group->insert(usageonly_button);
    f77_group = new QButtonGroup;
    f77_group->insert(f77all_button);
    f77_group->insert(f77only_button);
    portability_group = new QButtonGroup;
    portability_group->insert(portabilityall_button);
    portability_group->insert(portabilityonly_button);

    arguments_listview->header()->hide();
    new FtnchekToolTip(arguments_listview);

    common_listview->header()->hide();
    new FtnchekToolTip(common_listview);

    truncation_listview->header()->hide();
    new FtnchekToolTip(truncation_listview);

    usage_listview->header()->hide();
    new FtnchekToolTip(usage_listview);

    f77_listview->header()->hide();
    new FtnchekToolTip(f77_listview);

    portability_listview->header()->hide();
    new FtnchekToolTip(portability_listview);

    for (const char **p = arguments_flags; *p; p += 2)
        new FtnchekItem(arguments_listview, QString::fromUtf8(*p), i18n(*(p+1)));

    for (const char **p = common_flags; *p; p += 2)
        new FtnchekItem(common_listview, QString::fromUtf8(*p), i18n(*(p+1)));

    for (const char **p = truncation_flags; *p; p += 2)
        new FtnchekItem(truncation_listview, QString::fromUtf8(*p), i18n(*(p+1)));

    for (const char **p = usage_flags; *p; p += 2)
        new FtnchekItem(usage_listview, QString::fromUtf8(*p), i18n(*(p+1)));

    for (const char **p = f77_flags; *p; p += 2)
        new FtnchekItem(f77_listview, QString::fromUtf8(*p), i18n(*(p+1)));

    for (const char **p = portability_flags; *p; p += 2)
        new FtnchekItem(portability_listview, QString::fromUtf8(*p), i18n(*(p+1)));

    readConfig();
}


FtnchekConfigWidget::~FtnchekConfigWidget()
{
    delete arguments_group;
    delete common_group;
    delete truncation_group;
    delete usage_group;
    delete f77_group;
    delete portability_group;
}


void FtnchekConfigWidget::accept()
{
    storeConfig();
}


void FtnchekConfigWidget::readConfig()
{
    division_box->setChecked(DomUtil::readBoolEntry(dom,          "/kdevfortransupport/ftnchek/division"));
    extern_box->setChecked(DomUtil::readBoolEntry(dom,            "/kdevfortransupport/ftnchek/extern"));
    declare_box->setChecked(DomUtil::readBoolEntry(dom,           "/kdevfortransupport/ftnchek/declare"));
    pure_box->setChecked(DomUtil::readBoolEntry(dom,              "/kdevfortransupport/ftnchek/pure"));

    argumentsall_button->setChecked(DomUtil::readBoolEntry(dom,   "/kdevfortransupport/ftnchek/argumentsall"));
    commonall_button->setChecked(DomUtil::readBoolEntry(dom,      "/kdevfortransupport/ftnchek/commonall"));
    truncationall_button->setChecked(DomUtil::readBoolEntry(dom,  "/kdevfortransupport/ftnchek/truncationall"));
    usageall_button->setChecked(DomUtil::readBoolEntry(dom,       "/kdevfortransupport/ftnchek/usageall"));
    f77all_button->setChecked(DomUtil::readBoolEntry(dom,         "/kdevfortransupport/ftnchek/f77all"));
    portabilityall_button->setChecked(DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/portabilityall"));

    QStringList list;

    list = QStringList::split(',', DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/argumentsonly"));
    FtnchekItem::readFlagsToListView(arguments_listview, &list);
    list = QStringList::split(',', DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/commononly"));
    FtnchekItem::readFlagsToListView(common_listview, &list);
    list = QStringList::split(',', DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/truncationonly"));
    FtnchekItem::readFlagsToListView(truncation_listview, &list);
    list = QStringList::split(',', DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/usageonly"));
    FtnchekItem::readFlagsToListView(usage_listview, &list);
    list = QStringList::split(',', DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/f77only"));
    FtnchekItem::readFlagsToListView(f77_listview, &list);
    list = QStringList::split(',', DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/portabilityonly"));
    FtnchekItem::readFlagsToListView(portability_listview, &list);
}


void FtnchekConfigWidget::storeConfig()
{
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/division",       division_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/extern",         extern_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/declare",        declare_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/pure",           pure_box->isChecked());

    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/argumentsall",   argumentsall_button->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/commonall",      commonall_button->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/truncationall",  truncationall_button->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/usageall",       usageall_button->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/f77all",         f77all_button->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevfortransupport/ftnchek/portabilityall", portabilityall_button->isChecked());

    QStringList list;

    FtnchekItem::writeFlagsFromListView(arguments_listview, &list);
    DomUtil::writeEntry(dom, "/kdevfortransupport/ftnchek/argumentsonly",   list.join(","));
    FtnchekItem::writeFlagsFromListView(common_listview, &list);
    DomUtil::writeEntry(dom, "/kdevfortransupport/ftnchek/commononly",      list.join(","));
    FtnchekItem::writeFlagsFromListView(truncation_listview, &list);
    DomUtil::writeEntry(dom, "/kdevfortransupport/ftnchek/truncationonly",  list.join(","));
    FtnchekItem::writeFlagsFromListView(usage_listview, &list);
    DomUtil::writeEntry(dom, "/kdevfortransupport/ftnchek/usageonly",       list.join(","));
    FtnchekItem::writeFlagsFromListView(f77_listview, &list);
    DomUtil::writeEntry(dom, "/kdevfortransupport/ftnchek/f77only",         list.join(","));
    FtnchekItem::writeFlagsFromListView(portability_listview, &list);
    DomUtil::writeEntry(dom, "/kdevfortransupport/ftnchek/portabilityonly", list.join(","));
}

#include "ftnchekconfigwidget.moc"
