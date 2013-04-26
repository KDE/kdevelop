/* This file is part of KDevelop
  Copyright 2005 Adam Treat <treat@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEW_PART_H
#define KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEW_PART_H

#include <QtCore/QHash>
#include <QtCore/QPointer>

#include <interfaces/iplugin.h>
#include <interfaces/idocumentcontroller.h>

class KDevDocumentViewPluginFactory;

class KDevDocumentViewPlugin: public KDevelop::IPlugin
{
    Q_OBJECT
public:
    enum RefreshPolicy
    {
        Refresh,
        NoRefresh,
        ForceRefresh
    };

public:
    KDevDocumentViewPlugin( QObject *parent, const QVariantList& args );
    virtual ~KDevDocumentViewPlugin();
    virtual void unload();

private:
    KDevDocumentViewPluginFactory* factory;
};

#endif

