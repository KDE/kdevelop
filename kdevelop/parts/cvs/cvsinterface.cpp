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

#include <klocale.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <stdio.h>
#include "main.h"
#include "commitdialog.h"
#include "cvsinterface.h"


CvsInterface::CvsInterface(QObject *parent, const char *name)
    : KDevVersionControl(parent, name)
{
    setInstance(CvsFactory::instance());
}


CvsInterface::~CvsInterface()
{}


void CvsInterface::addToRepositoryRequested(const QString &fileName)
{
    QString command("cd ");
    command += m_projectpath;
    command += " && cvs add ";
    command += fileName;
    command += " 2>&1";
    emit executeMakeCommand(command);
}


void CvsInterface::removeFromRepositoryRequested(const QString &fileName)
{
    QString command("cd ");
    command += m_projectpath;
    command += " && cvs remove -f ";
    command += fileName;
    command += " 2>&1";
    emit executeMakeCommand(command);
}


void CvsInterface::updateFromRepositoryRequested(const QString &fileName)
{
    QString command("cd ");
    command += m_projectpath;
    command += " && cvs update -dP ";
    command += fileName;
    command += " 2>&1";
    emit executeMakeCommand(command);
}


void CvsInterface::commitToRepositoryRequested(const QString &fileName)
{
    CommitDialog *d = new CommitDialog();
    if (d->exec() == QDialog::Rejected)
        return;

    QString message = d->logMessage();
    if (!message.isEmpty())
        message.replace(QRegExp("'"), "'\"'\"'");

    QString command("cd ");
    command += m_projectpath;
    command += " && cvs commit -m \'";
    command += message;
    command += "\' ";
    command += fileName;
    command += " 2>&1";

    delete d;

    emit executeMakeCommand(command);
}


#if 0
KDevVersionControl::State CvsInterface::registeredState(const QString &fileName)
{
    char buf[512];
    State state;
    
    QString m_projectpath, name;
    splitPath(filename, &m_projectpath, &name);
    
    QString entriesfile(m_projectpath + "/CVS/Entries");

    FILE *f = fopen(entriesfile, "r");
    if (!f)
	return State(0);

    state = canBeAdded;
    while (fgets(buf, sizeof buf, f))
	{
	    char *nextp;
	    if (buf[0] != '/')
		continue;
	    if ( (nextp = strchr(buf+1, '/')) == 0)
		continue;
	    *nextp = '\0';
	    if (qstrcmp(buf+1, name) == 0)
		{
                    state = State(state | canBeCommited);
                    if (*(nextp+1) != '-')
                        state = State(state & ~canBeAdded);
		    break;
		}
	}
    fclose(f);
    return state;
    return canBeAdded;
}
#endif
    
