/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Extended to use perforce 2002 by Harald Fernengel <harry@kdevelop.org>*
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PERFORCEPART_H_
#define _PERFORCEPART_H_

#include "kdevplugin.h"

class Context;
class QPopupMenu;


class PerforcePart : public KDevPlugin
{
    Q_OBJECT

public:
    PerforcePart( QObject *parent, const char *name, const QStringList & );
    ~PerforcePart();

    virtual QString shortDescription() const 
    { return "Perforce is a version control system"; };

private slots:
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotCommit();
    void slotUpdate();
    void slotAdd();
    void slotRemove();
    void slotEdit();
    void slotRevert();
    void slotDiff();
    void slotDiffFinished( const QString&, const QString& );

private:
    /** calls p4 with the command cmd and appends the filename */
    void execCommand( const QString& cmd );
    QString popupfile;
};

#endif
