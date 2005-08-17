/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "configureoptionswidget.h"

#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qvalidator.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <ktrader.h>

#include "kdevcompileroptions.h"
#include "autoprojectpart.h"
#include "environmentvariableswidget.h"


class ServiceComboBox
{
public:
    static void insertStringList(QComboBox *combo, const Q3ValueList<KService::Ptr> &list,
                                 QStringList *names, QStringList *execs)
    {
        Q3ValueList<KService::Ptr>::ConstIterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            combo->insertItem((*it)->comment());
            (*names) << (*it)->desktopEntryName();
            (*execs) << (*it)->exec();
            kdDebug(9020) << "insertStringList item " << (*it)->name() << "," << (*it)->exec() << endl;
        }
    }
    static QString currentText(QComboBox *combo, const QStringList &names)
    {
        if (combo->currentItem() == -1)
            return QString::null;
        return names[combo->currentItem()];
    }
    static void setCurrentText(QComboBox *combo, const QString &str, const QStringList &names)
    {
        QStringList::ConstIterator it;
        int i = 0;
        for (it = names.begin(); it != names.end(); ++it) {
            if (*it == str) {
                combo->setCurrentItem(i);
                break;
            }
            ++i;
        }
    }
};


ConfigureOptionsWidget::ConfigureOptionsWidget(AutoProjectPart *part, QWidget *parent, const char *name)
    : ConfigureOptionsWidgetBase(parent, name)
{
    config_combo->setValidator(new QRegExpValidator(QRegExp("^\\D.*"), this));

    m_part = part;
    env_groupBox->setColumnLayout( 1, Qt::Vertical );
    QDomDocument &dom = *part->projectDom();
    m_environmentVariablesWidget = new EnvironmentVariablesWidget(dom, "/kdevautoproject/general/envvars", env_groupBox);

    coffers   = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C'");
    cxxoffers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C++'");
    f77offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Fortran'");

    ServiceComboBox::insertStringList(cservice_combo, coffers, &cservice_names, &cservice_execs);
    ServiceComboBox::insertStringList(cxxservice_combo, cxxoffers, &cxxservice_names, &cxxservice_execs);
    ServiceComboBox::insertStringList(f77service_combo, f77offers, &f77service_names, &f77service_execs);

    if (coffers.isEmpty())
        cflags_button->setEnabled(false);
    if (cxxoffers.isEmpty())
        cxxflags_button->setEnabled(false);
    if (f77offers.isEmpty())
        f77flags_button->setEnabled(false);

    allConfigs = part->allBuildConfigs();
    config_combo->insertStringList(allConfigs);

    dirty = false;
    currentConfig = QString::null;
    configChanged(part->currentBuildConfig());

    fixLayout();
}


ConfigureOptionsWidget::~ConfigureOptionsWidget()
{}


void ConfigureOptionsWidget::fixLayout()
{
    int w1 = ccompiler_label->sizeHint().width();
    int w2 = cbinary_label->sizeHint().width();
    int w3 = cflags_label->sizeHint().width();
    int w4 = cxxcompiler_label->sizeHint().width();
    int w5 = cxxbinary_label->sizeHint().width();
    int w6 = cxxflags_label->sizeHint().width();
    int w7 = f77compiler_label->sizeHint().width();
    int w8 = f77binary_label->sizeHint().width();
    int w9 = f77flags_label->sizeHint().width();

    int w = QMAX(w1, QMAX(w2, w3));
    w = QMAX(w, QMAX(w4, w5));
    w = QMAX(w, QMAX(w6, w7));
    w = QMAX(w, QMAX(w8, w9));

    ccompiler_label->setMinimumWidth(w);
    cxxcompiler_label->setMinimumWidth(w);
    f77compiler_label->setMinimumWidth(w);
}


void ConfigureOptionsWidget::readSettings(const QString &config)
{
    QDomDocument dom = *m_part->projectDom();
    QString prefix = "/kdevautoproject/configurations/" + config + "/";
    kdDebug(9020) << "Reading config from " << prefix << endl;

    configargs_edit->setText(DomUtil::readEntry(dom, prefix + "configargs"));
    QString builddir = DomUtil::readEntry(dom, prefix + "builddir");
    if (builddir.isEmpty() && config != "default")
        builddir = config;
    builddir_edit->setText(builddir);

    topsourcedir_edit->setText(DomUtil::readEntry(dom, prefix + "topsourcedir"));

    cppflags_edit->setText(DomUtil::readEntry(dom, prefix + "cppflags"));
    ldflags_edit->setText(DomUtil::readEntry(dom, prefix + "ldflags"));

    QString ccompiler = DomUtil::readEntry(dom, prefix + "ccompiler");
    QString cxxcompiler = DomUtil::readEntry(dom, prefix + "cxxcompiler");
    QString f77compiler = DomUtil::readEntry(dom, prefix + "f77compiler");

    if (ccompiler.isEmpty()) {
        kdDebug(9020) << "No c compiler set" << endl;
        Q3ValueList<KService::Ptr>::ConstIterator it;
        for (it = coffers.begin(); it != coffers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                kdDebug(9020) << "Found default " << (*it)->name() << endl;
                ccompiler = (*it)->name();
                break;
            }
        }
    }
    if (cxxcompiler.isEmpty()) {
        kdDebug(9020) << "No cxx compiler set" << endl;
        Q3ValueList<KService::Ptr>::ConstIterator it;
        for (it = cxxoffers.begin(); it != cxxoffers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                kdDebug(9020) << "Found default " << (*it)->name() << endl;
                cxxcompiler = (*it)->name();
                break;
            }
        }
    }
    if (f77compiler.isEmpty()) {
        kdDebug(9020) << "No c compiler set" << endl;
        Q3ValueList<KService::Ptr>::ConstIterator it;
        for (it = f77offers.begin(); it != f77offers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                kdDebug(9020) << "Found default " << (*it)->name() << endl;
                f77compiler = (*it)->name();
                break;
            }
        }
    }

    ServiceComboBox::setCurrentText(cservice_combo, ccompiler, cservice_names);
    ServiceComboBox::setCurrentText(cxxservice_combo, cxxcompiler, cxxservice_names);
    ServiceComboBox::setCurrentText(f77service_combo, f77compiler, f77service_names);

    cbinary_edit->setText(DomUtil::readEntry(dom, prefix + "ccompilerbinary"));
    cxxbinary_edit->setText(DomUtil::readEntry(dom, prefix + "cxxcompilerbinary"));
    f77binary_edit->setText(DomUtil::readEntry(dom, prefix + "f77compilerbinary"));

    cflags_edit->setText(DomUtil::readEntry(dom, prefix + "cflags"));
    cxxflags_edit->setText(DomUtil::readEntry(dom, prefix + "cxxflags"));
    f77flags_edit->setText(DomUtil::readEntry(dom, prefix + "f77flags"));

    m_environmentVariablesWidget->readEnvironment(dom, prefix + "envvars");
}


void ConfigureOptionsWidget::saveSettings(const QString &config)
{
    m_environmentVariablesWidget->accept();
    QDomDocument dom = *m_part->projectDom();
    QString prefix = "/kdevautoproject/configurations/" + config + "/";
    kdDebug(9020) << "Saving config under " << prefix << endl;

    DomUtil::writeEntry(dom, prefix + "configargs", configargs_edit->text());
    DomUtil::writeEntry(dom, prefix + "builddir", builddir_edit->text());
    DomUtil::writeEntry(dom, prefix + "topsourcedir", topsourcedir_edit->text());

    DomUtil::writeEntry(dom, prefix + "cppflags", cppflags_edit->text());
    DomUtil::writeEntry(dom, prefix + "ldflags", ldflags_edit->text());

    QFileInfo fi(m_part->buildDirectory());
    QDir dir(fi.dir());
    dir.mkdir(fi.fileName());

    DomUtil::writeEntry(dom, prefix + "ccompiler",
                        ServiceComboBox::currentText(cservice_combo, cservice_names));
    DomUtil::writeEntry(dom, prefix + "cxxcompiler",
                        ServiceComboBox::currentText(cxxservice_combo, cxxservice_names));
    DomUtil::writeEntry(dom, prefix + "f77compiler",
                        ServiceComboBox::currentText(f77service_combo, f77service_names));

    DomUtil::writeEntry(dom, prefix + "ccompilerbinary", cbinary_edit->text());
    DomUtil::writeEntry(dom, prefix + "cxxcompilerbinary", cxxbinary_edit->text());
    DomUtil::writeEntry(dom, prefix + "f77compilerbinary", f77binary_edit->text());

    DomUtil::writeEntry(dom, prefix + "cflags", cflags_edit->text());
    DomUtil::writeEntry(dom, prefix + "cxxflags", cxxflags_edit->text());
    DomUtil::writeEntry(dom, prefix + "f77flags", f77flags_edit->text());

    if (KMessageBox::questionYesNo(this, i18n("Re-run configure for %1 now?").arg(config)) == KMessageBox::Yes)
        QTimer::singleShot(0, m_part, SLOT(slotConfigure()));

}


void ConfigureOptionsWidget::setDirty()
{
    kdDebug(9020) << "config dirty" << endl;
    dirty = true;
}


void ConfigureOptionsWidget::builddirClicked()
{
    QString dir = builddir_edit->text();
    dir = KFileDialog::getExistingDirectory(dir, this);
    if (!dir.isNull())
        builddir_edit->setText(dir);
}

void ConfigureOptionsWidget::topsourcedirClicked()
{
    QString dir = topsourcedir_edit->text();
    dir = KFileDialog::getExistingDirectory(dir, this);
    if (!dir.isNull())
        topsourcedir_edit->setText(dir);
}

void ConfigureOptionsWidget::configComboTextChanged(const QString &config)
{
    bool canAdd = !allConfigs.contains(config) && !config.contains("/") && !config.isEmpty();
    bool canRemove = allConfigs.contains(config) && config != "default";
    addconfig_button->setEnabled(canAdd);
    removeconfig_button->setEnabled(canRemove);
}


void ConfigureOptionsWidget::configChanged(const QString &config)
{
    if (config == currentConfig || !allConfigs.contains(config))
        return;

    if (!currentConfig.isNull() && dirty)
        saveSettings(currentConfig);

    currentConfig = config;
    readSettings(config);
    dirty = false;

    config_combo->blockSignals(true);
    config_combo->setEditText(config);
    config_combo->blockSignals(false);
}


void ConfigureOptionsWidget::configAdded()
{
    QString config = config_combo->currentText();

    allConfigs.append(config);

    config_combo->clear();
    config_combo->insertStringList(allConfigs);
    configChanged(config);
    setDirty(); // force saving
}


void ConfigureOptionsWidget::configRemoved()
{
    QString config = config_combo->currentText();

    QDomDocument dom = *m_part->projectDom();
    QDomNode node = dom.documentElement().namedItem("kdevautoproject").namedItem("configurations");
    node.removeChild(node.namedItem(config));
    allConfigs.remove(config);

    config_combo->clear();
    config_combo->insertStringList(allConfigs);

    currentConfig = QString::null;
    configChanged("default");
}


void ConfigureOptionsWidget::cserviceChanged()
{
    QString exec = ServiceComboBox::currentText(cservice_combo, cservice_execs);
    cbinary_edit->setText(exec);
    kdDebug(9020) << "exec: " << exec << endl;
}


void ConfigureOptionsWidget::cxxserviceChanged()
{
    QString exec = ServiceComboBox::currentText(cxxservice_combo, cxxservice_execs);
    cxxbinary_edit->setText(exec);
}


void ConfigureOptionsWidget::f77serviceChanged()
{
    QString exec = ServiceComboBox::currentText(f77service_combo, f77service_execs);
    f77binary_edit->setText(exec);
    kdDebug(9020) << "exec: " << exec << endl;
}


void ConfigureOptionsWidget::cflagsClicked()
{
    QString name = ServiceComboBox::currentText(cservice_combo, cservice_names);
    KDevCompilerOptions *plugin = createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, cflags_edit->text());
        cflags_edit->setText(flags);
        delete plugin;
    }
}


void ConfigureOptionsWidget::cxxflagsClicked()
{
    QString name = ServiceComboBox::currentText(cxxservice_combo, cxxservice_names);
    KDevCompilerOptions *plugin = createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, cxxflags_edit->text());
        cxxflags_edit->setText(flags);
        delete plugin;
    }
}


void ConfigureOptionsWidget::f77flagsClicked()
{
    QString name = ServiceComboBox::currentText(f77service_combo, f77service_names);
    KDevCompilerOptions *plugin = createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, f77flags_edit->text());
        f77flags_edit->setText(flags);
        delete plugin;
    }
}


KDevCompilerOptions *ConfigureOptionsWidget::createCompilerOptions(const QString &name)
{
    KService::Ptr service = KService::serviceByDesktopName(name);
    if (!service) {
        kdDebug(9020) << "Can't find service " << name;
        return 0;
    }

    KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
    if (!factory) {
        QString errorMessage = KLibLoader::self()->lastErrorMessage();
        KMessageBox::error(0, i18n("There was an error loading the module %1.\n"
                                   "The diagnostics is:\n%2").arg(service->name()).arg(errorMessage));
        exit(1);
    }

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(this, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug(9020) << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;

    return dlg;
}


void ConfigureOptionsWidget::accept()
{
    DomUtil::writeEntry(*m_part->projectDom(), "/kdevautoproject/general/useconfiguration", currentConfig);
    m_environmentVariablesWidget->accept();
    if (dirty)
    {
        saveSettings(currentConfig);
    }
}

#include "configureoptionswidget.moc"
