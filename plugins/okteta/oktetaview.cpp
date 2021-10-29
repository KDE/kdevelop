/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "oktetaview.h"

// lib
#include "oktetaplugin.h"
#include "oktetadocument.h"
#include "oktetawidget.h"
// Okteta Kasten
#include <Kasten/Okteta/ByteArrayView>


namespace KDevelop
{

OktetaView::OktetaView( OktetaDocument* document, Kasten::ByteArrayViewProfileSynchronizer* viewProfileSynchronizer )
  : Sublime::View( document, View::TakeOwnership ),
    mByteArrayView( new Kasten::ByteArrayView( document->byteArrayDocument(), viewProfileSynchronizer ) )
{
}

QWidget* OktetaView::createWidget( QWidget* parent )
{
    OktetaPlugin* plugin = static_cast<OktetaDocument*>( document() )->plugin();

    return new OktetaWidget( parent, mByteArrayView, plugin );
}

OktetaView::~OktetaView()
{
    delete mByteArrayView;
}

}
