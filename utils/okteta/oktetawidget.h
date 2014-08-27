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

#ifndef OKTETAWIDGET_H
#define OKTETAWIDGET_H

// Plugin
#include "oktetaglobal.h"
// KDE
#include <KXMLGUIClient>
// Qt
#include <QWidget>

namespace KASTEN_NAMESPACE {
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
    virtual ~OktetaWidget();
#if 0
    private slots:
        void save();
        void editWidgets();
        void editBuddys();
        void editSignals();
        void editTabOrder();
#endif
  private:
    void setupActions(OktetaPlugin* plugin);

  private:
    Kasten::ByteArrayView* mByteArrayView;
    // TODO: move this to the plugin, per mainwindow
    QList<Kasten::AbstractXmlGuiController*> mControllers;
};

}

#endif
