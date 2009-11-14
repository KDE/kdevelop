/**************************************************************************
*   Copyright 2009 Andreas Pakulat <apaku@gmx.de                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License as       *
*   published by the Free Software Foundation; either version 2 of the    *
*   License, or (at your option) any later version.                       *
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

#include "loadedpluginsdialog.h"

#include <klocale.h>
#include <kglobal.h>
#include <ktitlewidget.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <QVBoxLayout>
#include <ktextbrowser.h>
#include "core.h"
#include "plugincontroller.h"
#include <qscrollbar.h>
#include <kdebug.h>

LoadedPluginsDialog::LoadedPluginsDialog( QWidget* parent )
    : KDialog( parent )
{
    setPlainCaption(i18n("Loaded Plugins"));
    setButtons(KDialog::Close);
    setDefaultButton(KDialog::Close);
    
    QVBoxLayout* vbox = new QVBoxLayout(mainWidget());
    
    KTitleWidget* title = new KTitleWidget(this);
    title->setPixmap(KIcon(KGlobal::mainComponent().aboutData()->programIconName()), KTitleWidget::ImageLeft);
    title->setText(i18n("<html><font size=\"4\">Plugins loaded for <b>%1</b></font></html>", KGlobal::mainComponent().aboutData()->programName()));
    vbox->addWidget(title);
    
    KTextBrowser* info = new KTextBrowser(this);
    QStringList loadedpluginsinfo;
    foreach( KDevelop::IPlugin* plugin,  KDevelop::Core::self()->pluginControllerInternal()->loadedPlugins() )
    {
        loadedpluginsinfo << i18n("Plugin Name: <b>%1</b><br />Plugin Description: %2<br/>",
                                  plugin->componentData().aboutData()->programName(),
                                  plugin->componentData().aboutData()->shortDescription() );
    }
    info->setHtml(i18n("<html>%1</html>", 
                       loadedpluginsinfo.join("<br/>")));
    QPalette transparentBackgroundPalette;
    transparentBackgroundPalette.setColor(QPalette::Base, Qt::transparent);
    transparentBackgroundPalette.setColor(QPalette::Text, transparentBackgroundPalette.color(QPalette::WindowText));
    info->setPalette(transparentBackgroundPalette);
    vbox->addWidget(info);
}
