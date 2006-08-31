#ifndef __KDEVPARTCONTROLLER_H__
#define __KDEVPARTCONTROLLER_H__

#include <kparts/partmanager.h>
#include "kdevcore.h"

#include <QMap>
#include <QHash>
#include <QWidget>
#include <QPointer>

#include <kurl.h>

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

class KDevPartController : public KParts::PartManager, protected KDevCoreInterface
{
    friend class KDevCore;
    Q_OBJECT
public:
    KDevPartController();
    virtual ~KDevPartController();

    KTextEditor::Document* createTextPart( const KUrl &url,
                                           const QString &encoding,
                                           bool activate );

    KParts::Part* createPart( const KUrl &url );
    KParts::Part* createPart( const QString &mimeType,
                              const QString &partType,
                              const QString &className,
                              const QString &preferredName = QString::null );

    virtual void removePart( KParts::Part *part);

    KParts::ReadOnlyPart* activeReadOnly() const;
    KParts::ReadWritePart* activeReadWrite() const;
    KParts::ReadOnlyPart* readOnly( KParts::Part *part ) const;
    KParts::ReadWritePart* readWrite( KParts::Part *part ) const;

    bool isTextType( KMimeType::Ptr mimeType );

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );
    virtual void initialize();
    virtual void cleanup();

private:
    KParts::Factory *findPartFactory( const QString &mimeType,
                                      const QString &partType,
                                      const QString &preferredName = QString::null );
    QString m_editor;
    QStringList m_textTypes;

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
