#ifndef __NEDIT_PART_H__
#define __NEDIT_PART_H__

#include <qptrlist.h>

#include <kparts/part.h>

#include "keditor/editor.h"
#include "document_impl.h"

class KURL;

class NeditPart : public KEditor::Editor
{
    Q_OBJECT
  public:
    NeditPart(QObject *parent, const char *name);
    virtual ~NeditPart();

    virtual KEditor::Document *document(const KURL &url);
    virtual KEditor::Document *createDocument(QWidget *parentWidget=0, const KURL &url="");

  private slots:
    void documentDestroyed();

  private:
    QPtrList <DocumentImpl> _documents;
};

#endif
