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

#ifndef _MAKEVIEWPART_H_
#define _MAKEVIEWPART_H_

#include <qpointer.h>

#include "kdevmakefrontend.h"

class MakeWidget;

class MakeViewPart : public KDevMakeFrontend
{
    Q_OBJECT

public:
    MakeViewPart( QObject *parent, const char *name, const QStringList & );
    ~MakeViewPart();

    virtual QWidget* widget();
    virtual void updateSettingsFromConfig();

private slots:
    void slotStopButtonClicked(KDevPlugin*);
    
protected:
    virtual void queueCommand(const QString &dir, const QString &command);
    virtual bool isRunning();

private:
    QPointer<MakeWidget> m_widget;
    friend class MakeWidget;
};

#endif
