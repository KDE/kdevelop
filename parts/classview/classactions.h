/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
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

class KDevPlugin;


class ClassListAction : public KSelectAction
{
public:
    ClassListAction( KDevPlugin *part, const QString &text, int accel,
                     const QObject *receiver, const char *slot,
                     QObject *parent, const char *name );
    void setCurrentClassName(const QString &name);
    QString currentClassName();
    void refresh();
    
private:
    QString currentText(); // only here for preventing its public use
    void setCurrentItem(const QString &item);
    KDevPlugin *m_part;
};


class MethodListAction : public KSelectAction
{
public:
    MethodListAction( KDevPlugin *part, const QString &text, int accel,
                      const QObject *receiver, const char *slot,
                      QObject *parent, const char *name );
    void refresh(const QString &className);
    QString currentMethodName();
    
private:
    QString currentText(); // only here for preventing its public use
    KDevPlugin *m_part;
};


class DelayedPopupAction : public KAction
{
    Q_OBJECT

public:
    DelayedPopupAction( const QString &text, const QString &pix, int accel,
                        QObject *receiver, const char *slot,
                        QObject *parent, const char* name );
    ~DelayedPopupAction();
    
    virtual int plug(QWidget *widget, int index=-1);
    virtual void unplug(QWidget *widget);

    QPopupMenu *popupMenu();

private:
    QPopupMenu *m_popup;
};

#endif
