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

#ifndef _FLAGBOXES_H_
#define _FLAGBOXES_H_

#include <qlistview.h>
#include <qcheckbox.h>
#include <qptrlist.h>

class FlagListBox;
class FlagListToolTip;
class FlagCheckBoxController;


class FlagListItem : public QCheckListItem
{
public:
    FlagListItem(FlagListBox *parent, const QString &flagstr,
                 const QString &description);
    FlagListItem(FlagListBox *parent, const QString &flagstr,
                 const QString &description, const QString &offstr);
    ~FlagListItem()
        {}
private:
    QString flag;
    QString off;
    QString desc;
    friend class FlagListToolTip;
    friend class FlagListBox;
};


class FlagListBox : public QListView
{
    Q_OBJECT
public:
    FlagListBox( QWidget *parent=0, const char *name=0 );
    ~FlagListBox()
        {}

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);
};


class FlagCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                 const QString &flagstr, const QString &description);
    FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                 const QString &flagstr, const QString &description,
                 const QString &offstr);
    ~FlagCheckBox()
        {}

private:
    QString flag;
    QString off;
    friend class FlagCheckBoxController;
};


class FlagCheckBoxController
{
public:
    FlagCheckBoxController();
    ~FlagCheckBoxController()
        {}

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);

private:
    void addCheckBox(FlagCheckBox *item);
    QPtrList<FlagCheckBox> cblist;
    friend class FlagCheckBox;
};

#endif
