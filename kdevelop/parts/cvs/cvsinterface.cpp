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
#include "kdevnodes.h"
#include "kdevactions.h"
#include "projectspace.h"
#include "kdevmakefrontend.h"
#include "KDevComponentManager.h"

CvsInterface::CvsInterface(QObject *parent, const char *name)
    : KDevVersionControl(parent, name){
    setInstance(CvsFactory::instance());
}

CvsInterface::~CvsInterface(){
}

ProjectSpace* CvsInterface::projectSpace(){
	return static_cast<ProjectSpace*>(componentManager()->component("ProjectSpace"));
}

KDevMakeFrontend* CvsInterface::makeFrontend(){
	return static_cast<KDevMakeFrontend*>(componentManager()->component("KDevMakeFrontend"));
}

QList<KAction> CvsInterface::kdevNodeActions(KDevNode *node){
    QList<KAction> list;
    
    if (node->inherits("KDevFileNode")) {
        list.append(new KDevNodeAction(node, i18n("Add to repository"),
                                       this, SLOT(addToRepository(KDevNode*)),
                                       0, "add_to_repository"));
        list.append(new KDevNodeAction(node, i18n("Remove from repository"),
                                       this, SLOT(removeFromRepository(KDevNode*)),
                                       0, "remove_from_repository"));
        list.append(new KDevNodeAction(node, i18n("Update"),
                                       this, SLOT(updateFromRepository(KDevNode*)),
                                       0, "update_from_repository"));
        list.append(new KDevNodeAction(node, i18n("Commit"),
                                       this, SLOT(commitToRepository(KDevNode*)),
                                       0, "commit_to_repository"));
    }
    
    return list;
}


void CvsInterface::addToRepository(KDevNode *node)
{
    KDevFileNode *fileNode = static_cast<KDevFileNode*>(node);

    QString command("cd ");
    command += projectSpace()->absolutePath();
    command += " && cvs add ";
    command += fileNode->absoluteFileName();
    command += " 2>&1";
    makeFrontend()->executeMakeCommand(command);
}


void CvsInterface::removeFromRepository(KDevNode *node)
{
    KDevFileNode *fileNode = static_cast<KDevFileNode*>(node);

    QString command("cd ");
    command += projectSpace()->absolutePath();
    command += " && cvs remove -f ";
    command += fileNode->absoluteFileName();
    command += " 2>&1";
    makeFrontend()->executeMakeCommand(command);
}


void CvsInterface::updateFromRepository(KDevNode *node)
{
    KDevFileNode *fileNode = static_cast<KDevFileNode*>(node);

    QString command("cd ");
    command += projectSpace()->absolutePath();
    command += " && cvs update -dP ";
    command += fileNode->absoluteFileName();
    command += " 2>&1";
    makeFrontend()->executeMakeCommand(command);
}


void CvsInterface::commitToRepository(KDevNode *node)
{
    KDevFileNode *fileNode = static_cast<KDevFileNode*>(node);

    CommitDialog *d = new CommitDialog();
    if (d->exec() == QDialog::Rejected)
        return;

    QString message = d->logMessage();
    if (!message.isEmpty())
        message.replace(QRegExp("'"), "'\"'\"'");

    QString command("cd ");
    command += projectSpace()->absolutePath();
    command += " && cvs commit -m \'";
    command += message;
    command += "\' ";
    command += fileNode->absoluteFileName();
    command += " 2>&1";

    delete d;

    makeFrontend()->executeMakeCommand(command);
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
    
#include "cvsinterface.moc"
