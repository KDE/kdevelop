/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <dcopobject.h>

#ifndef __CVSSERVICEDCOPIFACE_H_
#define __CVSSERVICEDCOPIFACE_H_
/**
* DCOP Iface for classes which use CvsService services.
*/
class CVSServiceDCOPIface : virtual public DCOPObject
{
    K_DCOP
k_dcop:
    virtual void slotJobExited( bool normalExit, int exitStatus ) = 0;
    virtual void slotReceivedOutput( QString someOutput ) = 0;
    virtual void slotReceivedErrors( QString someErrors ) = 0;
};

#endif
