//
// C++ Implementation: ClearcaseManipulator
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "clearcasemanipulator.h"

#include <kprocess.h>

#include <qregexp.h>
#include <qstring.h>

#include <stdlib.h>
#include <libgen.h>
#include <errno.h>

#if defined(Q_OS_MACX) || defined(MACOSX)
//this function is taken from GNU libc
//it does not exist on macos
int getline(char **lineptr, size_t *n, FILE *stream)
{
    static char line[256];
    char *ptr;
    unsigned int len;

   if (lineptr == NULL || n == NULL)
   {
      errno = EINVAL;
      return -1;
   }

   if (ferror (stream))
      return -1;

   if (feof(stream))
      return -1;

   fgets(line,256,stream);

   ptr = strchr(line,'\n');
   if (ptr)
      *ptr = '\0';

   len = strlen(line);

   if ((len+1) < 256)
   {
      ptr = (char*)realloc(*lineptr, 256);
      if (ptr == NULL)
         return(-1);
      *lineptr = ptr;
      *n = 256;
   }

   strcpy(*lineptr,line);
   return(len);
}
#endif

const char ClearcaseManipulator::CT_DESC_SEPARATOR = ';';


ClearcaseManipulator::ClearcaseManipulator()
{
}


ClearcaseManipulator::~ClearcaseManipulator()
{}


bool ClearcaseManipulator::isCCRepository( const QString & directory ) {
  QString cmd;
  cmd = "cd " + directory + " && cleartool pwv -root";
  if ( system(cmd.ascii()) == 0 ) return true;

  return false;
}

VCSFileInfoMap* ClearcaseManipulator::retreiveFilesInfos(const QString& directory) {


  VCSFileInfoMap* fileInfoMap = new VCSFileInfoMap();

  char CCcommand[1024];
  sprintf(CCcommand, "cleartool desc -fmt \"%%m;%%En;%%Rf;%%Sn;%%PVn\\n\" %s/*", directory.ascii());
  FILE* outputFile = popen(CCcommand, "r");

  char* line = NULL;
  size_t numRead;
  while (!feof(outputFile)) {
    getline(&line,&numRead,outputFile);

    if (numRead > 0) {
      int pos = 0;
      int lastPos = -1;

      QStringList outputList;
      outputList = outputList.split(CT_DESC_SEPARATOR, QString(line), true );
      outputList[Name] = QString(basename((char*)outputList[Name].ascii()));

      VCSFileInfo::FileState state;
      if (outputList[ClearcaseManipulator::State] == "unreserved" || outputList[ClearcaseManipulator::State] == "reserved") {
	state = VCSFileInfo::Modified;
      }
      else if (outputList[ClearcaseManipulator::State] == "") {
	state = VCSFileInfo::Uptodate;
      }
      else {
	state = VCSFileInfo::Unknown;
      }


      (*fileInfoMap)[outputList[ClearcaseManipulator::Name]] = VCSFileInfo(outputList[ClearcaseManipulator::Name], outputList[ClearcaseManipulator::Version], outputList[ClearcaseManipulator::RepositoryVersion], state);
    }
  }

  pclose(outputFile);

  return fileInfoMap;
}



