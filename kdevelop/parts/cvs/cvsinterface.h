/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CVSINTERFACE_H_
#define _CVSINTERFACE_H_

#include "kdevversioncontrol.h"


class CvsInterface : public KDevVersionControl
{
    Q_OBJECT
    
public:
    CvsInterface( QObject *parent=0, const char *name=0 );
    ~CvsInterface();

protected:
    virtual QList<KAction> kdevNodeActions(KDevNode *node);

private slots:
    void addToRepository(KDevNode *node);
    void removeFromRepository(KDevNode *node);
    void updateFromRepository(KDevNode *node);
    void commitToRepository(KDevNode *node);
    // virtual KDevVersionControl::State registeredState(const QString &fileName);
};

#endif
