/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtooltip.h>
#include <qheader.h>
#include <qstringlist.h>
#include <klocale.h>

#include "flagboxes.h"


class FlagListToolTip : public QToolTip
{
public:
    FlagListToolTip(QWidget *parent);
protected:
    void maybeTip(const QPoint &p);
};


FlagListToolTip::FlagListToolTip(QWidget *parent)
    : QToolTip(parent)
{}


void FlagListToolTip::maybeTip(const QPoint &pos)
{
    FlagListBox *listbox = static_cast<FlagListBox*>(parentWidget());
    QListViewItem *item = listbox->itemAt(pos);
    FlagListItem *flitem = static_cast<FlagListItem*>(item);

    if (item)
        tip(listbox->itemRect(item), flitem->desc);
}


FlagListItem::FlagListItem(FlagListBox *parent, const QString &flagstr,
                           const QString &description)
    : QCheckListItem(parent, flagstr, QCheckListItem::CheckBox),
      flag(flagstr), desc(description)
{}


FlagListItem::FlagListItem(FlagListBox *parent, const QString &flagstr,
                           const QString &description, const QString &offstr)
    : QCheckListItem(parent, flagstr, QCheckListItem::CheckBox),
      flag(flagstr), off(offstr), desc(description)
{}


FlagListBox::FlagListBox(QWidget *parent, const char *name)
    : QListView(parent, name)
{
    setResizeMode(LastColumn);
    header()->hide();
    addColumn(i18n("Flags"));
    (void) new FlagListToolTip(this);
}


void FlagListBox::readFlags(QStringList *list)
{
    QListViewItem *item = firstChild();
    for (; item; item = item->nextSibling()) {
        FlagListItem *flitem = static_cast<FlagListItem*>(item);
        QStringList::Iterator sli = list->find(flitem->flag);
        if (sli != list->end()) {
            flitem->setOn(true);
            list->remove(sli);
        }
        sli = list->find(flitem->off);
        if (sli != list->end()) {
            flitem->setOn(false);
            list->remove(sli);
        }
    }
}


void FlagListBox::writeFlags(QStringList *list)
{
    QListViewItem *item = firstChild();
    for (; item; item = item->nextSibling()) {
        FlagListItem *flitem = static_cast<FlagListItem*>(item);
        if (flitem->isOn())
            (*list) << flitem->flag;
    }
}


FlagCheckBox::FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                           const QString &flagstr, const QString &description)
    : QCheckBox(description, parent), flag(flagstr)
{
    QToolTip::add(this, flagstr);
    controller->addCheckBox(this);
}


FlagCheckBox::FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                           const QString &flagstr, const QString &description,
                           const QString &offstr)
    : QCheckBox(description, parent), flag(flagstr), off(offstr)
{
    QToolTip::add(this, flagstr);
    controller->addCheckBox(this);
}


FlagCheckBoxController::FlagCheckBoxController()
{}


void FlagCheckBoxController::addCheckBox(FlagCheckBox *item)
{
    cblist.append(item);
}


void FlagCheckBoxController::readFlags(QStringList *list)
{
    QListIterator<FlagCheckBox> it(cblist);
    for (; it.current(); ++it) {
        FlagCheckBox *fitem = it.current();
        QStringList::Iterator sli = list->find(fitem->flag);
        if (sli != list->end()) {
            fitem->setChecked(true);
            list->remove(sli);
        }
        sli = list->find(fitem->off);
        if (sli != list->end()) {
            fitem->setChecked(false);
            list->remove(sli);
        }
    }
}


void FlagCheckBoxController::writeFlags(QStringList *list)
{
    QListIterator<FlagCheckBox> it(cblist);
    for (; it.current(); ++it) {
        FlagCheckBox *fitem = it.current();
        if (fitem->isChecked())
            (*list) << fitem->flag;
    }
}
    

#include "flagboxes.moc"
