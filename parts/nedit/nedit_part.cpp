#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qlabel.h>

#include "nedit_part.h"
#include "nedit_factory.h"


using namespace KEditor;


NeditPart::NeditPart(QObject *parent, const char *name )
  : Editor(parent, name)
{
  setInstance( NeditPartFactory::instance() );

  setXMLFile("nedit_part.rc");

  _documents.setAutoDelete(true);
}


NeditPart::~NeditPart()
{
}


Document *NeditPart::document(const KURL &url)
{
  QPtrListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
    if (it.current()->url() == url)
      return it.current();

  return 0;
}

KEditor::Document *NeditPart::createDocument(QWidget *parentWidget, const KURL &url)
{
  DocumentImpl *impl = new DocumentImpl(this, parentWidget);
  if (!url.isEmpty())
    impl->openURL(url);

  _documents.append(impl);
  connect(impl, SIGNAL(destroyed()), this, SLOT(documentDestroyed()));

  return impl;
}

void NeditPart::documentDestroyed()
{
  _documents.remove(static_cast<const DocumentImpl*>(sender()));
}

#include "nedit_part.moc"
