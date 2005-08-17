/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kurlrequester.h>
#include <kservice.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qvalidator.h>
//Added by qt3to4:
#include <Q3ValueList>

#include "domutil.h"
#include "kdevcompileroptions.h"

#include "service.h"
#include "pascalproject_part.h"
#include "pascalprojectoptionsdlg.h"

PascalProjectOptionsDlg::PascalProjectOptionsDlg(PascalProjectPart *part, QWidget* parent, const char* name, Qt::WFlags fl)
    : PascalProjectOptionsDlgBase(parent,name, fl), m_part(part)
{
    config_combo->setValidator(new QRegExpValidator(QRegExp("^\\D.*"), this));

    offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Pascal'");

    ServiceComboBox::insertStringList(compiler_box, offers, &service_names, &service_execs);

    if (offers.isEmpty())
        options_button->setEnabled(false);

    allConfigs = allBuildConfigs();
    config_combo->insertStringList(allConfigs);

    dirty = false;

    QDomDocument &dom = *(m_part->projectDom());
    currentConfig = QString::null;
    configChanged(DomUtil::readEntry(dom, "/kdevpascalproject/general/useconfiguration", "default"));
}

PascalProjectOptionsDlg::~PascalProjectOptionsDlg()
{
}

QStringList PascalProjectOptionsDlg::allBuildConfigs()
{
    QDomDocument &dom = *(m_part->projectDom());

    QStringList allConfigs;
    allConfigs.append("default");

    QDomNode node = dom.documentElement().namedItem("kdevpascalproject").namedItem("configurations");
    QDomElement childEl = node.firstChild().toElement();
    while (!childEl.isNull()) {
        QString config = childEl.tagName();
        kdDebug() << "Found config " << config << endl;
        if (config != "default")
            allConfigs.append(config);
        childEl = childEl.nextSibling().toElement();
    }

    return allConfigs;
}

void PascalProjectOptionsDlg::accept()
{
    DomUtil::writeEntry(*m_part->projectDom(), "/kdevpascalproject/general/useconfiguration", currentConfig);
    if (dirty)
    {
        saveConfig(currentConfig);
    }
}

void PascalProjectOptionsDlg::compiler_box_activated( const QString& /*s*/ )
{
    QString exec = ServiceComboBox::currentText(compiler_box, service_execs);
    exec_edit->setText(exec);
}

void PascalProjectOptionsDlg::saveConfig( QString config )
{
    QDomDocument dom = *m_part->projectDom();
    QString prefix = "/kdevpascalproject/configurations/" + config + "/";

    DomUtil::writeEntry(dom, prefix + "compiler",
                        ServiceComboBox::currentText(compiler_box, service_names));
    DomUtil::writeEntry(dom, prefix + "compileroptions", options_edit->text());
    DomUtil::writeEntry(dom, prefix + "compilerexec", exec_edit->text());
    DomUtil::writeEntry(dom, prefix + "mainsource", mainSourceUrl->url().replace(QRegExp(m_part->projectDirectory() + QString("/")),""));
}

void PascalProjectOptionsDlg::readConfig( QString config )
{
    QDomDocument dom = *m_part->projectDom();
    QString prefix = "/kdevpascalproject/configurations/" + config + "/";

    QString compiler = DomUtil::readEntry(dom, prefix + "compiler", "");

    if (compiler.isEmpty())
    {
        offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Pascal'");
        Q3ValueList<KService::Ptr>::ConstIterator it;
        for (it = offers.begin(); it != offers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                compiler = (*it)->name();
                kdDebug() << "compiler is " << compiler << endl;
                break;
            }
        }
    }
    ServiceComboBox::setCurrentText(compiler_box, compiler, service_names);

    QString exec = DomUtil::readEntry(dom, prefix + "compilerexec", "");
    if (exec.isEmpty())
        exec = ServiceComboBox::currentText(compiler_box, service_execs);
    exec_edit->setText(exec);
    options_edit->setText(DomUtil::readEntry(dom, prefix + "compileroptions"));
    mainSourceUrl->setURL(m_part->projectDirectory() + "/" + DomUtil::readEntry(dom, prefix + "mainsource"));
}

void PascalProjectOptionsDlg::configComboTextChanged(const QString &config)
{
    bool canAdd = !allConfigs.contains(config) && !config.contains("/") && !config.isEmpty();
    bool canRemove = allConfigs.contains(config) && config != "default";
    addconfig_button->setEnabled(canAdd);
    removeconfig_button->setEnabled(canRemove);
}


void PascalProjectOptionsDlg::configChanged(const QString &config)
{
    if (config == currentConfig || !allConfigs.contains(config))
        return;

    if (!currentConfig.isNull() && dirty)
        saveConfig(currentConfig);

    currentConfig = config;
    readConfig(config);
    dirty = false;

    config_combo->blockSignals(true);
    config_combo->setEditText(config);
    config_combo->blockSignals(false);
}


void PascalProjectOptionsDlg::configAdded()
{
    QString config = config_combo->currentText();

    allConfigs.append(config);

    config_combo->clear();
    config_combo->insertStringList(allConfigs);
    configChanged(config);
    setDirty(); // force saving
}


void PascalProjectOptionsDlg::configRemoved()
{
    QString config = config_combo->currentText();

    QDomDocument dom = *m_part->projectDom();
    QDomNode node = dom.documentElement().namedItem("kdevpascalproject").namedItem("configurations");
    node.removeChild(node.namedItem(config));
    allConfigs.remove(config);

    config_combo->clear();
    config_combo->insertStringList(allConfigs);

    currentConfig = QString::null;
    configChanged("default");
}

void PascalProjectOptionsDlg::optionsButtonClicked( )
{
    QString name = ServiceComboBox::currentText(compiler_box, service_names);
    KDevCompilerOptions *plugin = m_part->createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, options_edit->text());
        options_edit->setText(flags);
        delete plugin;
    }
}

void PascalProjectOptionsDlg::setDirty( )
{
    dirty = true;
}

void PascalProjectOptionsDlg::setDefaultOptions( )
{
    if (!compiler_box->currentText().isEmpty())
        options_edit->setText(m_part->defaultOptions(compiler_box->currentText()));
}

#include "pascalprojectoptionsdlg.moc"
