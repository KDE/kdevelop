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

#ifndef _GREPVIEWPART_H_
#define _GREPVIEWPART_H_

#include <qguardedptr.h>
#include "kdevpart.h"

class KDialogBase;
class GrepViewWidget;


class GrepViewPart : public KDevPart
{
    Q_OBJECT

public:
    GrepViewPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~GrepViewPart();

private slots:
    void slotRaiseWidget();
    void configWidget(KDialogBase *dlg);
    void stopButtonClicked();
    void projectOpened();
    void projectClosed();

private:
    QGuardedPtr<GrepViewWidget> m_widget;
    friend class GrepViewWidget;
};

#endif
