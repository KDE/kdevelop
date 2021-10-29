/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OKTETAWIDGET_H
#define OKTETAWIDGET_H

// KF
#include <KXMLGUIClient>
// Qt
#include <QWidget>

namespace Kasten {
class ByteArrayView;
class AbstractXmlGuiController;
}


namespace KDevelop
{
class OktetaPlugin;

class OktetaWidget : public QWidget, public KXMLGUIClient
{
  Q_OBJECT

  public:
    OktetaWidget( QWidget* parent, Kasten::ByteArrayView* byteArrayView, OktetaPlugin* plugin );
    ~OktetaWidget() override;

  private:
    void setupActions(OktetaPlugin* plugin);

  private:
    Kasten::ByteArrayView* mByteArrayView;
    // TODO: move this to the plugin, per mainwindow
    QList<Kasten::AbstractXmlGuiController*> mControllers;
};

}

#endif
