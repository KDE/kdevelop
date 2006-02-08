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

#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qapplication.h>
#include <qframe.h>
#include <qpushbutton.h>

#include "flagboxes.h"

#include "optiontabs.h"

FeedbackTab::FeedbackTab(QWidget *parent, const char *name)
    : QWidget(parent, name), controller(new FlagCheckBoxController(QStringList::split(",","-v")))
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    QVButtonGroup *output_group = new QVButtonGroup(i18n("Output"), this);
    new FlagCheckBox(output_group, controller,
                     "-vr", i18n("Format errors like GCC does"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *verbose_group = new QVButtonGroup(i18n("Verbose"), this);
    new FlagCheckBox(verbose_group, controller,
                     "-va", i18n("Write all possible info"));
    new FlagCheckBox(verbose_group, controller,
                     "-v0", i18n("Write no messages"));
    new FlagCheckBox(verbose_group, controller,
                     "-ve", i18n("Show only errors"));
    new FlagCheckBox(verbose_group, controller,
                     "-vi", i18n("Show some general information"));
    new FlagCheckBox(verbose_group, controller,
                     "-vw", i18n("Issue warnings"));
    new FlagCheckBox(verbose_group, controller,
                     "-vn", i18n("Issue notes"));
    new FlagCheckBox(verbose_group, controller,
                     "-vh", i18n("Issue hints"));
    new FlagCheckBox(verbose_group, controller,
                     "-vd", i18n("Write other debugging info"));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *other_group = new QVButtonGroup(i18n("Other Information"), this);
    new FlagCheckBox(other_group, controller,
                     "-vl", i18n("Show line numbers when processing files"));
    new FlagCheckBox(other_group, controller,
                     "-vu", i18n("Print information on loaded units"));
    new FlagCheckBox(other_group, controller,
                     "-vt", i18n("Print the names of loaded files"));
    new FlagCheckBox(other_group, controller,
                     "-vm", i18n("Write which macros are defined"));
    new FlagCheckBox(other_group, controller,
                     "-vc", i18n("Warn when processing a conditional"));
    new FlagCheckBox(other_group, controller,
                     "-vp", i18n("Print the names of procedures and functions"));
    new FlagCheckBox(other_group, controller,
                     "-vb", i18n("Show all procedure declarations if an overloaded function error occurs"));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
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
    :QWidget(parent, name), controller(new FlagCheckBoxController()),
    pathController(new FlagPathEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagPathEdit(this, ":", pathController,
                     "-Fu", i18n("Unit search path (delimited by \":\"):"));
    new FlagPathEdit(this, ":", pathController,
                     "-Fi", i18n("Include file search path (delimited by \":\"):"));
    new FlagPathEdit(this, ":", pathController,
                     "-Fo", i18n("Object file search path (delimited by \":\"):"));
    new FlagPathEdit(this, ":", pathController,
                     "-Fl", i18n("Library search path (delimited by \":\"):"));
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

FilesAndDirectoriesTab2::FilesAndDirectoriesTab2( QWidget * parent, const char * name )
    :QWidget(parent, name), controller(new FlagCheckBoxController()),
    pathController(new FlagPathEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagPathEdit(this, "", pathController,
                     "-FE", i18n("Write executables and units in:"));
    new FlagPathEdit(this, "", pathController,
                     "-FU", i18n("Write units in:"));
    new FlagPathEdit(this, "", pathController,
                     "-o", i18n("Executable name:"), KFile::File);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(20);

    new FlagPathEdit(this, "", pathController,
                     "-e", i18n("Location of as and ld programs:"));
    new FlagPathEdit(this, "", pathController,
                     "-FL", i18n("Dynamic linker executable:"), KFile::File);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(20);

    new FlagPathEdit(this, "", pathController,
                     "-Fr", i18n("Compiler messages file:"), KFile::File);
    new FlagPathEdit(this, "", pathController,
                     "-Fe", i18n("Write compiler messages to file:"), KFile::File);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
}

FilesAndDirectoriesTab2::~FilesAndDirectoriesTab2( )
{
    delete controller;
    delete pathController;
}

void FilesAndDirectoriesTab2::readFlags( QStringList * str )
{
    controller->readFlags(str);
    pathController->readFlags(str);
}

void FilesAndDirectoriesTab2::writeFlags( QStringList * str )
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
                     "-S2", i18n("Switch on Delphi 2 extensions"));
    new FlagCheckBox(compat_group, controller,
                     "-Sd", i18n("Strict Delphi compatibility mode"));
    new FlagCheckBox(compat_group, controller,
                     "-So", i18n("Borland TP 7.0 compatibility mode"));
    new FlagCheckBox(compat_group, controller,
                     "-Sp", i18n("GNU Pascal compatibility mode"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *ccompat_group = new QVButtonGroup(i18n("C/C++ Compatibility"), this);
    new FlagCheckBox(ccompat_group, controller,
                     "-Sc", i18n("Support C style operators *=, +=, /=, -="));
    new FlagCheckBox(ccompat_group, controller,
                     "-Si", i18n("Support C++ style INLINE"));
    new FlagCheckBox(ccompat_group, controller,
                     "-Sm", i18n("Support C style macros"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *lang_group = new QVButtonGroup(i18n("Language"), this);
    new FlagCheckBox(lang_group, controller,
                     "-Sg", i18n("Support the label and goto commands"));
    new FlagCheckBox(lang_group, controller,
                     "-Sh", i18n("Use ansistrings by default for strings"));
    new FlagCheckBox(lang_group, controller,
                     "-Ss", i18n("Require the name of constructors to be init\n and the name of destructors to be done"));
    new FlagCheckBox(lang_group, controller,
                     "-St", i18n("Allow the static keyword in objects"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
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

AssemblerTab::AssemblerTab( QWidget * parent, const char * name )
    : QWidget(parent, name), controller(new FlagCheckBoxController()),
    asmController(new FlagRadioButtonController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
//    layout->setAutoAdd(true);

    QBoxLayout *layout2 = new QHBoxLayout(layout, KDialog::spacingHint());

    QVButtonGroup *info_group = new QVButtonGroup(i18n("Assembler Info"), this);
    new FlagCheckBox(info_group, controller,
                     "-a", i18n("Do not delete assembler files"));
    new FlagCheckBox(info_group, controller,
                     "-al", i18n("List source"));
    new FlagCheckBox(info_group, controller,
                     "-ar", i18n("List register allocation and release info"));
    new FlagCheckBox(info_group, controller,
                     "-at", i18n("List temporary allocations and deallocations"));
    layout2->addWidget(info_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    //layout->addSpacing(10);

    QVButtonGroup *asmkind_group = new QVButtonGroup(i18n("Assembler Reader"), this);
    QRadioButton *m_defaultkind = new QRadioButton(i18n("Use default reader"), asmkind_group);
    m_defaultkind->setChecked(true);
    new FlagRadioButton(asmkind_group, asmController,
                        "-Ratt", i18n("AT&T style assembler"));
    new FlagRadioButton(asmkind_group, asmController,
                        "-Rintel", i18n("Intel style assembler"));
    new FlagRadioButton(asmkind_group, asmController,
                        "-Rdirect", i18n("Direct assembler"));
    layout2->addWidget(asmkind_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);


    QVButtonGroup *asm_group = new QVButtonGroup(i18n("Assembler Output"), this);
    new FlagCheckBox(asm_group, controller,
                     "-P", i18n("Use pipes instead of files when assembling"));
    QRadioButton *m_default = new QRadioButton(i18n("Use default output"), asm_group);
    m_default->setChecked(true);
    new FlagRadioButton(asm_group, asmController,
                        "-Aas", i18n("Use GNU as"));
    new FlagRadioButton(asm_group, asmController,
                        "-Aasout", i18n("Use GNU asaout"));
    new FlagRadioButton(asm_group, asmController,
                        "-Anasmcoff", i18n("Use NASM coff"));
    new FlagRadioButton(asm_group, asmController,
                        "-Anasmelf", i18n("Use NASM elf"));
    new FlagRadioButton(asm_group, asmController,
                        "-Anasmobj", i18n("Use NASM obj"));
    new FlagRadioButton(asm_group, asmController,
                        "-Amasm", i18n("Use MASM"));
    new FlagRadioButton(asm_group, asmController,
                        "-Atasm", i18n("Use TASM"));
    new FlagRadioButton(asm_group, asmController,
                        "-Acoff", i18n("Use coff"));
    new FlagRadioButton(asm_group, asmController,
                        "-Apecoff", i18n("Use pecoff"));
    layout->addWidget(asm_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
}

 AssemblerTab::~ AssemblerTab( )
{
    delete controller;
    delete asmController;
}

void AssemblerTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    asmController->readFlags(str);
}

void AssemblerTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    asmController->writeFlags(str);
}



DebugOptimTab::DebugOptimTab( QWidget * parent, const char * name )
    : QWidget(parent, name), controller(new FlagCheckBoxController()),
    optimController(new FlagRadioButtonController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
//    layout->setAutoAdd(true);

    QBoxLayout *layout2 = new QHBoxLayout(layout, KDialog::spacingHint());

    QBoxLayout *layout3 = new QVBoxLayout(layout2, KDialog::spacingHint());

    QVButtonGroup *debug_group = new QVButtonGroup(i18n("Debugging"), this);
    new FlagCheckBox(debug_group, controller,
                     "-g", i18n("Generate information for GDB"), "-!g");
    new FlagCheckBox(debug_group, controller,
                     "-gd", i18n("Generate information for DBX"), "-!gd");
    new FlagCheckBox(debug_group, controller,
                     "-gl", i18n("Use lineinfo unit"), "-!gl");
    new FlagCheckBox(debug_group, controller,
                     "-gh", i18n("Use heaptrc unit"), "-!gh");
    new FlagCheckBox(debug_group, controller,
                     "-gc", i18n("Generate checks for pointers"), "-!gc");
    layout3->addWidget(debug_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout3->addSpacing(10);

    QVButtonGroup *profile_group = new QVButtonGroup(i18n("Profiling"), this);
    new FlagCheckBox(profile_group, controller,
                     "-pg", i18n("Generate profiler code for gprof"), "-!pg");
    layout3->addWidget(profile_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout3->addSpacing(10);

    QBoxLayout *layout4 = new QVBoxLayout(layout2, KDialog::spacingHint());

    QVButtonGroup *optim_group1 = new QVButtonGroup(i18n("General Optimization"), this);
    m_default = new QRadioButton(i18n("Default"), optim_group1);
    m_default->setChecked(true);
    new FlagRadioButton(optim_group1, optimController,
                        "-Og", i18n("Generate smaller code"));
    optim1 = new FlagRadioButton(optim_group1, optimController,
                        "-OG", i18n("Generate faster code"));
    layout4->addWidget(optim_group1);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout4->addSpacing(10);

    QVButtonGroup *optim_group2 = new QVButtonGroup(i18n("Optimization Levels"), this);
    m_default2 = new QRadioButton(i18n("Default"), optim_group2);
    m_default2->setChecked(true);
    new FlagRadioButton(optim_group2, optimController,
                        "-O1", i18n("Level 1"));
    new FlagRadioButton(optim_group2, optimController,
                        "-O2", i18n("Level 2"));
    optim2 = new FlagRadioButton(optim_group2, optimController,
                        "-O3", i18n("Level 3"));
    layout4->addWidget(optim_group2);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout4->addSpacing(10);

    QHBoxLayout *layout5 = new QHBoxLayout(layout, KDialog::spacingHint());

    QVButtonGroup *optim_group3 = new QVButtonGroup(i18n("Architecture"), this);
    m_default3 = new QRadioButton(i18n("Default"), optim_group3);
    m_default3->setChecked(true);
    new FlagRadioButton(optim_group3, optimController,
                     "-Op1", i18n("386/486"));
    new FlagRadioButton(optim_group3, optimController,
                     "-Op2", i18n("Pentium/PentiumMMX"));
    new FlagRadioButton(optim_group3, optimController,
                     "-Op2", i18n("PentiumPro/PII/Cyrix 6x86/K6"));
    layout5->addWidget(optim_group3);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QVButtonGroup *optim_group4 = new QVButtonGroup(i18n("Another Optimization"), this);
    new FlagCheckBox(optim_group4, controller,
                     "-Or", i18n("Use register variables"), "-!Or");
    new FlagCheckBox(optim_group4, controller,
                     "-Ou", i18n("Uncertain optimizations"), "-!Ou");
    layout5->addWidget(optim_group4);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QHBoxLayout *layout6 = new QHBoxLayout(layout, KDialog::spacingHint());
    QPushButton *release = new QPushButton(i18n("Release"), this);
    QPushButton *debug = new QPushButton(i18n("Debug"), this);
    layout6->addWidget(release);
    layout6->addWidget(debug);
    connect(release, SIGNAL(clicked()), this, SLOT(setReleaseOptions()));
    connect(debug, SIGNAL(clicked()), this, SLOT(setDebugOptions()));

    layout->addStretch();
}

 DebugOptimTab::~ DebugOptimTab( )
{
    delete controller;
    delete optimController;
}

void DebugOptimTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    optimController->readFlags(str);
}

void DebugOptimTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    optimController->writeFlags(str);
}

void DebugOptimTab::setReleaseOptions()
{
    m_default->setChecked(true);
    m_default2->setChecked(true);
//    m_default3->setChecked(true);
    QStringList sl = QStringList::split(",", "-!g,-!gd,-!gl,-!gh,-!gc,-!pg,-!Ou,-!Or");
    readFlags(&sl);
    optim1->setChecked(true);
    optim2->setChecked(true);
}

void DebugOptimTab::setDebugOptions()
{
    QStringList sl = QStringList::split(",", "-g,-gl,-gh,-gc");
    readFlags(&sl);
    m_default->setChecked(true);
    m_default2->setChecked(true);
//    m_default3->setChecked(true);
}

CodegenTab::CodegenTab( QWidget * parent, const char * name )
    : QWidget(parent, name), controller(new FlagCheckBoxController()),
    listController(new FlagEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    QVButtonGroup *compile_group = new QVButtonGroup(i18n("Compile Time Checks"), this);
    new FlagCheckBox(compile_group, controller,
                     "-Sa", i18n("Include assert statements in compiled code"));
    new FlagCheckBox(compile_group, controller,
                     "-Un", i18n("Do not check the unit name for being the same as the file name"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *run_group = new QVButtonGroup(i18n("Run Time Checks"), this);
    new FlagCheckBox(run_group, controller,
                     "-Cr", i18n("Range checking"));
    new FlagCheckBox(run_group, controller,
                     "-Ct", i18n("Stack checking"));
    new FlagCheckBox(run_group, controller,
                     "-Ci", i18n("Input/Output checking"));
    new FlagCheckBox(run_group, controller,
                     "-Co", i18n("Integer overflow checking"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagListEdit(this, ":", listController, "-d", i18n("Conditional defines (delimited by \":\"):"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    new FlagListEdit(this, ":", listController, "-u", i18n("Undefine conditional defines (delimited by \":\"):"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagSpinEdit(this, 1024, 67107840, 1, 131072, listController,
                    "-Cs", i18n("Stack size:"));
    new FlagSpinEdit(this, 1024, 67107840, 1, 2097152, listController,
                    "-Ch", i18n("Heap size:"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
}

CodegenTab::~ CodegenTab( )
{
    delete controller;
    delete listController;
}

void CodegenTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    listController->readFlags(str);
}

void CodegenTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    listController->writeFlags(str);
}

LinkerTab::LinkerTab( QWidget * parent, const char * name )
    : QWidget(parent, name), controller(new FlagCheckBoxController()),
    listController(new FlagEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

    QBoxLayout *layout2 = new QHBoxLayout(layout, KDialog::spacingHint());

    QVButtonGroup *link_group = new QVButtonGroup(i18n("Linking Stage"), this);
    new FlagCheckBox(link_group, controller,
                     "-CD", i18n("Create dynamic library"));
    new FlagCheckBox(link_group, controller,
                     "-CX", i18n("Create smartlinked units"));
    new FlagCheckBox(link_group, controller,
                     "-Ur", i18n("Generate release units"));
    new FlagCheckBox(link_group, controller,
                     "-Cn", i18n("Omit the linking stage"));
    new FlagCheckBox(link_group, controller,
                     "-s",  i18n("Create assembling and linking script"));
    layout2->addWidget(link_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QVButtonGroup *exec_group = new QVButtonGroup(i18n("Executable Generation"), this);
    new FlagCheckBox(exec_group, controller,
                     "-Xs",  i18n("Strip the symbols from the executable"));
    new FlagCheckBox(exec_group, controller,
                     "-XS",  i18n("Link with static units"));
    new FlagCheckBox(exec_group, controller,
                     "-XX",  i18n("Link with smartlinked units"));
    new FlagCheckBox(exec_group, controller,
                     "-XD",  i18n("Link with dynamic libraries"));
    new FlagCheckBox(exec_group, controller,
                     "-Xc",  i18n("Link with the C library"));
    layout2->addWidget(exec_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    FlagListEdit *led = new FlagListEdit(this, ":", listController, "-k", i18n("Options passed to the linker (delimited by \":\"):"));
    layout->addWidget(led);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
}

LinkerTab::~LinkerTab( )
{
    delete controller;
    delete listController;
}

void LinkerTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    listController->readFlags(str);
}

void LinkerTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    listController->writeFlags(str);
}

MiscTab::MiscTab( QWidget * parent, const char * name )
    : QWidget(parent, name), controller(new FlagCheckBoxController()),
    radioController(new FlagRadioButtonController()),
    pathController(new FlagPathEditController()),
    editController(new FlagEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagCheckBox(this, controller,
                     "-B", i18n("Recompile all used units"));
    new FlagCheckBox(this, controller,
                     "-n", i18n("Do not read default configuration file"));
    new FlagPathEdit(this, "", pathController,
                     "@", i18n("Compiler configuration file:"), KFile::File);
    new FlagSpinEdit(this, 1, 1000, 1, 50, editController,
                    "-Se", i18n("Stop after the error:"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *browser_group = new QVButtonGroup(i18n("Browser Info"), this);
    QRadioButton *m_defaultBrowser = new QRadioButton(i18n("No browser info"), browser_group);
    m_defaultBrowser->setChecked(true);
    new FlagRadioButton(browser_group, radioController,
                     "-b", i18n("Global browser info"));
    new FlagRadioButton(browser_group, radioController,
                     "-bl", i18n("Global and local browser info"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *target_group = new QVButtonGroup(i18n("Target OS"), this);
    QRadioButton *m_defaultTarget = new QRadioButton(i18n("Default"), target_group);
    m_defaultTarget->setChecked(true);
    new FlagRadioButton(target_group, radioController,
                     "-TGO32V1", i18n("DOS and version 1 of the DJ DELORIE extender"));
    new FlagRadioButton(target_group, radioController,
                     "-TGO32V2", i18n("DOS and version 2 of the DJ DELORIE extender"));
    new FlagRadioButton(target_group, radioController,
                     "-TLINUX",  i18n("Linux"));
    new FlagRadioButton(target_group, radioController,
                     "-TOS2", i18n("OS/2 (2.x) using the EMX extender"));
    new FlagRadioButton(target_group, radioController,
                     "-TWIN32", i18n("WINDOWS 32 bit"));
    new FlagRadioButton(target_group, radioController,
                     "-TSUNOS", i18n("SunOS/Solaris"));
    new FlagRadioButton(target_group, radioController,
                     "-TBEOS", i18n("BeOS"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    layout->addStretch();
}

MiscTab::~ MiscTab( )
{
    delete controller;
    delete pathController;
    delete radioController;
    delete editController;
}

void MiscTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    radioController->readFlags(str);
    pathController->readFlags(str);
    editController->readFlags(str);
}

void MiscTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    radioController->writeFlags(str);
    pathController->writeFlags(str);
    editController->writeFlags(str);
}

#include "optiontabs.moc"
