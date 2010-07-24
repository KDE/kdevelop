/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef SNIPPETFEATURES_H
#define SNIPPETFEATURES_H

#include <kdeversion.h>

#if KDE_IS_VERSION(4, 4, 90)
  #define SNIPPETS_HAVE_TPLIFACE2
#endif

#if KDE_IS_VERSION(4, 4, 60)
  #define SNIPPETS_ENABLE_GHNS_UPLOAD
#endif

#if KDE_IS_VERSION(4, 4, 0)
  #define SNIPPETS_HAVE_HIGHLIGHTIFACE
  #define SNIPPETS_HAVE_GHNS
#endif

#endif // SNIPPETFEATURES_H
