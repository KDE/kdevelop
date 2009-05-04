/*  This file is part of the KDE project
    
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "processes_local_p.h"
#include "process.h"

#include <klocale.h>

#include <QFile>
#include <QHash>
#include <QSet>
#include <QByteArray>
#include <QTextStream>

//for sysconf
#include <unistd.h>  
//for kill and setNice
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <dirent.h>
#include <stdlib.h>
//for ionice
#include <sys/ptrace.h>
#include <asm/unistd.h>
//for getsched
#include <sched.h>

#define PROCESS_BUFFER_SIZE 1000

/* For ionice */
extern int sys_ioprio_set(int, int, int);
extern int sys_ioprio_get(int, int);

#define HAVE_IONICE
/* Check if this system has ionice */
#if !defined(SYS_ioprio_get) || !defined(SYS_ioprio_set)
/* All new kernels have SYS_ioprio_get and _set defined, but for the few that do not, here are the definitions */
#if defined(__i386__)
#define __NR_ioprio_set         289
#define __NR_ioprio_get         290
#elif defined(__ppc__) || defined(__powerpc__)
#define __NR_ioprio_set         273
#define __NR_ioprio_get         274
#elif defined(__x86_64__)
#define __NR_ioprio_set         251
#define __NR_ioprio_get         252
#elif defined(__ia64__)
#define __NR_ioprio_set         1274
#define __NR_ioprio_get         1275
#else
#ifdef __GNUC__
#warning "This architecture does not support IONICE.  Disabling ionice feature."
#endif
#undef HAVE_IONICE
#endif
/* Map these to SYS_ioprio_get */
#define SYS_ioprio_get                __NR_ioprio_get
#define SYS_ioprio_set                __NR_ioprio_set

#endif /* !SYS_ioprio_get */

/* Set up ionice functions */
#ifdef HAVE_IONICE
#define IOPRIO_WHO_PROCESS 1
#define IOPRIO_CLASS_SHIFT 13

/* Expose the kernel calls to usespace via syscall
 * See man ioprio_set  and man ioprio_get   for information on these functions */
static int ioprio_set(int which, int who, int ioprio)
{
  return syscall(SYS_ioprio_set, which, who, ioprio);
}
 
static int ioprio_get(int which, int who)
{
  return syscall(SYS_ioprio_get, which, who);
}
#endif




namespace KSysGuard
{

  class ProcessesLocal::Private
  {
    public:
      Private() { mProcDir = opendir( "/proc" );}
      ~Private();
      inline bool readProcStatus(long pid, Process *process);
      inline bool readProcStat(long pid, Process *process);
      inline bool readProcStatm(long pid, Process *process);
      inline bool readProcCmdline(long pid, Process *process);
      inline bool getNiceness(long pid, Process *process);
      QFile mFile;
      char mBuffer[PROCESS_BUFFER_SIZE+1]; //used as a buffer to read data into      
      DIR* mProcDir;
  };

ProcessesLocal::Private::~Private() 
{ 
    closedir(mProcDir);
}
  
ProcessesLocal::ProcessesLocal() : d(new Private())
{

}

bool ProcessesLocal::Private::readProcStatus(long pid, Process *process)
{
    mFile.setFileName(QString("/proc/%1/status").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */

    process->uid = 0;
    process->gid = 0;
    process->tracerpid = 0;

    int size;
    int found = 0; //count how many fields we found
    while( (size = mFile.readLine( mBuffer, sizeof(mBuffer))) > 0) {  //-1 indicates an error
        switch( mBuffer[0]) {
	  case 'N':
	    if((unsigned int)size > sizeof("Name:") && qstrncmp(mBuffer, "Name:", sizeof("Name:")-1) == 0) {
	        if(process->command.isEmpty())
                process->name = QString::fromLocal8Bit(mBuffer + sizeof("Name:")-1, size-sizeof("Name:")+1).trimmed();
	        if(++found == 4) goto finish;
	    }
	    break;
	  case 'U': 
	    if((unsigned int)size > sizeof("Uid:") && qstrncmp(mBuffer, "Uid:", sizeof("Uid:")-1) == 0) {
		sscanf(mBuffer + sizeof("Uid:") -1, "%Ld %Ld %Ld %Ld", &process->uid, &process->euid, &process->suid, &process->fsuid );
	        if(++found == 4) goto finish;
	    }
	    break;
	  case 'G':
	    if((unsigned int)size > sizeof("Gid:") && qstrncmp(mBuffer, "Gid:", sizeof("Gid:")-1) == 0) {
		sscanf(mBuffer + sizeof("Gid:")-1, "%Ld %Ld %Ld %Ld", &process->gid, &process->egid, &process->sgid, &process->fsgid );
	        if(++found == 4) goto finish;
	    }
	    break;
	  case 'T':
	    if((unsigned int)size > sizeof("TracerPid:") && qstrncmp(mBuffer, "TracerPid:", sizeof("TracerPid:")-1) == 0) {
		process->tracerpid = atol(mBuffer + sizeof("TracerPid:")-1);
	        if(++found == 4) goto finish;
	    }
	    break;
	  default:
	    break;
	}
    }

    finish:
    mFile.close();
    return true;
}

long ProcessesLocal::getParentPid(long pid) {
    Q_ASSERT(pid != 0);
    d->mFile.setFileName(QString("/proc/%1/stat").arg(pid));
    if(!d->mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;      /* process has terminated in the meantime */

    int size; //amount of data read in
    if( (size = d->mFile.readLine( d->mBuffer, sizeof(d->mBuffer))) <= 0) { //-1 indicates nothing read
        d->mFile.close();
        return 0;
    }

    d->mFile.close();
    int current_word = 0;
    char *word = d->mBuffer;

    while(true) {
	    if(word[0] == ' ' ) {
		    if(++current_word == 3)
			    break;
	    } else if(word[0] == 0) {
	    	return 0; //end of data - serious problem
	    }
	    word++;
    }
    return atol(++word);
}

bool ProcessesLocal::Private::readProcStat(long pid, Process *ps)
{
    QString filename = QString("/proc/%1/stat").arg(pid);
    // As an optomization, if the last file read in was stat, then we already have this info in memory
    if(mFile.fileName() != filename) {  
        mFile.setFileName(filename);
        if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;      /* process has terminated in the meantime */
	if( mFile.readLine( mBuffer, sizeof(mBuffer)) <= 0) { //-1 indicates nothing read
	    mFile.close();
	    return false;
	}
	mFile.close();
    }

    int current_word = 0;  //count from 0
    char *word = mBuffer;
    char status='\0';
    long vmSize = 0;
    long vmRSS = 0;
    while(current_word < 23) {
	    if(word[0] == ' ' ) {
		    ++current_word;
		    switch(current_word) {
			    case 2: //status
                              status=word[1];  // Look at the first letter of the status.  
			                      // We analyze this after the while loop
			      break;
			    case 6: //ttyNo
			      {
			        int ttyNo = atoi(word+1);
				int major = ttyNo >> 8;
				int minor = ttyNo & 0xff;
				switch(major) {
				  case 136:
				    ps->setTty(QByteArray("pts/") + QByteArray::number(minor));
				    break;
				  case 5:
				    ps->setTty(QByteArray("tty"));
				  case 4:
				    if(minor < 64)
				      ps->setTty(QByteArray("tty") + QByteArray::number(minor));
				    else
				      ps->setTty(QByteArray("ttyS") + QByteArray::number(minor-64));
				    break;
				  default:
				    ps->setTty(QByteArray());
				}
			      }
			      break;
			    case 13: //userTime
			      ps->setUserTime(atoll(word+1));
			      break;
			    case 14: //sysTime
			      ps->setSysTime(atoll(word+1));
			      break;
			    case 18: //niceLevel
			      ps->setNiceLevel(atoi(word+1));  /*Or should we use getPriority instead? */
			      break;
			    case 22: //vmSize
			      vmSize = atol(word+1);
			      break;
			    case 23: //vmRSS
			      vmRSS = atol(word+1);
			      break;
			    default:
			      break;
		    }
	    } else if(word[0] == 0) {
	    	return false; //end of data - serious problem
	    }
	    word++;
    }

    /* There was a "(ps->vmRss+3) * sysconf(_SC_PAGESIZE)" here in the original ksysguard code.  I have no idea why!  After comparing it to
     *   meminfo and other tools, this means we report the RSS by 12 bytes differently compared to them.  So I'm removing the +3
     *   to be consistent.  NEXT TIME COMMENT STRANGE THINGS LIKE THAT! :-) */
    ps->setVmRSS(vmRSS * sysconf(_SC_PAGESIZE) / 1024); /*convert to KiB*/
    ps->setVmSize(vmSize /= 1024); /* convert to KiB */

    switch( status) {
      case 'R':
        ps->setStatus(Process::Running);
	break;
      case 'S':
        ps->setStatus(Process::Sleeping);
	break;
      case 'D':
        ps->setStatus(Process::DiskSleep);
	break;
      case 'Z':
        ps->setStatus(Process::Zombie);
	break;
      case 'T':
         ps->setStatus(Process::Stopped);
         break;
      case 'W':
         ps->setStatus(Process::Paging);
         break;
      default:
         ps->setStatus(Process::OtherStatus);
         break;
    }
    return true;
}

bool ProcessesLocal::Private::readProcStatm(long pid, Process *process)
{
    mFile.setFileName(QString("/proc/%1/statm").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */

    if( mFile.readLine( mBuffer, sizeof(mBuffer)) <= 0) { //-1 indicates nothing read
        mFile.close();
        return 0;
    }
    mFile.close();

    int current_word = 0;
    char *word = mBuffer;

    while(true) {
	    if(word[0] == ' ' ) {
		    if(++current_word == 2) //number of pages that are shared
			    break;
	    } else if(word[0] == 0) {
	    	return false; //end of data - serious problem
	    }
	    word++;
    }
    long shared = atol(word+1);

    /* we use the rss - shared  to find the amount of memory just this app uses */
    process->vmURSS = process->vmRSS - (shared * sysconf(_SC_PAGESIZE) / 1024);
    return true;
}


bool ProcessesLocal::Private::readProcCmdline(long pid, Process *process)
{
    if(!process->command.isNull()) return true; //only parse the cmdline once
    mFile.setFileName(QString("/proc/%1/cmdline").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */

    QTextStream in(&mFile);
    process->command = in.readAll();

    //cmdline seperates parameters with the NULL character
    process->command.replace('\0', ' ');
    process->command = process->command.trimmed();
    if(!process->command.isEmpty()) {
        QString processname = process->command;
	processname.remove(QRegExp("^[^ ]*/"));
	if(processname.startsWith(process->name))
	    process->name = processname;
    }

    mFile.close();
    return true;
}

bool ProcessesLocal::Private::getNiceness(long pid, Process *process) {
  int sched = sched_getscheduler(pid);
  switch(sched) {
      case (SCHED_OTHER):
	    process->scheduler = KSysGuard::Process::Other;
            break;
      case (SCHED_RR):
	    process->scheduler = KSysGuard::Process::RoundRobin;
            break;
      case (SCHED_FIFO):
	    process->scheduler = KSysGuard::Process::Fifo;
            break;
#ifdef SCHED_BATCH
      case (SCHED_BATCH):
	    process->scheduler = KSysGuard::Process::Batch;
            break;
#endif
      default:
	    process->scheduler = KSysGuard::Process::Other;
    }
  if(sched == SCHED_FIFO || sched == SCHED_RR) {
    struct sched_param param;
    if(sched_getparam(pid, &param) == 0)
      process->niceLevel = param.sched_priority;
    else
      process->niceLevel = 0;  //Error getting scheduler parameters. 
  }

#ifdef HAVE_IONICE
  int ioprio = ioprio_get(IOPRIO_WHO_PROCESS, pid);  /* Returns from 0 to 7 for the iopriority, and -1 if there's an error */
  if(ioprio == -1) {
	  process->ioniceLevel = -1;
	  process->ioPriorityClass = KSysGuard::Process::None;
	  return false; /* Error. Just give up. */
  }
  process->ioniceLevel = ioprio & 0xff;  /* Bottom few bits are the priority */
  process->ioPriorityClass = (KSysGuard::Process::IoPriorityClass)(ioprio >> IOPRIO_CLASS_SHIFT); /* Top few bits are the class */
#else
  return false;  /* Do nothing, if we do not support this architecture */
#endif
  return true;
}

bool ProcessesLocal::updateProcessInfo( long pid, Process *process)
{
    if(!d->readProcStat(pid, process)) return false;
    if(!d->readProcStatus(pid, process)) return false;
    if(!d->readProcStatm(pid, process)) return false;
    if(!d->readProcCmdline(pid, process)) return false;
    if(!d->getNiceness(pid, process)) return false;

    return true;
}

QSet<long> ProcessesLocal::getAllPids( )
{
    QSet<long> pids;
    if(d->mProcDir==NULL) return pids; //There's not much we can do without /proc
    struct dirent* entry;
    rewinddir(d->mProcDir);
    while ( ( entry = readdir( d->mProcDir ) ) )
	    if ( entry->d_name[ 0 ] >= '0' && entry->d_name[ 0 ] <= '9' )
		    pids.insert(atol( entry->d_name ));
    return pids;
}

bool ProcessesLocal::sendSignal(long pid, int sig) {
    if ( kill( (pid_t)pid, sig ) ) {
	//Kill failed
        return false;
    }
    return true;
}

bool ProcessesLocal::setNiceness(long pid, int priority) {
    if(pid <= 0) return false; // check the parameters
    if ( setpriority( PRIO_PROCESS, pid, priority ) ) {
	    //set niceness failed
	    return false;
    }
    return true;
}

bool ProcessesLocal::setScheduler(long pid, int priorityClass, int priority) {
    if(priorityClass == KSysGuard::Process::Other || priorityClass == KSysGuard::Process::Batch)
	    priority = 0;
    if(pid <= 0) return false; // check the parameters
    struct sched_param params;
    params.sched_priority = priority;
    switch(priorityClass) {
      case (KSysGuard::Process::Other):
	    return (sched_setscheduler( pid, SCHED_OTHER, &params) == 0);
      case (KSysGuard::Process::RoundRobin):
	    return (sched_setscheduler( pid, SCHED_RR, &params) == 0);
      case (KSysGuard::Process::Fifo):
	    return (sched_setscheduler( pid, SCHED_FIFO, &params) == 0);
#ifdef SCHED_BATCH
      case (KSysGuard::Process::Batch):
	    return (sched_setscheduler( pid, SCHED_BATCH, &params) == 0);
#endif
      default:
	    return false;
    }
}


bool ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority) {
#ifdef HAVE_IONICE
    if (ioprio_set(IOPRIO_WHO_PROCESS, pid, priority | priorityClass << IOPRIO_CLASS_SHIFT) == -1) {
	    //set io niceness failed
	    return false;
    }
    return true;
#else
    return false;
#endif
}

bool ProcessesLocal::supportsIoNiceness() {
#ifdef HAVE_IONICE
    return true;
#else
    return false;
#endif
}

long long ProcessesLocal::totalPhysicalMemory() {
    //Try to get the memory via sysconf.  Note the cast to long long to try to avoid a long overflow
    //Should we use sysconf(_SC_PAGESIZE)  or getpagesize()  ?
    long long memory = ((long long)sysconf(_SC_PHYS_PAGES)) * (sysconf(_SC_PAGESIZE)/1024);
    if(memory > 0) return memory;

    //This is backup code incase the above failed.  It should never fail on a linux system.

    d->mFile.setFileName("/proc/meminfo");
    if(!d->mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0; 

    int size;
    while( (size = d->mFile.readLine( d->mBuffer, sizeof(d->mBuffer))) > 0) {  //-1 indicates an error
        switch( d->mBuffer[0]) {
	  case 'M':
            if((unsigned int)size > sizeof("MemTotal:") && qstrncmp(d->mBuffer, "MemTotal:", sizeof("MemTotal:")-1) == 0) {
		    d->mFile.close();
		    return atoll(d->mBuffer + sizeof("MemTotal:")-1);
            }
	}
    }
    return 0; // Not found.  Probably will never happen
}
ProcessesLocal::~ProcessesLocal()
{
  delete d;  
}

}
