#include <qfileinfo.h>
#include <stdio.h>
#include <cvsdialog.h>
#include "cvs.h"


void CVS::add(const char *filename, const char *text)
{
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());

    QString command("cd ");
    command += dirpath;
    command += " && cvs add ";
    command += filename;
    command += "2>&1";
    // >/dev/null";

    ( new CvsDialog(command, text) )->show();
}


void CVS::remove(const char *filename, const char *text)
{
    QFileInfo fi(filename);
    QString dirpath(fi.dirPath());
    QString name(fi.fileName());

    QString command("cd ");
    command += dirpath;
    command += " && cvs remove -f ";
    command += filename;
    command += "2>&1";
    // >/dev/null";

    ( new CvsDialog(command, text) )->show();
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
	    if (qstrcmp(buf+1, name) == 0)
		{
		    found = true;
		    break;
		}
	}
    fclose(f);
    return found;
}
    
