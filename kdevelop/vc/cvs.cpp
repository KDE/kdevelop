#include <kapp.h>
#include <qfileinfo.h>
#include <stdio.h>
#include "cvsdialog.h"
#include "commitdialog.h"
#include "cvs.h"


void CVS::add(const char *filename)
{
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());

    QString command("cd ");
    command += dirpath;
    command += " && cvs add ";
    command += name;
    command += " 2>&1";

    CvsDialog l(i18n("Adding file to repository"));
    l.startCommand(command);
    l.exec();
}


void CVS::remove(const char *filename)
{
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());

    QString command("cd ");
    command += dirpath;
    command += " && cvs remove -f ";
    command += name;
    command += " 2>&1";

    CvsDialog l(i18n("Removing file from repository"));
    l.startCommand(command);
    l.exec();
}


void CVS::update(const char *filename)
{
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());

    QString command("cd ");
    command += dirpath;
    command += " && cvs update -dP ";
    command += name;
    command += " 2>&1";

    CvsDialog l(i18n("Updating"));
    l.startCommand(command);
    l.exec();
}


void CVS::commit(const char *filename)
{
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());

    CommitDialog *d = new CommitDialog();
    if (d->exec() == QDialog::Rejected)
        return;
        
    QString command("cd ");
    command += dirpath;
    command += " && cvs commit -m \"";
    command += d->logMessage();
    command += "\" ";
    command += name;
    command += " 2>&1";

    delete d;

    CvsDialog l(i18n("Commiting file"));
    l.startCommand(command);
    l.exec();
}


VersionControl::State CVS::registeredState(const char *filename)
{
    char buf[512];
    State state;
    
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());
    QString entriesfile(dirpath + "/CVS/Entries");

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
}
    
