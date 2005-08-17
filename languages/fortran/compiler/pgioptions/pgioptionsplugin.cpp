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

#include <qapplication.h>
#include <q3header.h>
#include <qlayout.h>
#include <q3listview.h>
#include <qradiobutton.h>
#include <q3vbox.h>
#include <qvbuttongroup.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QEvent>
#include <QBoxLayout>
#include <kdebug.h>
#include <klocale.h>

#include "pgioptionsplugin.h"


const char * const optimization_flags[] = {
    "-Mautopar",
    "-Mextract",
    "-Minline",
    "-Mipa",
    0
};


const char * const hpf_flags[] = {
    "-Mbackslash",
    "-Mcmf",
    "-Mdclchk",
    "-Mextend",
    "-Mf90",
    "-Mnofree",
    "-Mstandard",
    "-Mupcase",
    "-Mbyteswapio",
    "-Mdepchk",
    "-Mdlines",
    "-Mg",
    "-Mftn",
    "-Minfo",
    "-Minform",
    "-Mkeepft",
    "-Mkeepstaticn",
    "-Mmpi",
    "-Mmpl",
    "-Mlist",
    "-Mnohpfc",
    "-Mnoindependent",
    "-Mnoprelink",
    "-Moverlap",
    "-Mpreprocess",
    "-Mprof",
    "-Mpvm",
    "-Mr8",
    "-Mrecursive",
    "-Mreplicate",
    "-Mrpm",
    "-Mrpm1",
    "-M[no]sequence",
    "-Msmp",
    "-Mstats",
    0
};



class GeneralTab : public QWidget
{
public:
    GeneralTab( QWidget *parent=0, const char *name=0 );
    ~GeneralTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);
};


class OptimizationTab : public QWidget
{
public:
    OptimizationTab( PgiOptionsPlugin::Type type, QWidget *parent=0, const char *name=0 );
    ~OptimizationTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    QRadioButton *Odefault, *O0, *O1, *O2;
    Q3ListView *optBox;
};


class HpfTab : public QWidget
{
public:
    HpfTab( QWidget *parent=0, const char *name=0 );
    ~HpfTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    Q3ListView *hpfBox;
};


OptimizationTab::OptimizationTab(PgiOptionsPlugin::Type type, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    Q3VButtonGroup *group = new Q3VButtonGroup(i18n("Optimization Level"), this);
    Odefault = new QRadioButton(i18n("Default"), group);
    Odefault->setChecked(true);
    O0 = new QRadioButton(i18n("No optimization"), group);
    O1 = new QRadioButton(i18n("Level 1"), group);
    O2 = new QRadioButton(i18n("Level 2"), group);

    if (type == PgiOptionsPlugin::PGHPF) {
        optBox = new Q3ListView(this);
        optBox->addColumn(QString::null);
        optBox->header()->hide();
        for (const char * const *p = optimization_flags; *p; ++p) {
            new Q3CheckListItem(optBox, *p, Q3CheckListItem::CheckBox);
            kdDebug() << (*p) << endl;
        }
    } else
        optBox = 0;

    QApplication::sendPostedEvents(this, QEvent::ChildAdded);
    layout->addStretch();
}


OptimizationTab::~OptimizationTab()
{}


void OptimizationTab::readFlags(QStringList *list)
{
    if (optBox) {
        Q3ListViewItem *item = optBox->firstChild();
        for (; item; item = item->nextSibling()) {
            QStringList::Iterator sli = list->find(item->text(0));
            if (sli != list->end()) {
                static_cast<Q3CheckListItem*>(item)->setOn(true);
                list->remove(sli);
            }
        }
    }
    
    QStringList::Iterator sli;
    sli = list->find("-O0");
    if (sli != list->end()) {
        O0->setChecked(true);
        list->remove(sli);
    }
    sli = list->find("-O1");
    if (sli != list->end()) {
        O1->setChecked(true);
        list->remove(sli);
    }
    sli = list->find("-O2");
    if (sli != list->end()) {
        O2->setChecked(true);
        list->remove(sli);
    }
}


void OptimizationTab::writeFlags(QStringList *list)
{
    if (optBox) {
        Q3ListViewItem *item = optBox->firstChild();
        for (; item; item = item->nextSibling())
            if (static_cast<Q3CheckListItem*>(item)->isOn())
                (*list) << item->text(0);
    }
    
    if (O0->isChecked())
        (*list) << "-O0";
    else if (O1->isChecked())
        (*list) << "-O1";
    else if (O2->isChecked())
        (*list) << "-O2";
}


HpfTab::HpfTab(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    hpfBox = new Q3ListView(this);
    hpfBox->addColumn(QString::null);
    hpfBox->header()->hide();
    for (const char * const *p = hpf_flags; *p; ++p)
        new Q3CheckListItem(hpfBox, *p, Q3CheckListItem::CheckBox);

    QApplication::sendPostedEvents(this, QEvent::ChildAdded);
    layout->addStretch();
}


HpfTab::~HpfTab()
{}


void HpfTab::readFlags(QStringList *list)
{
    Q3ListViewItem *item = hpfBox->firstChild();
    for (; item; item = item->nextSibling()) {
        QStringList::Iterator sli = list->find(item->text(0));
        if (sli != list->end()) {
            static_cast<Q3CheckListItem*>(item)->setOn(true);
            list->remove(sli);
        }
    }
}


void HpfTab::writeFlags(QStringList *list)
{
    Q3ListViewItem *item = hpfBox->firstChild();
    for (; item; item = item->nextSibling()) {
        if (static_cast<Q3CheckListItem*>(item)->isOn())
            (*list) << item->text(0);
    }
}


PgiOptionsDialog::PgiOptionsDialog(PgiOptionsPlugin::Type type, QWidget *parent, const char *name)
    : KDialogBase(Tabbed, (type == PgiOptionsPlugin::PGHPF)? i18n("PGHPF Compiler Options") : i18n("PGF77 Compiler Options"),
                  Ok|Cancel, Ok, parent, name, true)
{
    Q3VBox *vbox;

    //    vbox = addVBoxPage(i18n("General"));
    //    general = new GeneralTab(vbox, "general tab");

    vbox = addVBoxPage(i18n("Optimization"));
    optimization = new OptimizationTab(type, vbox, "optimization tab");

    if (type == PgiOptionsPlugin::PGHPF) {
        vbox = addVBoxPage(i18n("HPF"));
        hpf = new HpfTab(vbox, "optimization tab");
    } else
        hpf = 0;
        
}


PgiOptionsDialog::~PgiOptionsDialog()
{
}


void PgiOptionsDialog::setFlags(const QString &flags)
{
    QStringList flaglist = QStringList::split(" ", flags);

    // Hand them to 'general' at last, so it can make a line edit
    // with the unprocessed items
    if (hpf)
        hpf->readFlags(&flaglist);
    optimization->readFlags(&flaglist);
    //    general->readFlags(&flaglist);
}


QString PgiOptionsDialog::flags() const
{
    QStringList flaglist;

    if (hpf)
        hpf->writeFlags(&flaglist);
    optimization->writeFlags(&flaglist);
    //    general->writeFlags(&flaglist);

    QString flags;
    QStringList::ConstIterator li;
    for (li = flaglist.begin(); li != flaglist.end(); ++li) {
        flags += (*li);
        flags += " ";
    }

    flags.truncate(flags.length()-1);
    return flags;
}


PgiOptionsPlugin::PgiOptionsPlugin(Type type, QObject *parent, const char *name)
    : KDevCompilerOptions(parent, name)
{
    pgitype = type;
}


PgiOptionsPlugin::~PgiOptionsPlugin()
{}


QString PgiOptionsPlugin::exec(QWidget *parent, const QString &flags)
{
    PgiOptionsDialog *dlg = new PgiOptionsDialog(pgitype, parent, "pgi options dialog");
    QString newFlags = flags;
    dlg->setFlags(flags);
    if (dlg->exec() == QDialog::Accepted)
        newFlags = dlg->flags();
    delete dlg;
    return newFlags;
}

#include "pgioptionsplugin.moc"
