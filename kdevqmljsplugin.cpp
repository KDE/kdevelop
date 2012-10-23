/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "kdevqmljsplugin.h"
#include <KPluginFactory>
#include <KAboutData>

K_PLUGIN_FACTORY(KDevQmlJsSupportFactory, registerPlugin<KDevQmlJsPlugin>(); )
K_EXPORT_PLUGIN(KDevQmlJsSupportFactory(KAboutData("kdevqmljsplugin","kdevqmljs", ki18n("QML/JS Support"), "0.1", ki18n("Support for QML and JS Languages"), KAboutData::License_GPL)))

KDevQmlJsPlugin::KDevQmlJsPlugin(QObject* parent, const QVariantList& )
     : KDevelop::IPlugin( KDevQmlJsSupportFactory::componentData(), parent )
     , KDevelop::ILanguageSupport()
{}

KDevelop::ParseJob* KDevQmlJsPlugin::createParseJob(const KUrl& url)
{
    return 0;
}

QString KDevQmlJsPlugin::name() const
{
    return "qml/js";
}
