/***************************************************************************
                          kdevelopfactory.cpp  -  description
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

#include <klocale.h>

#include "kdevelopfactory.h"

static const char *description=I18N_NOOP("The KDE Integrated Development Environment");
static const char *version="2.0pre";

KAboutData *KDevelopFactory::kdev_aboutData = 0;

KDevelopFactory::KDevelopFactory(){
}


KDevelopFactory::~KDevelopFactory(){
}


const KAboutData *KDevelopFactory::aboutData()
{
  if (!kdev_aboutData)
  {
    kdev_aboutData = new KAboutData( "kdevelop", I18N_NOOP("KDevelop"),
                        version, description,
                        KAboutData::License_GPL,
                        "(c) 1998-2000, The KDevelop Team" );
    kdev_aboutData->addAuthor("Sandy Meier",I18N_NOOP("Maintainer"), "smeier@rz.uni-potsdam.de");
    kdev_aboutData->addAuthor("Stefan Heidrich",I18N_NOOP("KAppWizard, Printing"), "sheidric@rz.uni-potsdam.de");
    kdev_aboutData->addAuthor("Ralf Nolden",I18N_NOOP("KDevelop<->Dialogeditor Interface, Configuration Functionality, Online Help"), "");
    kdev_aboutData->addAuthor("Jonas Nordin",I18N_NOOP("Classviewer and Classparser"), "jonas.nordin@syncom.se");
    kdev_aboutData->addAuthor("Pascal Krahmer",I18N_NOOP("Dialogeditor"), "pascal@beast.de");
    kdev_aboutData->addAuthor("Jörgen Olsson",I18N_NOOP("Graphical Classviewer"), "jorgen@trej.net");
    kdev_aboutData->addAuthor("Stefan Bartel",I18N_NOOP("Real-File-Viewer, Project Options"), "bartel@rz.uni-potsdam.de");
    kdev_aboutData->addAuthor("Bernd Gehrmann",I18N_NOOP("Documentation Tree"), "bernd@physik.hu-berlin.de");
    kdev_aboutData->addAuthor("Walter Tasin",I18N_NOOP("Many, Many Bugfixes, General Enhancements"), "tasin@e-technik.fh-muenchen.de");
    kdev_aboutData->addAuthor("John Birch",I18N_NOOP("Internal Debugger"), "jbb@ihug.co.nz");
    kdev_aboutData->addAuthor("Falk Brettschneider",I18N_NOOP("Dockwidget/MDI GUI"), "gigafalk@yahoo.com");
  }
  return kdev_aboutData;
}
