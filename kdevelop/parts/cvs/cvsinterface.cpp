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


void CvsInterface::add(const QString &filename)
{
    QString command("cd ");
    command += m_projectpath;
    command += " && cvs add ";
    command += filename;
    command += " 2>&1";
    executeCommand(command);
}


void CvsInterface::remove(const QString &filename)
{
    QString command("cd ");
    command += m_projectpath;
    command += " && cvs remove -f ";
    command += filename;
    command += " 2>&1";
    executeCommand(command);
}


void CvsInterface::update(const QString &filename)
{
    QString command("cd ");
    command += m_projectpath;
    command += " && cvs update -dP ";
    command += filename;
    command += " 2>&1";
    executeCommand(command);
}


void CvsInterface::commit(const QString &filename)
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
    command += filename;
    command += " 2>&1";

    delete d;

    executeCommand(command);
}


KDevVersionControl::State CvsInterface::registeredState(const QString &filename)
{
#if 0
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
#endif
}
    
