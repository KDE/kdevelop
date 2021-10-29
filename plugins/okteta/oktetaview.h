/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OKTETAVIEW_H
#define OKTETAVIEW_H

// KDevPlatform
#include <sublime/view.h>

namespace Kasten {
class ByteArrayViewProfileSynchronizer;
class ByteArrayView;
}


namespace KDevelop
{
class OktetaDocument;


class OktetaView : public Sublime::View
{
  Q_OBJECT

  public:
    OktetaView( OktetaDocument* document, Kasten::ByteArrayViewProfileSynchronizer* viewProfileSynchronizer );
    ~OktetaView() override;

  public:
    Kasten::ByteArrayView* byteArrayView() const;

  protected: // Sublime::View API
    QWidget* createWidget( QWidget* parent = nullptr ) override;

  protected:
    Kasten::ByteArrayView* mByteArrayView;
};


inline Kasten::ByteArrayView* OktetaView::byteArrayView() const { return mByteArrayView; }

}

#endif

