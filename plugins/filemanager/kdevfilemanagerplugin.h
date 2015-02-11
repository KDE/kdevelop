/***************************************************************************
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
#ifndef KDEVPLATFORM_PLUGIN_KDEVFILEMANAGERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVFILEMANAGERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QtCore/QVariant>

class FileManager;

class KDevFileManagerPlugin: public KDevelop::IPlugin {
    Q_OBJECT
public:
    KDevFileManagerPlugin(QObject *parent, const QVariantList &args = QVariantList() );
    ~KDevFileManagerPlugin();

    virtual void unload() override;
private slots:
    void init();

private:
    class KDevFileManagerViewFactory *m_factory;

};

#endif

