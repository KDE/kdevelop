/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */


#include <kinstance.h>
#include <kaction.h>


#include "qeditor_factory.h"
#include "qeditor_part.h"


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance* QEditorPartFactory::s_instance = 0L;
KAboutData* QEditorPartFactory::s_about = 0L;

QEditorPartFactory::QEditorPartFactory()
{}




QEditorPartFactory::~QEditorPartFactory()
{
  delete s_instance;
  delete s_about;

  s_instance = 0L;
}

QObject* QEditorPartFactory::createObject( QObject *parent, const char *name,
    const char *, const QStringList &)
{
  // Create an instance of our Part
  QEditorPart* obj = new QEditorPart(parent, name);

  return obj;
}

KInstance* QEditorPartFactory::instance()
{
  if ( !s_instance )
  {
    s_about = new KAboutData("qeditor_part", I18N_NOOP("QEditorPart"), "0.1");
    s_about->addAuthor("Matthias Hölzer-Klüpfel", 0, "mhk@caldera.de");
    s_instance = new KInstance(s_about);
  }
  return s_instance;
}

extern "C"
{
  void* init_libqeditorpart()
  {
    return new QEditorPartFactory;
  }
};

#include "qeditor_factory.moc"
