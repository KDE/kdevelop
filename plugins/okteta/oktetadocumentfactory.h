/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OKTETADOCUMENTFACTORY_H
#define OKTETADOCUMENTFACTORY_H

// plugin
#include "oktetadocument.h"
// KDev
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

namespace KDevelop
{
class OktetaPlugin;


class OktetaDocumentFactory : public IDocumentFactory
{
  public:
    explicit OktetaDocumentFactory( OktetaPlugin* plugin );

  public: // IDocumentFactory API
    IDocument* create( const QUrl &url, ICore* core ) override;

  protected:
    OktetaPlugin* mPlugin;
};


inline OktetaDocumentFactory::OktetaDocumentFactory( OktetaPlugin* plugin )
  : mPlugin( plugin )
{
}

inline IDocument* OktetaDocumentFactory::create( const QUrl &url, ICore* core )
{
    auto* document = new OktetaDocument( url, core );
    document->setPlugin( mPlugin );
//             m_plugin->activateDocument(d);
    return document;
}

}

#endif
