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

#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <kdebug.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <ktrader.h>

#include "kdevcompileroptions.h"
#include "autoprojectpart.h"
#include "compileroptionswidget.h"


class ServiceComboBox
{
public:
    static void insertStringList(QComboBox *combo, const QValueList<KService::Ptr> &list, QStringList *names)
    {
        QValueList<KService::Ptr>::ConstIterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            combo->insertItem((*it)->comment());
            (*names) << (*it)->name();
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

    QDomDocument doc = *m_part->document();

    KTrader::OfferList coffers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C'");
    KTrader::OfferList cxxoffers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C++'");
    KTrader::OfferList f77offers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Fortran'");

    ServiceComboBox::insertStringList(cservice_combo, coffers, &cservice_names);
    ServiceComboBox::insertStringList(cxxservice_combo, cxxoffers, &cxxservice_names);
    ServiceComboBox::insertStringList(f77service_combo, f77offers, &f77service_names);
    
    ServiceComboBox::setCurrentText(cservice_combo,
                                    DomUtil::readEntry(doc, "/kdevautoproject/compiler/ccompiler"),
                                    &cservice_names);
    ServiceComboBox::setCurrentText(cxxservice_combo,
                                    DomUtil::readEntry(doc, "/kdevautoproject/compiler/cxxcompiler"),
                                    &cxxservice_names);
    ServiceComboBox::setCurrentText(f77service_combo,
                                    DomUtil::readEntry(doc, "/kdevautoproject/compiler/f77compiler"),
                                    &f77service_names);

    if (coffers.isEmpty())
        cflags_button->setEnabled(false);
    if (cxxoffers.isEmpty())
        cxxflags_button->setEnabled(false);
    if (f77offers.isEmpty())
        f77flags_button->setEnabled(false);

    cbinary_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/compiler/ccompilerbinary"));
    cxxbinary_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/compiler/cxxcompilerbinary"));
    f77binary_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/compiler/f77compilerbinary"));
    
    cflags_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/compiler/cflags"));
    cxxflags_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/compiler/cxxflags"));
    f77flags_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/compiler/f77flags"));
}


CompilerOptionsWidget::~CompilerOptionsWidget()
{}


void CompilerOptionsWidget::accept()
{
    QDomDocument doc = *m_part->document();

    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/ccompiler",
                        ServiceComboBox::currentText(cservice_combo, &cservice_names));
    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/cxxcompiler",
                        ServiceComboBox::currentText(cxxservice_combo, &cxxservice_names));
    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/f77compiler",
                        ServiceComboBox::currentText(f77service_combo, &f77service_names));

    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/ccompilerbinary", cbinary_edit->text());
    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/cxxcompilerbinary", cxxbinary_edit->text());
    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/f77compilerbinary", f77binary_edit->text());

    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/cflags", cflags_edit->text());
    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/cxxflags", cxxflags_edit->text());
    DomUtil::writeEntry(doc, "/kdevautoproject/compiler/f77flags", f77flags_edit->text());

    if (KMessageBox::questionYesNo(this, i18n("Rerun configure now?")) == KMessageBox::Yes)
        m_part->slotConfigure();
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
    
    KLibFactory *factory = KLibLoader::self()->factory(service->library());

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
