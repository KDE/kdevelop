/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
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

#include "qmljsparsejob.h"

#include <KPluginFactory>
#include <KAboutData>

#include <language/highlighting/codehighlighting.h>

#include "version.h"

K_PLUGIN_FACTORY(KDevQmlJsSupportFactory, registerPlugin<KDevQmlJsPlugin>(); )
K_EXPORT_PLUGIN(KDevQmlJsSupportFactory(
    KAboutData("kdevqmljssupport", 0, ki18n("QML/JS Support"), VERSION_STR,
    ki18n("Support for QML and JS Languages"), KAboutData::License_GPL)))

using namespace KDevelop;

KDevQmlJsPlugin::KDevQmlJsPlugin(QObject* parent, const QVariantList& )
: IPlugin( KDevQmlJsSupportFactory::componentData(), parent )
, ILanguageSupport()
, m_highlighting(new CodeHighlighting(this))
{
    KDEV_USE_EXTENSION_INTERFACE(ILanguageSupport)
}

ParseJob* KDevQmlJsPlugin::createParseJob(const IndexedString& url)
{
    return new QmlJsParseJob(url, this);
}

QString KDevQmlJsPlugin::name() const
{
    return "qml/js";
}

ICodeHighlighting* KDevQmlJsPlugin::codeHighlighting() const
{
    return m_highlighting;
}
