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

#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

class CvsProcessWidget;
class CvsJob_stub;
class DCOPRef;

/**
 * A simple interface for CVS jobs scheduling
 *
 * @author Mario Scalas
*/
class JobScheduler
{
public:
    JobScheduler( CvsProcessWidget *aProcessWidget );
    virtual ~JobScheduler();

    virtual bool schedule( DCOPRef &job ) = 0;

    CvsProcessWidget *processWidget() const { return m_processWidget; }

private:
    CvsProcessWidget *m_processWidget;
};


/**
 * An implementation which simply run the job, without any buffering
 *
*/
class DirectScheduler : public JobScheduler
{
public:
    DirectScheduler( CvsProcessWidget *aProcessWidget );

    virtual bool schedule( DCOPRef &job );
};

#endif
