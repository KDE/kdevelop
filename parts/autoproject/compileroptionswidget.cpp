/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "compileroptionswidget.h"

#include <qcombobox.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <kdebug.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <ktrader.h>

#include "kdevcompileroptions.h"
#include "autoprojectpart.h"


class ServiceComboBox
{
public:
    static void insertStringList(QComboBox *combo, const QValueList<KService::Ptr> &list,
                                 QStringList *names, QStringList *execs)
    {
        QValueList<KService::Ptr>::ConstIterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            combo->insertItem((*it)->comment());
            (*names) << (*it)->name();
            (*execs) << (*it)->exec();
            kdDebug() << "insertStringList item " << (*it)->name() << "," << (*it)->exec() << endl;
        }
    }
    static QString currentText(QComboBox *combo, QStringList *names)
    {
        if (combo->currentItem() == -1)
            return QString::null;
        return (*names)[combo->currentItem()];
    }
    static void setCurrentText(QComboBox *combo, const QString &str, QStringList *names)
    {
        QStringList::ConstIterator it;
        int i = 0;
        for (it = names->begin(); it != names->end(); ++it) {
            if (*it == str) {
                combo->setCurrentItem(i);
                break;
            }
            ++i;
        }
    }
};


CompilerOptionsWidget::CompilerOptionsWidget(AutoProjectPart *part, QWidget *parent, const char *name)
    : CompilerOptionsWidgetBase(parent, name)
{
    m_part = part;

    QDomDocument dom = *m_part->projectDom();

    KTrader::OfferList coffers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C'");
    KTrader::OfferList cxxoffers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C++'");
    KTrader::OfferList f77offers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Fortran'");

    ServiceComboBox::insertStringList(cservice_combo, coffers, &cservice_names, &cservice_execs);
    ServiceComboBox::insertStringList(cxxservice_combo, cxxoffers, &cxxservice_names, &cxxservice_execs);
    ServiceComboBox::insertStringList(f77service_combo, f77offers, &f77service_names, &f77service_execs);
    
    ServiceComboBox::setCurrentText(cservice_combo,
                                    DomUtil::readEntry(dom, "/kdevautoproject/compiler/ccompiler"),
                                    &cservice_names);
    ServiceComboBox::setCurrentText(cxxservice_combo,
                                    DomUtil::readEntry(dom, "/kdevautoproject/compiler/cxxcompiler"),
                                    &cxxservice_names);
    ServiceComboBox::setCurrentText(f77service_combo,
                                    DomUtil::readEntry(dom, "/kdevautoproject/compiler/f77compiler"),
                                    &f77service_names);

    if (coffers.isEmpty())
        cflags_button->setEnabled(false);
    if (cxxoffers.isEmpty())
        cxxflags_button->setEnabled(false);
    if (f77offers.isEmpty())
        f77flags_button->setEnabled(false);

    cbinary_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/compiler/ccompilerbinary"));
    cxxbinary_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/compiler/cxxcompilerbinary"));
    f77binary_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/compiler/f77compilerbinary"));
    
    cflags_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/compiler/cflags"));
    cxxflags_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/compiler/cxxflags"));
    f77flags_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/compiler/f77flags"));
}


CompilerOptionsWidget::~CompilerOptionsWidget()
{}


void CompilerOptionsWidget::accept()
{
    QDomDocument dom = *m_part->projectDom();

    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/ccompiler",
                        ServiceComboBox::currentText(cservice_combo, &cservice_names));
    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/cxxcompiler",
                        ServiceComboBox::currentText(cxxservice_combo, &cxxservice_names));
    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/f77compiler",
                        ServiceComboBox::currentText(f77service_combo, &f77service_names));

    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/ccompilerbinary", cbinary_edit->text());
    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/cxxcompilerbinary", cxxbinary_edit->text());
    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/f77compilerbinary", f77binary_edit->text());

    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/cflags", cflags_edit->text());
    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/cxxflags", cxxflags_edit->text());
    DomUtil::writeEntry(dom, "/kdevautoproject/compiler/f77flags", f77flags_edit->text());

    if (KMessageBox::questionYesNo(this, i18n("Rerun configure now?")) == KMessageBox::Yes)
        QTimer::singleShot(0, m_part, SLOT(slotConfigure()));
}


void CompilerOptionsWidget::cserviceChanged()
{
    QString exec = ServiceComboBox::currentText(cservice_combo, &cservice_execs);
    cbinary_edit->setText(exec);
    kdDebug() << "exec: " << exec << endl;
}


void CompilerOptionsWidget::cxxserviceChanged()
{
    QString exec = ServiceComboBox::currentText(cxxservice_combo, &cxxservice_execs);
    cxxbinary_edit->setText(exec);
}


void CompilerOptionsWidget::f77serviceChanged()
{
    QString exec = ServiceComboBox::currentText(f77service_combo, &f77service_execs);
    f77binary_edit->setText(exec);
    kdDebug() << "exec: " << exec << endl;
}


void CompilerOptionsWidget::cflagsClicked()
{
    QString name = ServiceComboBox::currentText(cservice_combo, &cservice_names);
    KDevCompilerOptions *plugin = createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, cflags_edit->text());
        if (!flags.isNull())
            cflags_edit->setText(flags);
        delete plugin;
    }
}


void CompilerOptionsWidget::cxxflagsClicked()
{
    QString name = ServiceComboBox::currentText(cxxservice_combo, &cxxservice_names);
    KDevCompilerOptions *plugin = createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, cxxflags_edit->text());
        if (!flags.isNull())
            cxxflags_edit->setText(flags);
        delete plugin;
    }
}


void CompilerOptionsWidget::f77flagsClicked()
{
    QString name = ServiceComboBox::currentText(f77service_combo, &f77service_names);
    KDevCompilerOptions *plugin = createCompilerOptions(name);

    if (plugin) {
        QString flags = plugin->exec(this, f77flags_edit->text());
        if (!flags.isNull())
            f77flags_edit->setText(flags);
        delete plugin;
    }
}


KDevCompilerOptions *CompilerOptionsWidget::createCompilerOptions(const QString &name)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service) {
        kdDebug(9000) << "Can't find service " << name;
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
        kdDebug(9000) << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;
    
    return dlg;
}

#include "compileroptionswidget.moc"
