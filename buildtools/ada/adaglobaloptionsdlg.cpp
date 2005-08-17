/* Copyright (C) 2003 Oliver Kellogg
 * okellogg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include <kurlrequester.h>
#include <kdebug.h>
#include <kconfig.h>

#include "kdevcompileroptions.h"

#include "service.h"
#include "adaproject_part.h"
#include "adaglobaloptionsdlg.h"

AdaGlobalOptionsDlg::AdaGlobalOptionsDlg(AdaProjectPart *part, QWidget* parent, const char* name, Qt::WFlags fl)
    :AdaProjectOptionsDlgBase(parent,name,fl), m_part(part)
{
    delete config_label;
    delete config_combo;
    delete addconfig_button;
    delete removeconfig_button;
    delete compiler_label;
    delete configuration_layout;
    delete configuration_line;
    delete exec_label;
    delete exec_edit;
    delete mainSourceLabel;
    delete mainSourceUrl;
    delete defaultopts_button;

    offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Ada'");

    ServiceComboBox::insertStringList(compiler_box, offers, &service_names, &service_execs);

    if (offers.isEmpty())
        options_button->setEnabled(false);

    currentCompiler = QString::null;

    /*kdDebug() << ServiceComboBox::defaultCompiler() << endl;
    kdDebug() << ServiceComboBox::itemForText(ServiceComboBox::defaultCompiler(), service_names) << endl;
    kdDebug() << compiler_box->text(ServiceComboBox::itemForText(ServiceComboBox::defaultCompiler(), service_names)) << endl;
*/
    ServiceComboBox::setCurrentText(compiler_box, ServiceComboBox::defaultCompiler(), service_names);
    compiler_box_activated(compiler_box->currentText());
}

AdaGlobalOptionsDlg::~AdaGlobalOptionsDlg()
{
}

void AdaGlobalOptionsDlg::optionsButtonClicked()
{
    QString name = ServiceComboBox::currentText(compiler_box, service_names);
    KDevCompilerOptions *plugin = m_part->createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, options_edit->text());
        options_edit->setText(flags);
        delete plugin;
    }
}

void AdaGlobalOptionsDlg::compiler_box_activated(const QString& text)
{
    kdDebug() << "text changed from " << currentCompiler << " to " << text << endl;
    if (currentCompiler == text)
        return;
    if (!currentCompiler.isEmpty())
        saveCompilerOpts(currentCompiler);
    currentCompiler = text;
    readCompilerOpts(currentCompiler);
}

void AdaGlobalOptionsDlg::accept()
{
    saveCompilerOpts(currentCompiler);

    saveConfigCache();
}

void AdaGlobalOptionsDlg::saveCompilerOpts( QString compiler )
{
    configCache[compiler] = options_edit->text();
}

void AdaGlobalOptionsDlg::readCompilerOpts( QString compiler )
{
    QString settings = configCache[compiler];
    if (settings.isEmpty())
    {
        KConfig *config = KGlobal::config();
        config->setGroup("Ada Compiler");
        settings = config->readPathEntry(compiler);
    }

    options_edit->setText(settings);
}

void AdaGlobalOptionsDlg::readConfigCache( )
{
/*    KConfig *config = KGlobal::config();
    config->setGroup("Ada Compiler");

    QMap<QString, QString> settings = config->entryMap("Ada Compiler");
*/
}

void AdaGlobalOptionsDlg::saveConfigCache( )
{
    KConfig *config = KGlobal::config();
    config->setGroup("Ada Compiler");

    for (QMap<QString, QString>::iterator it = configCache.begin(); it != configCache.end(); ++it)
    {
        config->writeEntry(it.key(), it.data());
    }
}

#include "adaglobaloptionsdlg.moc"
