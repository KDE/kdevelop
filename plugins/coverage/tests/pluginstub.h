/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef TESTSTUBS_PLUGINSTUB_H
#define TESTSTUBS_PLUGINSTUB_H

#include "interfaces/iplugin.h"
#include "interfaces/contextmenuextension.h"
#include "../coverageexport.h"

namespace TestStubs
{
class VERITAS_COVERAGE_EXPORT Plugin : public KDevelop::IPlugin
{
Q_OBJECT
public:
    Plugin(const KComponentData &instance, QObject *parent) : KDevelop::IPlugin(instance, parent) {}
    virtual ~Plugin() {}
    virtual bool isCentralPlugin() const { Q_ASSERT(0); return true; }
    virtual void unload() { Q_ASSERT(0); }
    KIconLoader* iconLoader() const { Q_ASSERT(0); return 0; }
    KDevelop::ICore *core() const { Q_ASSERT(0); return 0; }
    virtual KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* context ) { return KDevelop::ContextMenuExtension(); }

/*public Q_SLOTS:
    void newIconLoader() const;

protected:
    void addExtension( const QString& );
    virtual void initializeGuiState();*/
};
}

#endif
