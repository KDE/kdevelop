/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PARTEXPLORERPLUGIN_H_
#define _PARTEXPLORERPLUGIN_H_

#include <qguardedptr.h>

#include <kdevversioncontrol.h>

class QWidget;
class QPainter;
class KURL;
class QVBox;

namespace KParts
{
    class ReadOnlyPart;
};

/**
 *
 * @short Main Part
 * @author Mario Scalas <mario.scalas@libero.it>
 * @version 0.1
 */
class CervisiaPlugin : public KDevVersionControl
{
    Q_OBJECT
public:
    /**
    * Default constructor
    */
    CervisiaPlugin(  QObject *parent, const char *name, const QStringList & );
    /**
    * Destructor
    */
    virtual ~CervisiaPlugin();

public slots:
    void slotProjectOpened();
    void slotProjectClosed();

private:
    KParts::ReadOnlyPart *m_part;
};

#endif // _PARTEXPLORERPLUGIN_H_
