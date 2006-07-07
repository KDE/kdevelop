#ifndef __PARTCONTROLLER_H__
#define __PARTCONTROLLER_H__

#include <QMap>
#include <QHash>
#include <QWidget>
#include <QPointer>

#include <kurl.h>
#include <kparts/partmanager.h>

#include "kdevdocument.h"

namespace KParts
{
class Part;
class Factory;
class PartManager;
class ReadOnlyPart;
class ReadWritePart;
}

namespace KTextEditor
{
class Document;
class Editor;
}

class PartController : public KParts::PartManager
{
    Q_OBJECT
public:
    PartController( QWidget *toplevel );
    virtual ~PartController();

    KParts::Part* createPart( const KUrl &url );
    KParts::Part* createPart( const QString &mimeType,
                              const QString &partType,
                              const QString &className,
                              const QString &preferredName = QString::null );

    KParts::ReadOnlyPart* activeReadOnly() const;
    KParts::ReadWritePart* activeReadWrite() const;
    KParts::ReadOnlyPart* readOnly( KParts::Part *part ) const;
    KParts::ReadWritePart* readWrite( KParts::Part *part ) const;

private:
    KParts::Factory *findPartFactory( const QString &mimeType,
                                      const QString &partType,
                                      const QString &preferredName = QString::null );

    QHash<KParts::ReadOnlyPart*, KDevDocument*> m_partHash;

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
