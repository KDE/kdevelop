/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */


#include <kinstance.h>
#include <kaction.h>


#include "kwrite_factory.h"
#include "kwrite_part.h"


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance* KWritePartFactory::s_instance = 0L;
KAboutData* KWritePartFactory::s_about = 0L;

KWritePartFactory::KWritePartFactory()
    : KParts::Factory()
{}




KWritePartFactory::~KWritePartFactory()
{
  delete s_instance;
  delete s_about;

  s_instance = 0L;
}

KParts::Part* KWritePartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
    QObject *parent, const char *name,
    const char *, const QStringList &)
{
  // Create an instance of our Part
  KWritePart* obj = new KWritePart( parentWidget, widgetName, parent, name );

  return obj;
}

KInstance* KWritePartFactory::instance()
{
  if ( !s_instance )
  {
    s_about = new KAboutData("kwriteeditor_part", I18N_NOOP("KWriteEditorPart"), "0.1");
    s_about->addAuthor("Matthias Hölzer-Klüpfel", 0, "mhk@caldera.de");
    s_instance = new KInstance(s_about);
  }
  return s_instance;
}

extern "C"
{
  void* init_libkwriteeditorpart()
  {
    return new KWritePartFactory;
  }
};

#include "kwrite_factory.moc"
