/***************************************************************************
                          LdLinkerOptionsWidget.cpp  -  description
                             -------------------
    begin                : Mon Feb 5 2001
    copyright            : (C) 2001 by Omid Givi & Bernd Gehrmann
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "LdLinkerOptionsWidget.h"
#include "LdLinkerOptionsWidgetBase.h"
#include <qlineedit.h>
#include <qobject.h>
#include "kdebug.h"
#include <qwidget.h>
#include <qvbuttongroup.h>
#include <qvbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qvbox.h>
#include <kdialog.h>

LdLinkerOptionsWidget::LdLinkerOptionsWidget(QWidget *parent, const char *name, QWidget* pdlg)
	 : LdLinkerOptionsWidgetBase(parent,name) {
	linkerFlagsPreview->setReadOnly(true);
	connect(pdlg, SIGNAL(WidgetStarted()), this, SLOT(slotWidgetStarted()));
	connect(pdlg, SIGNAL(ButtonApplyClicked()), this, SLOT(slotButtonApplyClicked()));
}

LdLinkerOptionsWidget::~LdLinkerOptionsWidget(){
}

void LdLinkerOptionsWidget::slotFlagsToolButtonClicked(){
 	LdOptionsDlg *dlg = new LdOptionsDlg(this, "Ld Project Options Dialog");
 	linkerFlagsPreview->setText(dlg->setFlags(linkerFlagsPreview->text()));
 	dlg->exec();
 	linkerFlagsPreview->setText(dlg->flags() + " " + linkerFlagsPreview->text());
 	delete dlg;
}

// reads the Linker flags
void LdLinkerOptionsWidget::slotWidgetStarted(){
 	linkerFlagsPreview->setText(*(m_pKDevLinker->flags()));
}

// writess the Linker flags
void LdLinkerOptionsWidget::slotButtonApplyClicked(){
 	if (ldLinkerFlags->text() != ""){
		linkerFlagsPreview->setText(linkerFlagsPreview->text() + " " + ldLinkerFlags->text());
		ldLinkerFlags->setText("");
	}
 	m_pKDevLinker->setFlags(linkerFlagsPreview->text());
}

void LdLinkerOptionsWidget::slotClearAllClicked(){
  linkerFlagsPreview->setText("");
}

void LdLinkerOptionsWidget::setLinker(KDevLinker* kdl){
	m_pKDevLinker = kdl;
}

KDETabLd::KDETabLd(QWidget *parent, const char *name)
    : QWidget(parent, name), controller(new FlagCheckBoxController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagCheckBox(this, controller, "-lkdecore", i18n("KDE basics"));
    new FlagCheckBox(this, controller, "-lkdeui", i18n("KDE user interface"));
    new FlagCheckBox(this, controller, "-lkhtml", i18n("KDE HTML widget"));
    new FlagCheckBox(this, controller, "-lkfm", i18n("KDE kfm functionality"));
    new FlagCheckBox(this, controller, "-lkfile", i18n("KDE file handling"));
    new FlagCheckBox(this, controller, "-lkspell", i18n("KDE Spell checking"));
    new FlagCheckBox(this, controller, "-lkab", i18n("KDE addressbook"));
}


KDETabLd::~KDETabLd(){
    delete controller;
}


void KDETabLd::readFlags(QStringList *list){
    controller->readFlags(list);
}


void KDETabLd::writeFlags(QStringList *list){
    controller->writeFlags(list);
}

MiscTabLd::MiscTabLd(QWidget *parent, const char *name)
    : QWidget(parent, name), controller(new FlagCheckBoxController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagCheckBox(this, controller, "-lX11", i18n("X11 basics"));
    new FlagCheckBox(this, controller, "-lXext", i18n("X11 extensions"));
    new FlagCheckBox(this, controller, "-lqt", i18n("QT Library"));
    new FlagCheckBox(this, controller, "-lm", i18n("Math library"));
}


MiscTabLd::~MiscTabLd(){
    delete controller;
}


void MiscTabLd::readFlags(QStringList *list){
    controller->readFlags(list);
}


void MiscTabLd::writeFlags(QStringList *list){
    controller->writeFlags(list);
}



LdOptionsDlg::LdOptionsDlg(QWidget *parent, const char *name)
    : KDialogBase(Tabbed, i18n("GNU Linker Options"), Ok|Cancel, Ok,parent, name, true)
{
    QVBox *vbox;

    vbox = addVBoxPage(i18n("KDE"));
    kdetab = new KDETabLd(vbox, "KDE tab");

    vbox = addVBoxPage(i18n("Misc"));
    misctab = new MiscTabLd(vbox, "Misc tab");
}


LdOptionsDlg::~LdOptionsDlg()
{
}

QString LdOptionsDlg::flags() const
{
    QStringList flaglist;

    kdetab->writeFlags(&flaglist);
    misctab->writeFlags(&flaglist);

    QString flags;
    QStringList::ConstIterator li;
    for (li = flaglist.begin(); li != flaglist.end(); ++li) {
        flags += (*li);
        flags += " ";
    }

    flags.truncate(flags.length()-1);
    return flags;
}

QString LdOptionsDlg::setFlags(const QString &flags)
{
    QStringList flaglist = QStringList::split(" ", flags);

    kdetab->readFlags(&flaglist);
    misctab->readFlags(&flaglist);

    // lets see what we have over and return it
    QString flagsRest;
    QStringList::ConstIterator li;
    for (li = flaglist.begin(); li != flaglist.end(); ++li) {
        flagsRest += (*li);
        flagsRest += " ";
    }

    flagsRest.truncate(flagsRest.length()-1);
    return flagsRest;
}


#include "LdLinkerOptionsWidget.moc"
