/*
    This file is part of the KDevelop Okteta module, part of the KDE project.

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OKTETAPLUGIN_H
#define OKTETAPLUGIN_H

// Plugin
#include "oktetaglobal.h"
// KDev
#include <interfaces/iplugin.h>
// Qt
#include <QtCore/QVariantList>

class QUrl;

namespace KASTEN_NAMESPACE
{
class ByteArrayViewProfileManager;
}

namespace KDevelop
{
class OktetaDocumentFactory;


class OktetaPlugin: public IPlugin
{
    Q_OBJECT

  public:
    explicit OktetaPlugin( QObject* parent, const QVariantList& args = QVariantList() );

    virtual ~OktetaPlugin();

  public: // KDevelop::IPlugin API
    virtual ContextMenuExtension contextMenuExtension( Context* context );

#if KASTEN_VERSION == 2
  public:
    Kasten::ByteArrayViewProfileManager* viewProfileManager() const { return mViewProfileManager; }
#endif

  private Q_SLOTS:
    void onOpenTriggered();

  protected:
    OktetaDocumentFactory* mDocumentFactory;
#if KASTEN_VERSION == 2
    Kasten::ByteArrayViewProfileManager* const mViewProfileManager;
#endif
};

}

#endif
