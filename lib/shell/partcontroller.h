/***************************************************************************
 *   Copyright (C) 2006 by Adam Treat  <treat@kde.org>                     *
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef __KDEVPARTCONTROLLER_H__
#define __KDEVPARTCONTROLLER_H__

#include <kparts/partmanager.h>
#include "core.h"

#include <QMap>
#include <QHash>
#include <QWidget>
#include <QPointer>

#include <kurl.h>
#include <kmimetype.h>

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

namespace KDevelop
{

class KDEVPLATFORM_EXPORT PartController : public KParts::PartManager
{
    friend class CorePrivate;
    Q_OBJECT
public:
    PartController(Core *core, QWidget *toplevel);
    virtual ~PartController();

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

    Core *m_core;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
