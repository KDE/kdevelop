
/***************************************************************************
*   Copyright (C) 2006 by Andras Mantia                                   *
*   amantia@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SETUPHELPER_H
#define SETUPHELPER_H

#include <qstringlist.h>

/**
  A helper methods for setting up the various Driver derivates.

  @author Andras Mantia <amantia@kde.org>
 */
namespace SetupHelper {
  /** Get the include paths returned by gcc.
   * @param ok false if there was a problem running gcc
   */  
  QString getGccIncludePath(bool *ok);

  /** Get the include-path return by gcc -v
   * this path includes the path from the environment,
   * the c++-include-path, etc., and of couse it also includes the above path.
   * @param ok false if there was a problem running gcc
   * */
  QString getVerboseGccIncludePath(bool* ok);
  
  /** Get the predefined macros returned by gcc
   * @param ok false if there was a problem running gcc
   */
  QStringList getGccMacros(bool *ok);
};

#endif
