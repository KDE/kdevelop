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
#include <qcombobox.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kapplication.h>
#include <kfontrequester.h>

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

    gsw->projectsURL->setMode((int)KFile::Directory);

    config->setGroup("General Options");
    gsw->lastProjectCheckbox->setChecked(config->readBoolEntry("Read Last Project On Startup",true));
    gsw->outputFont->setFont( config->readFontEntry( "OutputViewFont" ) );
    config->setGroup("MakeOutputView");
    gsw->lineWrappingCheckBox->setChecked(config->readBoolEntry("LineWrapping",true));
    gsw->dirNavigMsgCheckBox->setChecked(config->readBoolEntry("ShowDirNavigMsg",false));
    gsw->compileOutputCombo->setCurrentItem(config->readNumEntry("CompilerOutputLevel",2));
    config->setGroup("General Options");
    gsw->projectsURL->setURL(config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/"));
    gsw->designerButtonGroup->setButton( config->readNumEntry( "DesignerApp", 0 ) );

    config->setGroup("TerminalEmulator");
    gsw->terminalButtonGroup->setButton( config->readNumEntry( "UseKDESetting", 0 ) );
    gsw->terminalEdit->setText( config->readEntry( "TerminalApplication", QString::fromLatin1("konsole") ) );
}

void KDevIDEExtension::acceptGlobalSettingsPage(KDialogBase *dlg)
{
    KConfig* config = kapp->config();

    config->setGroup("General Options");
    config->writeEntry("DesignerApp", gsw->designerButtonGroup->selectedId());
    config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
    config->writePathEntry("DefaultProjectsDir", gsw->projectsURL->url());
    config->writeEntry("OutputViewFont", gsw->outputFont->font());
    config->setGroup("MakeOutputView");
    config->writeEntry("LineWrapping",gsw->lineWrappingCheckBox->isChecked());
    config->writeEntry("ShowDirNavigMsg",gsw->dirNavigMsgCheckBox->isChecked());
    //current item id must be in sync with the enum!
    config->writeEntry("CompilerOutputLevel",gsw->compileOutputCombo->currentItem());
    config->sync();
    if( KDevPlugin *makeExt = API::getInstance()->pluginController()->extension("KDevelop/MakeFrontend"))
    {
        static_cast<KDevMakeFrontend*>(makeExt)->updateSettingsFromConfig();
    }

    config->setGroup("TerminalEmulator");
    config->writeEntry("UseKDESetting", gsw->useKDETerminal->isChecked() );
    config->writeEntry("TerminalApplication", gsw->terminalEdit->text().stripWhiteSpace() );
}

QString KDevIDEExtension::xmlFile()
{
    return "kdevelopui.rc";
}

QString KDevIDEExtension::defaultProfile()
{
    return "IDE";
}
