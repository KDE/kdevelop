/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLASSACTION_H_
#define _CLASSACTION_H_

#include <kaction.h>

class CClassStore;


class ClassListAction : public KSelectAction
{
public:
    ClassListAction(const QString &text, int accel,
                    const QObject *receiver, const char *methodname,
                    QObject *parent, const char *name);
    void setClassStore(CClassStore *store);
    void setCurrentItem(const QString &item);
    void refresh();
    
private:
    CClassStore *m_store;
};


class MethodListAction : public KSelectAction
{
public:
    MethodListAction(const QString &text, int accel,
                     const QObject *receiver, const char *methodname,
                     QObject *parent, const char *name);
    void setClassStore(CClassStore *store);
    void refresh(const QString &className);

private:
    CClassStore *m_store;
};


class DelayedPopupAction : public KAction
{
    Q_OBJECT

public:
    DelayedPopupAction( const QString &text, const QString &pix, int accel,
                            QObject *receiver, const char *methname,
                            QObject *parent, const char* name );
    ~DelayedPopupAction();
    
    virtual int plug(QWidget *widget, int index=-1);
    virtual void unplug(QWidget *widget);

    QPopupMenu *popupMenu();

private:
    QPopupMenu *m_popup;
};

#endif
