/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "kdevideextension.h"

#include <qvbox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kapplication.h>

#include <kdevplugin.h>
#include <kdevmakefrontend.h>
#include <kdevplugincontroller.h>

#include "api.h"
#include "settingswidget.h"

KDevIDEExtension::KDevIDEExtension()
 : ShellExtension()
{
}

void KDevIDEExtension::init()
{
    s_instance = new KDevIDEExtension();
}

void KDevIDEExtension::createGlobalSettingsPage(KDialogBase *dlg)
{
    KConfig* config = kapp->config();
    QVBox *vbox = dlg->addVBoxPage(i18n("General"), i18n("General"), BarIcon("kdevelop", KIcon::SizeMedium) );
    gsw = new SettingsWidget(vbox, "general settings widget");

    gsw->projects_url->setMode((int)KFile::Directory);

    config->setGroup("General Options");
    gsw->lastProjectCheckbox->setChecked(config->readBoolEntry("Read Last Project On Startup",true));
    config->setGroup("MakeOutputView");
    gsw->setMessageFont(config->readFontEntry("Messages Font"));
    gsw->lineWrappingCheckBox->setChecked(config->readBoolEntry("LineWrapping",true));
    gsw->dirNavigMsgCheckBox->setChecked(config->readBoolEntry("ShowDirNavigMsg",false));
    gsw->compilerOutputButtonGroup->setRadioButtonExclusive(true);
    gsw->compilerOutputButtonGroup->setButton(config->readNumEntry("CompilerOutputLevel",0));
    config->setGroup("General Options");
    gsw->setApplicationFont(config->readFontEntry("Application Font"));
    gsw->changeMessageFontButton->setText(gsw->messageFont().family());
    gsw->changeMessageFontButton->setFont(gsw->messageFont());
    gsw->changeApplicationFontButton->setText(gsw->applicationFont().family());
    gsw->changeApplicationFontButton->setFont(gsw->applicationFont());
    gsw->projects_url->setURL(config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/"));
    gsw->embedDesignerCheckBox->setChecked(config->readBoolEntry("Embed KDevDesigner", true));    
}

void KDevIDEExtension::acceptGlobalSettingsPage(KDialogBase *dlg)
{
    KConfig* config = kapp->config();
    
    config->setGroup("General Options");
    config->writeEntry("Embed KDevDesigner", gsw->embedDesignerCheckBox->isChecked());
    config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
    config->writePathEntry("DefaultProjectsDir", gsw->projects_url->url());
    config->writeEntry("Application Font", gsw->applicationFont());
    config->setGroup("MakeOutputView");
    config->writeEntry("Messages Font",gsw->messageFont());
    config->writeEntry("LineWrapping",gsw->lineWrappingCheckBox->isChecked());
    config->writeEntry("ShowDirNavigMsg",gsw->dirNavigMsgCheckBox->isChecked());
    QButton* pSelButton = gsw->compilerOutputButtonGroup->selected();
    config->writeEntry("CompilerOutputLevel",gsw->compilerOutputButtonGroup->id(pSelButton)); // id must be in sync with the enum!
    config->sync();
    if( KDevPlugin *makeExt = API::getInstance()->pluginController()->extension("KDevelop/MakeFrontend"))
    {
        static_cast<KDevMakeFrontend*>(makeExt)->updateSettingsFromConfig();
    }
}

QString KDevIDEExtension::xmlFile()
{
    return "kdevelopui.rc";
}

QString KDevIDEExtension::defaultProfile()
{
    return "KDevelop";
}
