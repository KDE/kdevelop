#include <stdlib.h>

#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtabwidget.h>

#include <kdebug.h>
#include <klocale.h>

#include "nedit_factory.h"
#include "cursor_iface_impl.h"
#include "document_impl.h"

using namespace KEditor;

int DocumentImpl::mCounter = 0;

DocumentImpl::DocumentImpl(Editor *parent,QWidget *)
  : Document(parent)
{
  setInstance(NeditPartFactory::instance());

  new CursorIfaceImpl(this, parent);
}


bool DocumentImpl::create()
{
  system ("nc -noask");
  
  return true;
}

bool DocumentImpl::openFile()
{
  kdDebug() << "DocumentImpl::load(): " << m_file << endl; 

  mFilename = m_file;

  system ("nc -noask " + mFilename);

  resetModifiedTime();

  emit KEditor::Document::loaded(this);  

  return true;
}


bool DocumentImpl::saveFile()
{
  kdDebug() << "DocumentImpl::save(): " << m_file << endl; 

  if (!shouldBeSaved())
    return false;

  system ("nc -noask -do \"save()\" " + m_file);

  resetModifiedTime();

  emit KEditor::Document::saved(this);

  return true;
}

bool DocumentImpl::shouldBeSaved()
{
  return true;
}

#include "document_impl.moc"
