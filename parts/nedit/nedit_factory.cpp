/*
 * Copyright (C) 2001 Cornelius Schumacher
 */


#include <kinstance.h>
#include <kaction.h>
#include <kstandarddirs.h>

#include "nedit_factory.h"
#include "nedit_part.h"


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance* NeditPartFactory::s_instance = 0L;
KAboutData* NeditPartFactory::s_about = 0L;

NeditPartFactory::NeditPartFactory()
{}




NeditPartFactory::~NeditPartFactory()
{
  delete s_instance;
  delete s_about;

  s_instance = 0L;
}

QObject* NeditPartFactory::createObject(QObject *parent,const char *name,
                                        const char *,const QStringList &)
{
  if (KStandardDirs::findExe("nc") == QString::null) return 0;

  // Create an instance of our Part
  NeditPart* obj = new NeditPart( parent, name );

  return obj;
}

KInstance* NeditPartFactory::instance()
{
  if ( !s_instance )
  {
    s_about = new KAboutData("nedit_part", I18N_NOOP("NeditPart"), "0.1");
    s_about->addAuthor("Cornelius Schumacher", 0, "cs@caldera.de");
    s_instance = new KInstance(s_about);
  }
  return s_instance;
}

extern "C"
{
  void* init_libneditpart()
  {
    return new NeditPartFactory;
  }
};

#include "nedit_factory.moc"
