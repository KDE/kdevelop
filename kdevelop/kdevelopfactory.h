/***************************************************************************
                          kdevelopfactory.h  -  description
                             -------------------
    begin                : Thu Jun 15 2000
    copyright            : (C) 2000 by The KDevelop Team
    email                : kdevelop-team@fara.cs.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVELOPFACTORY_H
#define KDEVELOPFACTORY_H

#include <kaboutdata.h>

/**Provides about data and services
  *@author The KDevelop Team
  */

class KDevelopFactory {
public: 
	KDevelopFactory();
	virtual ~KDevelopFactory();
  /** provides the about data for KDevelop */
  static const KAboutData* aboutData();

private:
  static KAboutData *kdev_aboutData;
};

#endif
