/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _FLAGBOXES_H_
#define _FLAGBOXES_H_

#include <kfile.h>

/**
@file flagboxes.h
Support classes for compiler plugins.
*/

#include <q3listview.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <q3ptrlist.h>
#include <qstringlist.h>

class QSpinBox;
class FlagListBox;
class FlagListToolTip;
class FlagCheckBoxController;
class FlagRadioButtonController;
class FlagPathEditController;
class FlagEditController;
class KLineEdit;
class QPushButton;
class KURLRequester;

/**List item holding a compiler flag.*/
class FlagListItem : public Q3CheckListItem
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


/**List box item holding a compiler flag.*/
class FlagListBox : public Q3ListView
{
    Q_OBJECT
public:
    FlagListBox( QWidget *parent=0, const char *name=0 );
    ~FlagListBox()
        {}

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);
};


/**Check box item holding a compiler flag.*/
class FlagCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                 const QString &flagstr, const QString &description);
    FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                 const QString &flagstr, const QString &description,
                 const QString &offstr);
    FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                 const QString &flagstr, const QString &description,
                 const QString &offstr, const QString &defstr);
    ~FlagCheckBox()
        {}

private:
    QString flag;
    QString off;
    QString def;
    bool includeOff;
    bool useDef;
    bool defSet;
    friend class FlagCheckBoxController;
};

/**Radiobutton item holding an option of a compiler flag.*/
class FlagRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    FlagRadioButton(QWidget *parent, FlagRadioButtonController *controller,
                 const QString &flagstr, const QString &description);
    ~FlagRadioButton()
        {}

private:
    QString flag;
    friend class FlagRadioButtonController;
};

/**Path editor if path is a compiler flag.*/
class FlagPathEdit: public QWidget
{
    Q_OBJECT
public:
    /**If the pathDelimiter is not empty then path edit can contain a list of paths*/
    FlagPathEdit(QWidget *parent, QString pathDelimiter, FlagPathEditController *controller,
                 const QString &flagstr, const QString &description, KFile::Mode mode = KFile::Directory);

    ~FlagPathEdit() {}

    void setText(const QString text);
    bool isEmpty();
    QString text();

private slots:
    void showPathDetails();

private:
    KLineEdit *edit;
    QPushButton *details;
    KURLRequester *url;

    QString delimiter;
    QString flag;
    QString m_description;
    friend class FlagPathEditController;
};

/**List editor if list is a compiler flag.*/
class FlagListEdit: public QWidget
{
    Q_OBJECT
public:
    /**If the listDelimiter is not empty then list edit can contain a list of entries*/
    FlagListEdit(QWidget *parent, QString listDelimiter, FlagEditController *controller,
                 const QString &flagstr, const QString &description);

    ~FlagListEdit() {}

    void setText(const QString text);
    void appendText(const QString text);
    bool isEmpty();
    QString text();
    QStringList flags();

private slots:
    void showListDetails();

private:
    KLineEdit *edit;
    QPushButton *details;

    QString delimiter;
    QString flag;
    QString m_description;
    friend class FlagEditController;
};

/**Spin editor of a compiler flag.*/
class FlagSpinEdit: public QWidget
{
public:
    FlagSpinEdit(QWidget *parent, int minVal, int maxVal, int incr, int defaultVal, FlagEditController *controller,
                 const QString &flagstr, const QString &description);
    ~FlagSpinEdit() {}

    void setText(const QString text);
    QString text();
    bool isDefault();

    QString flags();

private:
    int m_defaultVal;
    QString flag;

    QSpinBox *spb;

    friend class FlagEditController;
};

/**Controller for path editors.*/
class FlagPathEditController
{
public:
    FlagPathEditController();
    ~FlagPathEditController();

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);

private:
    void addPathEdit(FlagPathEdit *item);
    Q3PtrList<FlagPathEdit> plist;
    friend class FlagPathEdit;
};

/**Controller for flag editors.*/
class FlagEditController
{
public:
    FlagEditController();
    ~FlagEditController();

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);

private:
    void addListEdit(FlagListEdit *item);
    void addSpinBox(FlagSpinEdit *item);
    Q3PtrList<FlagListEdit> plist;
    Q3PtrList<FlagSpinEdit>  slist;
    friend class FlagListEdit;
    friend class FlagSpinEdit;
};

/**Controller for check box editors.*/
class FlagCheckBoxController
{
public:
    /**"multi key" is a list of options like -vxyz (-vx -vy -vz)
       multiKeys must contain a list of option names like {-v}
       in the above example.
    */
    FlagCheckBoxController(QStringList multiKeys = QStringList());
    ~FlagCheckBoxController()
        {}

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);

    void addCheckBox(FlagCheckBox *item);
private:
    Q3PtrList<FlagCheckBox> cblist;

    QStringList m_multiKeys;
};

/**Controller for radiobutton editors.*/
class FlagRadioButtonController
{
public:
    /**multiKeys is a list of options like -vxyz (-vx -vy -vz)
       multiKeys must contain a list of option names like {-v}
       in the above example.
    */
    FlagRadioButtonController(QStringList multiKeys = QStringList());
    ~FlagRadioButtonController()
        {}

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);

    void addRadioButton(FlagRadioButton *item);
private:
    Q3PtrList<FlagRadioButton> cblist;

    QStringList m_multiKeys;
};

#endif
