/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLEARCASEPART_H_
#define _CLEARCASEPART_H_

#include "kdevplugin.h"

class Context;
class QPopupMenu;

class ClearcasePart : public KDevPlugin {
    Q_OBJECT

public:
    ClearcasePart( QObject *parent, const char *name, const QStringList & );
    ~ClearcasePart();

    const QString default_checkin;
    const QString default_checkout;
    const QString default_uncheckout;
    const QString default_create;
    const QString default_remove;
    const QString default_diff;
//    const QString default_log;

private slots:
    void contextMenu(QPopupMenu *popup, const Context *context);

    void slotCheckin();
    void slotCheckout();
    void slotUncheckout();

    void slotCreate();
    void slotRemove();
//    void slotLog();

    void slotDiff();
    void slotDiffFinished( const QString& diff, const QString& err );

private:
    QString popupfile;
};

#endif
