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
    command += filename;
    command += " 2>&1";

    ( new CvsDialog(command, i18n("Adding file to repository")) )->show();
}


void CVS::remove(const char *filename)
{
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());

    QString command("cd ");
    command += dirpath;
    command += " && cvs remove -f ";
    command += filename;
    command += " 2>&1";

    ( new CvsDialog(command, i18n("Removing file from repository")) )->show();
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
    command += filename;
    command += " 2>&1";

    delete d;

    ( new CvsDialog(command, i18n("Commiting file")) )->show();
}


bool CVS::isRegistered(const char *filename)
{
    char buf[512];
    bool found;
    
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());
    QString entriesfile(dirpath + "/CVS/Entries");

    FILE *f = fopen(entriesfile, "r");
    if (!f)
	return false;

    found = false;
    while (fgets(buf, sizeof buf, f))
	{
	    char *nextp;
	    if (buf[0] != '/')
		continue;
	    if ( (nextp = strchr(buf+1, '/')) == 0)
		continue;
	    *nextp = '\0';
	    if (qstrcmp(buf+1, name) == 0 && *(nextp+1) != '-')
		{
		    found = true;
		    break;
		}
	}
    fclose(f);
    return found;
}
    
