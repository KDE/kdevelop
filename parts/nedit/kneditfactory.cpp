/* This file is part of the KDE libraries
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kneditfactory.h"

#include "document.h"

#include <klocale.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>

#include <kdebug.h>

extern "C"
{
  void *init_libkneditpart()
  {
    KGlobal::locale()->insertCatalogue("kneditpart");
    kdDebug() << "in init_libkneditpart" << endl;
    return new KNEditFactory();
  }
}

KInstance *KNEditFactory::s_instance = 0;

KNEditFactory::KNEditFactory()
{
  s_instance = 0;
}

KNEditFactory::~KNEditFactory()
{
  if ( s_instance )
  {
    delete s_instance->aboutData();
    delete s_instance;
  }
  s_instance = 0;
}

KParts::Part *KNEditFactory::createPartObject( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList & )
{
  bool bReadOnly = (classname == QString::fromLatin1("KParts::ReadOnlyPart") );
  bool bSingleView = (classname != QString::fromLatin1("KTextEditor::Document"));

  // nc has to be installed...
  if ( KStandardDirs::findExe( "nc" ).isEmpty() )
    return 0;

  KParts::ReadWritePart *part = new KNEdit::Document (bReadOnly, bSingleView, parentWidget, widgetName, parent, name);
  part->setReadWrite( !bReadOnly );

  return part;
}

KInstance *KNEditFactory::instance()
{
  if ( !s_instance )
    s_instance = new KInstance( aboutData() );
  return s_instance;
}

const KAboutData *KNEditFactory::aboutData()
{
  KAboutData *data = new KAboutData  ("knedit", I18N_NOOP("KNEdit"), "1.0",
                                                           I18N_NOOP( "KNEdit - KDE wrapper for NEdit" ),
                                                           KAboutData::License_LGPL_V2,
                                                           I18N_NOOP( "(c) 2002" ), 0, "http://www.kde.org");
  data->addAuthor ("Cornelius Schumacher", I18N_NOOP("Initial version"), "cs@caldera.de");
  data->addAuthor ("Harald Fernengel", I18N_NOOP("Ported to KTextEditor"), "harry@kdevelop.org");
  data->addAuthor ("Christoph Cullmann", I18N_NOOP("A lot of help"), "cullmann@kde.org", "http://www.babylon2k.de");

  data->setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"), I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

  return data;
}

#include "kneditfactory.moc"

