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

#include "jobscheduler.h"

#include "cvsprocesswidget.h"

#include "kdebug.h"
#include "dcopref.h"

///////////////////////////////////////////////////////////////////////////////
// class JobScheduler
///////////////////////////////////////////////////////////////////////////////

JobScheduler::JobScheduler( CvsProcessWidget *aProcessWidget )
    : m_processWidget( aProcessWidget )
{
}

///////////////////////////////////////////////////////////////////////////////

JobScheduler::~JobScheduler()
{
}

///////////////////////////////////////////////////////////////////////////////
// class DirectScheduler
///////////////////////////////////////////////////////////////////////////////

DirectScheduler::DirectScheduler( CvsProcessWidget *aProcessWidget )
    : JobScheduler( aProcessWidget )
{
}

///////////////////////////////////////////////////////////////////////////////

bool DirectScheduler::schedule( DCOPRef &job )
{
    if (job.isNull())
    {
        kdDebug(9006) << "DirectScheduler::schedule(DCOPRef &): Job is null and will be rejected!" << endl;
        return false;
    }
    processWidget()->startJob( job );

    return true;
}
