#include <kinstance.h>


#include "qeditor_part.h"
#include "qeditor_factory.h"


QEditorPart::QEditorPart(QObject *parent, const char *name)
    : KEditor::Editor(parent, name)
{
    setInstance( QEditorPartFactory::instance() );

    setXMLFile("qeditor_part.rc");
}


QEditorPart::~QEditorPart()
{
}


KEditor::Document *QEditorPart::document(const KURL &url)
{
    QListIterator<DocumentImpl> it(_documents);
    for ( ; it.current(); ++it)
        if (it.current()->url() == url)
            return it.current();

    return 0;
}


KEditor::Document *QEditorPart::createDocument(QWidget *parentWidget, const KURL &url)
{
    DocumentImpl *impl = new DocumentImpl(this, parentWidget);
    if (!url.isEmpty())
        impl->openURL(url);

    _documents.append(impl);
    connect(impl, SIGNAL(destroyed()), this, SLOT(documentDestroyed()));

    return impl;
}


void QEditorPart::documentDestroyed()
{
    _documents.remove(static_cast<const DocumentImpl*>(sender()));
}


#include "qeditor_part.moc"
