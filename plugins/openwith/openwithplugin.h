/*
* This file is part of KDevelop
* Copyright 2009  Andreas Pakulat <apaku@gmx.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H
#define KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>
#include <kservice.h>

#include "iopenwith.h"

class QSignalMapper;

namespace KDevelop
{
class ContextMenuExtension;
class Context;
}

class OpenWithPlugin : public KDevelop::IPlugin, public KDevelop::IOpenWith
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IOpenWith )
public:
    OpenWithPlugin( QObject* parent, const QVariantList& args );
    virtual ~OpenWithPlugin();
    virtual KDevelop::ContextMenuExtension contextMenuExtension ( KDevelop::Context* context );

protected:
    virtual void openFilesInternal( const QList<QUrl>& files );

private slots:
    void open( const QString& );
    void openDefault();

private:
    QList<QAction*> actionsForServiceType( const QString& serviceType );
    QScopedPointer<QSignalMapper> m_actionMap;
    QList<QUrl> m_urls;
    QString m_mimeType;
    KService::List m_services;
};

#endif // KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H
