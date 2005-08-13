/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <kdialog.h>
#include <klocale.h>

#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qapplication.h>
#include <qpushbutton.h>

#include "flagboxes.h"

#include "optiontabs.h"

LinkerTab::LinkerTab( QWidget * parent, const char * name )
    : QWidget(parent, name), radioController(new FlagRadioButtonController()),
    pathController(new FlagPathEditController()),
    editController(new FlagEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    QVButtonGroup *map_group = new QVButtonGroup(i18n("Map File"), this);
    QRadioButton *m_defaultMap = new QRadioButton(i18n("Off"), map_group);
    m_defaultMap->setChecked(true);
    new FlagRadioButton(map_group, radioController,
                     "-GS", i18n("Segments"));
    new FlagRadioButton(map_group, radioController,
                     "-GP", i18n("Publics"));
    new FlagRadioButton(map_group, radioController,
                     "-GD", i18n("Detailed"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagPathEdit(this, "", pathController,
                     "--dynamicloader=", i18n("Default dynamic loader:"), KFile::File);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    new FlagSpinEdit(this, 0, 2147483647, 1024, 1048576, editController,
                     "-$M", i18n("Reserved address space:"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    layout->addStretch();
}

LinkerTab::~LinkerTab( )
{
    delete radioController;
    delete pathController;
    delete editController;
}

void LinkerTab::readFlags( QStringList * str )
{
    pathController->readFlags(str);
    radioController->readFlags(str);
    editController->readFlags(str);
}

void LinkerTab::writeFlags( QStringList * str )
{
    pathController->writeFlags(str);
    radioController->writeFlags(str);
    editController->writeFlags(str);
}



LocationsTab::LocationsTab( QWidget * parent, const char * name )
    :QWidget(parent, name), pathController(new FlagPathEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagPathEdit(this, ":", pathController,
                     "-I", i18n("Include search path (delimited by \":\"):"));
    new FlagPathEdit(this, ":", pathController,
                     "-R", i18n("Resource search path (delimited by \":\"):"));
    new FlagPathEdit(this, ":", pathController,
                     "-U", i18n("Unit search path (delimited by \":\"):"));
    new FlagPathEdit(this, ":", pathController,
                     "-O", i18n("Object search path (delimited by \":\"):"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addStretch();
}

LocationsTab::~LocationsTab( )
{
    delete pathController;
}

void LocationsTab::readFlags( QStringList * str )
{
    pathController->readFlags(str);
}

void LocationsTab::writeFlags( QStringList * str )
{
    pathController->writeFlags(str);
}



Locations2Tab::Locations2Tab( QWidget * parent, const char * name )
    :QWidget(parent, name), pathController(new FlagPathEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagPathEdit(this, "", pathController,
                     "-E", i18n("Executable output directory:"));
    new FlagPathEdit(this, "", pathController,
                     "-N", i18n("Unit output directory:"));
    new FlagPathEdit(this, "", pathController,
                     "-LE", i18n("Package directory:"));
    new FlagPathEdit(this, "", pathController,
                     "-LN", i18n("Package source code directory:"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addStretch();
}

Locations2Tab::~Locations2Tab( )
{
    delete pathController;
}

void Locations2Tab::readFlags( QStringList * str )
{
    pathController->readFlags(str);
}

void Locations2Tab::writeFlags( QStringList * str )
{
    pathController->writeFlags(str);
}



GeneralTab::GeneralTab( QWidget * parent, const char * name )
    :QWidget(parent, name), controller(new FlagCheckBoxController()),
    editController(new FlagEditController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    QVButtonGroup *build_group = new QVButtonGroup(i18n("Build"), this);
    new FlagCheckBox(build_group, controller,
                     "-B", i18n("Build all units"));
    new FlagCheckBox(build_group, controller,
                     "-M", i18n("Make modified units"));
    new FlagCheckBox(build_group, controller,
                     "-Q", i18n("Quiet compile"));
    new FlagCheckBox(build_group, controller,
                     "-Z", i18n("Disable implicit package compilation"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addSpacing(10);

    new FlagListEdit(this, ":", editController, "-D", i18n("Conditional defines (delimited by \":\"):"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    new FlagListEdit(this, ":", editController, "-A", i18n("Unit aliases in form unit=alias (delimited by \":\"):"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addSpacing(10);

    QVButtonGroup *message_group = new QVButtonGroup(i18n("Messages"), this);
    new FlagCheckBox(message_group, controller,
                     "-H", i18n("Output hint messages"));
    new FlagCheckBox(message_group, controller,
                     "-W", i18n("Output warning messages"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addSpacing(10);
    QVButtonGroup *package_group = new QVButtonGroup(i18n("Packages"), this);
    new FlagListEdit(package_group, ":", editController, "-LU", i18n("Build with packages (delimited by \":\"):"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    layout->addStretch();
}

GeneralTab::~GeneralTab( )
{
    delete controller;
    delete editController;
}

void GeneralTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    editController->readFlags(str);
}

void GeneralTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    editController->writeFlags(str);
}



CodegenTab::CodegenTab( QWidget * parent, const char * name )
    :QWidget(parent, name), controller(new FlagCheckBoxController()),
    listController(new FlagEditController()),
    radioController(new FlagRadioButtonController())
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
//    layout->setAutoAdd(true);

    QGridLayout *layout2 = new QGridLayout(layout, 2, 2, KDialog::spacingHint());

    QVButtonGroup *align_group = new QVButtonGroup(i18n("Code Alignment && Stack Frames"), this);
    QRadioButton *align_def = new QRadioButton(i18n("Default (-$A8)"), align_group);
    align_def->setChecked(true);
    new FlagRadioButton(align_group, radioController,
                     "'-$A1'", i18n("Never align"));
    new FlagRadioButton(align_group, radioController,
                     "'-$A2'", i18n("On word boundaries"));
    new FlagRadioButton(align_group, radioController,
                     "'-$A4'", i18n("On double word boundaries"));
    new FlagRadioButton(align_group, radioController,
                     "'-$A8'", i18n("On quad word boundaries"));
    new FlagCheckBox(align_group, controller,
                     "'-$W+'", i18n("Generate stack frames"), "'-$W-'");
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout2->addWidget(align_group, 0, 0);

    QVButtonGroup *enum_group = new QVButtonGroup(i18n("Enumeration Size"), this);
    QRadioButton *enum_def = new QRadioButton(i18n("Default (-$Z1)"), enum_group);
    enum_def->setChecked(true);
    new FlagRadioButton(enum_group, radioController,
                     "'-$Z1'", i18n("Unsigned byte (256 values)"));
    new FlagRadioButton(enum_group, radioController,
                     "'-$Z2'", i18n("Unsigned word (64K values)"));
    new FlagRadioButton(enum_group, radioController,
                     "'-$Z4'", i18n("Unsigned double word (4096M values)"));
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout2->addWidget(enum_group, 0, 1);

    QVButtonGroup *compile_group = new QVButtonGroup(i18n("Compile Time Checks"), this);
    new FlagCheckBox(compile_group, controller,
                     "'-$C+'", i18n("Assertions"), "'-$C-'", "'-$C+'");
    new FlagCheckBox(compile_group, controller,
                     "'-$B+'", i18n("Complete boolean evaluation"), "'-$B-'");
    new FlagCheckBox(compile_group, controller,
                     "'-$X+'", i18n("Extended syntax"), "'-$X-'", "'-$X+'");
    new FlagCheckBox(compile_group, controller,
                     "'-$H+'", i18n("Long strings"), "'-$H-'", "'-$H+'");
    new FlagCheckBox(compile_group, controller,
                     "'-$P+'", i18n("Open string parameters"), "'-$P-'", "'-$P+'");
    new FlagCheckBox(compile_group, controller,
                     "'-$T+'", i18n("Type-checked pointers"), "'-$T-'");
    new FlagCheckBox(compile_group, controller,
                     "'-$V+'", i18n("Var-string checking"), "'-$V-'", "'-$V+'");
    new FlagCheckBox(compile_group, controller,
                     "'-$J+'", i18n("Writable typed constants"), "'-$J-'");
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout2->addWidget(compile_group, 1, 0);

    QVButtonGroup *run_group = new QVButtonGroup(i18n("Run Time Checks"), this);
    new FlagCheckBox(run_group, controller,
                     "'-$M+'", i18n("Runtime type information"), "'-$M-'");
    new FlagCheckBox(run_group, controller,
                     "'-$G+'", i18n("Imported data references"), "'-$G-'", "'-$G+'");
    new FlagCheckBox(run_group, controller,
                     "'-$I+'", i18n("Input/Output checking"), "'-$I-'", "'-$I+'");
    new FlagCheckBox(run_group, controller,
                     "'-$Q+'", i18n("Overflow checking"), "'-$Q-'");
    new FlagCheckBox(run_group, controller,
                     "'-$R+'", i18n("Range checking"), "'-$R-'");
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout2->addWidget(run_group, 1, 1);

    layout->addStretch();
}

CodegenTab::~CodegenTab( )
{
    delete controller;
    delete listController;
    delete radioController;
}

void CodegenTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    listController->readFlags(str);
    radioController->readFlags(str);
}

void CodegenTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    listController->writeFlags(str);
    radioController->writeFlags(str);
}



DebugOptimTab::DebugOptimTab( QWidget * parent, const char * name )
    : QWidget(parent, name), controller(new FlagCheckBoxController()),
    radioController(new FlagRadioButtonController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
//    layout->setAutoAdd(true);

    QVButtonGroup *optim_group = new QVButtonGroup(i18n("Optimization"), this);
    new FlagCheckBox(optim_group, controller,
                     "'-$O+'", i18n("Enable optimizations"), "'-$O-'", "'-$O+'");
    layout->addWidget(optim_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QBoxLayout *layout2 = new QHBoxLayout(layout, KDialog::spacingHint());

    QVButtonGroup *debug_group = new QVButtonGroup(i18n("Debugging"), this);
    new FlagCheckBox(debug_group, controller,
                     "'-$D+'", i18n("Debug information"), "'-$D-'", "'-$D+'");
    new FlagCheckBox(debug_group, controller,
                     "'-$L+'", i18n("Local symbol information"), "'-$L-'", "'-$L+'");
    gdb = new FlagCheckBox(debug_group, controller,
                     "-V", i18n("Debug information for GDB"));
    namespacedb = new FlagCheckBox(debug_group, controller,
                     "-VN", i18n("Namespace debug info"));
    symboldb = new FlagCheckBox(debug_group, controller,
                     "-VR", i18n("Write symbol info in an .rsm file"));
    layout2->addWidget(debug_group);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    QVButtonGroup *debug_add = new QVButtonGroup(i18n("Symbol Reference Information"), this);
    QRadioButton *m_default = new QRadioButton(i18n("Default (-$YD)"), debug_add);
    m_default->setChecked(true);
    new FlagRadioButton(debug_add, radioController,
                        "'-$Y-'", i18n("No information"));
    new FlagRadioButton(debug_add, radioController,
                        "'-$YD'", i18n("Definition information"));
    new FlagRadioButton(debug_add, radioController,
                        "'-$Y+'", i18n("Full reference information"));
    layout2->addWidget(debug_add);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QHBoxLayout *layout3 = new QHBoxLayout(layout, KDialog::spacingHint());
    QPushButton *release = new QPushButton(i18n("Release"), this);
    QPushButton *debug = new QPushButton(i18n("Debug"), this);
    layout3->addWidget(release);
    layout3->addWidget(debug);
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    connect(release, SIGNAL(clicked()), this, SLOT(setReleaseOptions()));
    connect(debug, SIGNAL(clicked()), this, SLOT(setDebugOptions()));

    layout->addStretch();
}

 DebugOptimTab::~ DebugOptimTab( )
{
    delete controller;
    delete radioController;
}

void DebugOptimTab::readFlags( QStringList * str )
{
    controller->readFlags(str);
    radioController->readFlags(str);
}

void DebugOptimTab::writeFlags( QStringList * str )
{
    controller->writeFlags(str);
    radioController->writeFlags(str);
}

void DebugOptimTab::setReleaseOptions()
{
    QStringList sl = QStringList::split(",", "'-$O+','-$Y-','-$D-','-$L-'");
    readFlags(&sl);
    gdb->setChecked(false);
    namespacedb->setChecked(false);
    symboldb->setChecked(false);
}

void DebugOptimTab::setDebugOptions()
{
    QStringList sl = QStringList::split(",", "'-$O-','-$Y+','-$D+','-$L+',-V,-VN");
    readFlags(&sl);
}

#include "optiontabs.moc"
