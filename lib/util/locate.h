//
//
// C++ Interface for module: Locate
//
// Description: Deal with filenames in kdevelop
//
// Author: Eray Ozkural (exa) <erayo@cs.bilkent.edu.tr>
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Locate_Interface
#define Locate_Interface

#include <qstring.h>
class KDevPlugin;

/** This class deals with filenames in kdevelop
 ** Preferably used as a singleton
 */

class Locate
{
public:
  Locate(KDevPlugin& part);

  /** return the project-relative filename of a given URL if possible */
  QString relativeName(const QString);

  /** locate a data file */
  QString data(const QString);

  /** locate a source file */
  QString source(const QString);

  /** locate a build product */
  QString product(const QString);

private:
  KDevPlugin& part;
};

#endif
