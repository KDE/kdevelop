/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */


#include <kinstance.h>
#include <kaction.h>


#include "editortest_factory.h"
#include "editortest_part.h"


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance* EditorTestPartFactory::s_instance = 0L;
KAboutData* EditorTestPartFactory::s_about = 0L;

EditorTestPartFactory::EditorTestPartFactory()
    : KParts::Factory()
{}




EditorTestPartFactory::~EditorTestPartFactory()
{
  delete s_instance;
  delete s_about;

  s_instance = 0L;
}

KParts::Part* EditorTestPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
    QObject *parent, const char *name,
    const char *, const QStringList &)
{
  // Create an instance of our Part
  EditorTestPart* obj = new EditorTestPart( parentWidget, widgetName, parent, name );

  return obj;
}

KInstance* EditorTestPartFactory::instance()
{
  if ( !s_instance )
  {
    s_about = new KAboutData("editortest_part", I18N_NOOP("EditorTestPart"), "0.1");
    s_about->addAuthor("Matthias Hölzer-Klüpfel", 0, "mhk@caldera.de");
    s_instance = new KInstance(s_about);
  }
  return s_instance;
}

extern "C"
{
  void* init_libeditortestpart()
  {
    return new EditorTestPartFactory;
  }
};

#include "editortest_factory.moc"
