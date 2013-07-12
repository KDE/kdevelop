/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
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

#ifndef KDEVPLATFORM_KROSSPLUGIN_H
#define KDEVPLATFORM_KROSSPLUGIN_H

#include <interfaces/iplugin.h>

#include <kross/core/action.h>

#include "krossbuildsystemmanager.h"
#include "krossdistributedversioncontrol.h"

namespace Kross { class Action; }
namespace KDevelop { class IDocument; }
class KrossToolViewFactory;
class QToolBar;
class QMainWindow;
class KAboutData;

class KrossPlugin : public KDevelop::IPlugin, public KrossBuildSystemManager, public KrossDistributedVersionControl
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )
    Q_INTERFACES( KDevelop::IProjectFileManager )
    Q_INTERFACES( KDevelop::IDistributedVersionControl )
    public:
        explicit KrossPlugin( const QString& scriptfile, const KAboutData& about, QObject* parent = 0);
        virtual ~KrossPlugin();

        KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
        
        Q_SCRIPTABLE void createToolViewFactory(const QString& method, const QString& id, Qt::DockWidgetArea pos);
        Q_SCRIPTABLE KUrl pluginDirectory() const { return m_pluginDir; }
        
        Q_SCRIPTABLE static QToolBar* createToolBar(const QString& name, QMainWindow* parent);
    private:
        QList<KrossToolViewFactory*> m_toolFactories;
        Kross::Action* action;

        KUrl m_pluginDir;
};

#endif
