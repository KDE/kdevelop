/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FLAGBOXES_H_
#define _FLAGBOXES_H_

#include <kfile.h>

#include <qlistview.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qptrlist.h>

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

class FlagPathEditController
{
public:
    FlagPathEditController();
    ~FlagPathEditController();

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);

private:
    void addPathEdit(FlagPathEdit *item);
    QPtrList<FlagPathEdit> plist;
    friend class FlagPathEdit;
};

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
    QPtrList<FlagListEdit> plist;
    QPtrList<FlagSpinEdit>  slist;
    friend class FlagListEdit;
    friend class FlagSpinEdit;
};

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
    QPtrList<FlagCheckBox> cblist;

    QStringList m_multiKeys;
};

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
    QPtrList<FlagRadioButton> cblist;

    QStringList m_multiKeys;
};

#endif
