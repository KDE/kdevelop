/***************************************************************************
 *   Copyright (C) 1997-2000 by Dimitri van Heesch                         *
 *   dimitri@stack.nl                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _INPUT_H
#define _INPUT_H
 
#include <qcheckbox.h>
#include <q3dict.h>
//Added by qt3to4:
#include <Q3StrList>
#include <QLabel>
#include <Q3CString>

class QComboBox;
class QLabel;
class QLineEdit;
class Q3ListBox;
class QPushButton;
class QSpinBox;
 
class IInput
{
  public:
    virtual void init() = 0;
    virtual void setEnabled(bool) = 0;
    virtual QObject *qobject() = 0;
};


class InputBool : public QWidget, public IInput
{
    Q_OBJECT

public:
    InputBool(const Q3CString &key, const QString &text, QWidget *parent, bool &flag);
    ~InputBool();
    
    void init();
    virtual void setEnabled(bool b);
    virtual QObject *qobject() { return this; }
    virtual bool getState() const { return state; }

signals:
    void changed();
    void toggle(const QString &, bool);

private slots:
    void valueChanged(bool);

private:
    bool &state;
    Q3CString key;
    QCheckBox *cb;
};


class InputInt : public QWidget, public IInput
{
    Q_OBJECT

public:
    InputInt(const QString &text, QWidget *parent,
             int &val, int minVal, int maxVal);
    ~InputInt();
    
    void init();
    virtual void setEnabled(bool);
    QObject *qobject() { return this; }

signals:
    void changed();

private slots:
    void valueChanged(int val); 

private:
    QLabel *lab;
    QSpinBox *sp;
    int &m_val;
    int m_minVal;
    int m_maxVal;
};


class InputString : public QWidget, public IInput
{
    Q_OBJECT

public:
    enum StringMode { StringFree=0, 
                      StringFile=1, 
                      StringDir=2, 
                      StringFixed=3
                    };

    InputString(const QString &text, QWidget *parent,
                Q3CString &s, StringMode m=StringFree);
    ~InputString();
    
    void init();
    void addValue(const char *s);
    void setEnabled(bool);
    QObject *qobject() { return this; }

signals:
    void changed();

private slots:
    void textChanged(const QString&);
    void browse();
    void clear();

private:
    QLabel *lab;
    QLineEdit *le;
    QPushButton *br;
    QComboBox *com;
    Q3CString &str;
    StringMode sm;
    Q3Dict<int> *m_values;
    int m_index; 
};


class InputStrList : public QWidget, public IInput
{
    Q_OBJECT

public:
    enum ListMode { ListString=0, 
                    ListFile=1, 
                    ListDir=2, 
                    ListFileDir=ListFile|ListDir 
                  };
    
    InputStrList(const QString &text, QWidget *parent, 
                 Q3StrList &sl, ListMode v=ListString);
    ~InputStrList();

    void init();
    void setEnabled(bool);
    QObject *qobject() { return this; }

signals:
    void changed();

private slots:
    void addString(); 
    void delString(); 
    void updateString(); 
    void selectText(const QString &s);
    void browseFiles();
    void browseDir();

private:
    QLabel *lab;
    QLineEdit *le;
    QPushButton *add;
    QPushButton *del;
    QPushButton *upd;
    QPushButton *brFile;
    QPushButton *brDir;
    Q3ListBox *lb;
    Q3StrList &strList;
};

#endif
