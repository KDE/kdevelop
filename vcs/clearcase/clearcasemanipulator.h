//
// C++ Interface: ClearcaseManipulator
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CLEARCASEMANIPULATOR_H
#define CLEARCASEMANIPULATOR_H

#include "kdevversioncontrol.h"

#include <kprocess.h>

#include <qregexp.h>

#include <string>

/**
  @author KDevelop Authors
  */
class ClearcaseManipulator {
  public:
    ClearcaseManipulator();

    ~ClearcaseManipulator();

    static bool isCCRepository(const QString& directory);

    VCSFileInfoMap* retreiveFilesInfos(const QString& directory);

  private:
    enum FileInfosFields {
      Type = 0,
      Name,
      State,
      Version,
      RepositoryVersion
    };

    static const char CT_DESC_SEPARATOR;

};

#endif
