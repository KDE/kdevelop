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

#ifndef _CVSPART_H_
#define _CVSPART_H_

#include <kdialogbase.h>
#include <qguardedptr.h>

#include "kdevplugin.h"

class Context;
class QPopupMenu;
class CvsWidget;

class CvsPart : public KDevPlugin {
    Q_OBJECT

public:
    CvsPart( QObject *parent, const char *name, const QStringList & );
    ~CvsPart();

    const QString default_cvs;
    const QString default_commit;
    const QString default_update;
    const QString default_add;
    const QString default_remove;
    const QString default_replace;
    const QString default_diff;
    const QString default_log;
    const QString default_rsh;

private slots:
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotCommit();
    void slotUpdate();
    void slotAdd();
    void slotRemove();
    void slotReplace();
    void slotLog();
    void slotDiff();
    void slotDiffFinished( const QString& diff, const QString& err );

    void projectConfigWidget(KDialogBase *dlg);

private:
    QString cvs_rsh() const;

    QString popupfile;
    QGuardedPtr<CvsWidget> m_widget;
};

#endif
