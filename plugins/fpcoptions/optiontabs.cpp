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
#include <kdialog.h>
#include <klocale.h>

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qapplication.h>
#include <qframe.h>

#include "flagboxes.h"

#include "optiontabs.h"

FeedbackTab::FeedbackTab(QWidget *parent, const char *name)
    : QWidget(parent, name), controller(new FlagCheckBoxController(QStringList::split(",","-v")))
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    QVButtonGroup *output_group = new QVButtonGroup(i18n("Output"), this);
    new FlagCheckBox(output_group, controller,
                     "-vr", i18n("Format errors like GCC does."));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *verbose_group = new QVButtonGroup(i18n("Verbose"), this);
    new FlagCheckBox(verbose_group, controller,
                     "-va", i18n("Write all possible info."));
    new FlagCheckBox(verbose_group, controller,
                     "-v0", i18n("Write no messages."));
    new FlagCheckBox(verbose_group, controller,
                     "-ve", i18n("Show only errors"));
    new FlagCheckBox(verbose_group, controller,
                     "-vi", i18n("Show some general information."));
    new FlagCheckBox(verbose_group, controller,
                     "-vw", i18n("Issue warnings."));
    new FlagCheckBox(verbose_group, controller,
                     "-vn", i18n("Issue notes."));
    new FlagCheckBox(verbose_group, controller,
                     "-vh", i18n("Issue hints."));
    new FlagCheckBox(verbose_group, controller,
                     "-vd", i18n("Write other debugging info."));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *other_group = new QVButtonGroup(i18n("Other information"), this);
    new FlagCheckBox(other_group, controller,
                     "-vl", i18n("Show line numbers when processing files."));
    new FlagCheckBox(other_group, controller,
                     "-vu", i18n("Print information on loaded units."));
    new FlagCheckBox(other_group, controller,
                     "-vt", i18n("Print the names of loaded files."));
    new FlagCheckBox(other_group, controller,
                     "-vm", i18n("Write which macros are defined."));
    new FlagCheckBox(other_group, controller,
                     "-vc", i18n("Warn when processing a conditional."));
    new FlagCheckBox(other_group, controller,
                     "-vp", i18n("Print the names of procedures and functions."));
    new FlagCheckBox(other_group, controller,
                     "-vb", i18n("Show all procedure declarations if an overloaded function error occurs."));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
}

FeedbackTab::~FeedbackTab()
{
    delete controller;
}

void FeedbackTab::readFlags(QStringList *list)
{
    controller->readFlags(list);
}

void FeedbackTab::writeFlags(QStringList *list)
{
    controller->writeFlags(list);
}



FilesAndDirectoriesTab::FilesAndDirectoriesTab( QWidget * parent, const char * name )
    :QWidget(parent, name), controller(new FlagCheckBoxController(QStringList::split(",","-v"))),
    pathController(new FlagPathEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagCheckBox(this, controller,
                     "-P", i18n("Use pipes instead of files when assembling."));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagPathEdit(this, ":", pathController,
                     "-Fu", i18n("Unit search path:"));
    new FlagPathEdit(this, ":", pathController,
                     "-Fi", i18n("Include file search path:"));
    new FlagPathEdit(this, ":", pathController,
                     "-Fo", i18n("Object file search path:"));
    new FlagPathEdit(this, ":", pathController,
                     "-Fl", i18n("Library search path:"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagPathEdit(this, "", pathController,
                     "-e", i18n("Location of as and ld programs:"));
    new FlagPathEdit(this, "", pathController,
                     "-FL", i18n("Dynamic linker executable:"), KFile::File);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagPathEdit(this, "", pathController,
                     "-FU", i18n("Write units in:"));
    new FlagPathEdit(this, "", pathController,
                     "-Fe", i18n("Write compiler messages to file:"), KFile::File);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagPathEdit(this, "", pathController,
                     "-Fr", i18n("Compiler messages file:"), KFile::File);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
}

FilesAndDirectoriesTab::~FilesAndDirectoriesTab( )
{
    delete controller;
    delete pathController;
}

void FilesAndDirectoriesTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    pathController->readFlags(str);
}

void FilesAndDirectoriesTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    pathController->writeFlags(str);
}

LanguageTab::LanguageTab( QWidget * parent, const char * name )
    : QWidget(parent, name), controller(new FlagCheckBoxController(QStringList::split(",","-v")))
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    QVButtonGroup *compat_group = new QVButtonGroup(i18n("Pascal Compatibility"), this);
    new FlagCheckBox(compat_group, controller,
                     "-S2", i18n("Switch on Delphi 2 extensions."));
    new FlagCheckBox(compat_group, controller,
                     "-Sd", i18n("Strict Delphi compatibility mode."));
    new FlagCheckBox(compat_group, controller,
                     "-So", i18n("Borland TP 7.0 compatibility mode."));
    new FlagCheckBox(compat_group, controller,
                     "-Sp", i18n("GNU Pascal compatibility mode."));
    new FlagCheckBox(compat_group, controller,
                     "-vr", i18n("Format errors like GCC does."));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *ccompat_group = new QVButtonGroup(i18n("C/C++ Compatibility"), this);
    new FlagCheckBox(ccompat_group, controller,
                     "-Sc", i18n("Support C style operators *=, +=, /=, -=."));
    new FlagCheckBox(ccompat_group, controller,
                     "-Si", i18n("Support C++ style INLINE."));
    new FlagCheckBox(ccompat_group, controller,
                     "-Sm", i18n("Support C style macros."));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *lang_group = new QVButtonGroup(i18n("Language"), this);
    new FlagCheckBox(lang_group, controller,
                     "-Sg", i18n("Support the label and goto commands."));
    new FlagCheckBox(lang_group, controller,
                     "-Sh", i18n("Use ansistrings by default for strings."));
    new FlagCheckBox(lang_group, controller,
                     "-Ss", i18n("Require the name of constuctors to be init\n and the name of destructors to be done."));
    new FlagCheckBox(lang_group, controller,
                     "-St", i18n("Allow the static keyword in objects."));

    QVButtonGroup *code_group = new QVButtonGroup(i18n("Code Generation"), this);
    new FlagCheckBox(code_group, controller,
                     "-Sa", i18n("Include assert statements in compiled code."));
    new FlagCheckBox(code_group, controller,
                     "-Un", i18n("Do not check the unit name for being the same as the file name."));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
}

 LanguageTab::~ LanguageTab( )
{
    delete controller;
}

void LanguageTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
}

void LanguageTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
}








