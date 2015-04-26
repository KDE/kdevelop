/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once

#include <interfaces/iplugin.h>


namespace KDevelop {
class Declaration;
}

class OutlineViewPlugin : public KDevelop::IPlugin {
    Q_OBJECT

public:
    explicit OutlineViewPlugin(QObject *parent, const QVariantList& args = QVariantList());
    virtual ~OutlineViewPlugin();

public: // KDevelop::Plugin overrides
    void unload() override;

private:
    class OutlineViewFactory* m_factory;
    Q_DISABLE_COPY(OutlineViewPlugin)
};
