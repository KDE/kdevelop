/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qapplication.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qvaluelist.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qvbox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "flagboxes.h"
#include "gccoptionsplugin.h"

K_EXPORT_COMPONENT_FACTORY( libkdevgccoptions, KGenericFactory<GccOptionsPlugin>( "kdevgccoptions" ) )

class GeneralTab : public QWidget
{
public:
    GeneralTab( GccOptionsPlugin::Type type, QWidget *parent=0, const char *name=0 );
    ~GeneralTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
};


class OptimizationTab : public QWidget
{
public:
    OptimizationTab( GccOptionsPlugin::Type type, QWidget *parent=0, const char *name=0 );
    ~OptimizationTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    QRadioButton *Odefault, *O0, *O1, *O2;
    FlagListBox *optBox;
};


class G77Tab : public QWidget
{
public:
    G77Tab( QWidget *parent=0, const char *name=0 );
    ~G77Tab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
};


class Warnings1Tab : public QWidget
{
public:
    Warnings1Tab( GccOptionsPlugin::Type type, QWidget *parent=0, const char *name=0 );
    ~Warnings1Tab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagListBox *wallBox;
};


class Warnings2Tab : public QWidget
{
public:
    Warnings2Tab( GccOptionsPlugin::Type type, QWidget *parent=0, const char *name=0 );
    ~Warnings2Tab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagListBox *wrestBox;
};


GeneralTab::GeneralTab(GccOptionsPlugin::Type type, QWidget *parent, const char *name)
    : QWidget(parent, name), controller(new FlagCheckBoxController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);
    layout->addSpacing(10);

    QVButtonGroup *output_group = new QVButtonGroup(i18n("Output"), this);
    new FlagCheckBox(output_group, controller,
                     "-fsyntax-only", i18n("Only check the code for syntax errors, do not produce object code"));
    new FlagCheckBox(output_group, controller,
                     "-pg",           i18n("Generate extra code to write profile information for gprof"));
    new FlagCheckBox(output_group, controller,
                     "-save-temps",   i18n("Do not delete intermediate output like assembler files"));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *codegen_group = new QVButtonGroup(i18n("Code Generation"), this);
    if (type != GccOptionsPlugin::GPP) {
    new FlagCheckBox(codegen_group, controller,
                     "-fexceptions",        i18n("Enable exception handling"),
                     "-fno-exception");
    } else {
    new FlagCheckBox(codegen_group, controller,
                     "-fno-exceptions",     i18n("Disable exception handling"),
                     "-fexception");
    }
    // The following two are somehow mutually exclusive, but the default is
    // platform-dependent, so if we would leave out one of them, we wouldn't
    // know how to set the remaining one.
    new FlagCheckBox(codegen_group, controller,
                     "-fpcc-struct-return", i18n("Return certain struct and union values in memory rather than in registers"));
    new FlagCheckBox(codegen_group, controller,
                     "-freg-struct-return", i18n("Return certain struct and union values in registers when possible"));
    new FlagCheckBox(codegen_group, controller,
                     "-short-enums",        i18n("For an enum, choose the smallest possible integer type"));
    new FlagCheckBox(codegen_group, controller,
                     "-short-double",       i18n("Make 'double' the same as 'float'"));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addStretch();
}


GeneralTab::~GeneralTab()
{
    delete controller;
}


void GeneralTab::readFlags(QStringList *list)
{
    controller->readFlags(list);
}


void GeneralTab::writeFlags(QStringList *list)
{
    controller->writeFlags(list);
}


OptimizationTab::OptimizationTab(GccOptionsPlugin::Type type, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    QVButtonGroup *group = new QVButtonGroup(i18n("Optimization Level"), this);
    Odefault = new QRadioButton(i18n("Default"), group);
    Odefault->setChecked(true);
    O0 = new QRadioButton(i18n("No optimization"), group);
    O1 = new QRadioButton(i18n("Level 1"), group);
    O2 = new QRadioButton(i18n("Level 2"), group);

    optBox = new FlagListBox(this);

    new FlagListItem(optBox,
                     "-ffloat-store",       i18n("<qt>Do not store floating point variables in registers</qt>"),
                     "-fno-float-store");
    new FlagListItem(optBox,
                     "-fno-defer-pop",      i18n("<qt>Pop the arguments to each function call directly "
                                                 "after the function returns</qt>"),
                     "-fdefer-pop");
    new FlagListItem(optBox,
                     "-fforce-mem",         i18n("<qt>Force memory operands to be copied into registers before "
                                                 "doing arithmetic on them</qt>"),
                     "-fno-force-mem");
    new FlagListItem(optBox,
                     "-fforce-addr",        i18n("<qt>Force memory address constants to be copied into registers before "
                                                 "doing arithmetic on them</qt>"),
                     "-fno-force-addr");
    new FlagListItem(optBox,
                     "-omit-frame-pointer", i18n("<qt>Do not keep the frame pointer in a register for functions that "
                                                 "do not need one</qt>"),
                     "-fno-omit-frame-pointer");
    new FlagListItem(optBox,
                     "-no-inline",          i18n("<qt>Ignore the <i>inline</i> keyword</qt>"),
                     "-finline");

    if (type == GccOptionsPlugin::GPP) {
    new FlagListItem(optBox,
                     "-fno-default-inline", i18n("<qt>Do not make member functions inline merely because they "
                                                 "are defined inside the class scope</qt>"),
                     "-fdefault-inline");
    }

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addStretch();
}


OptimizationTab::~OptimizationTab()
{}


void OptimizationTab::readFlags(QStringList *list)
{
    optBox->readFlags(list);

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
    optBox->writeFlags(list);

    if (O0->isChecked())
        (*list) << "-O0";
    else if (O1->isChecked())
        (*list) << "-O1";
    else if (O2->isChecked())
        (*list) << "-O2";
}


G77Tab::G77Tab(QWidget *parent, const char *name)
    : QWidget(parent, name), controller(new FlagCheckBoxController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);
    layout->addSpacing(10);

    QVButtonGroup *dialect_group = new QVButtonGroup(i18n("Dialect"), this);
    new FlagCheckBox(dialect_group, controller,
                     "-ffree-form",       i18n("Interpret source code as Fortran 90 free form"),
                     "-fno-exception");
    new FlagCheckBox(dialect_group, controller,
                     "-ff90",             i18n("Allow certain Fortran 90 constructs"));
    new FlagCheckBox(dialect_group, controller,
                     "-fdollar-ok",       i18n("Allow '$' in symbol names"));
    new FlagCheckBox(dialect_group, controller,
                     "-fbackslash",       i18n("Allow '\' in character constants to escape special characters"),
                     "-fno-backslah");
    new FlagCheckBox(dialect_group, controller,
                     "-fonetrip",         i18n("DO loops are executed at least once"));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addSpacing(10);

    QVButtonGroup *codegen_group = new QVButtonGroup(i18n("Code Generation"), this);
    new FlagCheckBox(codegen_group, controller,
                     "-fno-automatic",    i18n("Treat local variables as if SAVE statement had been specified"));
    new FlagCheckBox(codegen_group, controller,
                     "-finit-local-zero", i18n("Init local variables to zero"));
    new FlagCheckBox(codegen_group, controller,
                     "-fbounds-check",    i18n("Generate run-time checks for array subscripts"));

    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addStretch();
}


G77Tab::~G77Tab()
{
    delete controller;
}


void G77Tab::readFlags(QStringList *list)
{
    controller->readFlags(list);
}


void G77Tab::writeFlags(QStringList *list)
{
    controller->writeFlags(list);
}


Warnings1Tab::Warnings1Tab(GccOptionsPlugin::Type type, QWidget *parent, const char *name)
    : QWidget(parent, name), controller(new FlagCheckBoxController)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    new FlagCheckBox(this, controller,
                     "-w",               i18n("Inhibit all warnings"));
    new FlagCheckBox(this, controller,
                     "-Wno-import",      i18n("Inhibit warnings about the use of #import"));
    new FlagCheckBox(this, controller,
                     "-Werror", 	 i18n("Make all warnings into errors"));
    new FlagCheckBox(this, controller,
                     "-pedantic",        i18n("Issue all warnings demanded by strict ANSI C or ISO C++"));
    new FlagCheckBox(this, controller,
                     "-pedantic-errors", i18n("Like -pedantic, but errors are produced instead of warnings"));
    new FlagCheckBox(this, controller,
                     "-Wall",            i18n("All warnings below, combined (-Wall):"));

    wallBox = new FlagListBox(this);

    new FlagListItem(wallBox,
                     "-Wchar-subscripts",    i18n("<qt>Warn if an array subscript has type <i>char</i></qt>"));
    new FlagListItem(wallBox,
                     "-Wcomment",            i18n("<qt>Warn when a comment-start sequence /* appears inside a comment</qt>"));
    new FlagListItem(wallBox,
                     "-Wformat",             i18n("<qt>Check calls to <i>printf()</i>, <i>scanf()</i> etc\n"
                                                  "to make sure that the arguments supplied have types appropriate\n"
                                                  "to the format string specified, and that the conversions specified\n"
                                                  "in the format string make sense</qt>"));
    new FlagListItem(wallBox,
                     "-Wimplicit-int",       i18n("<qt>Warn when a declaration does not specify a type</qt>"));
    new FlagListItem(wallBox,
                     "-Wimplicit-funtion-declaration",
                                             i18n("<qt>Issue a warning when a non-declared function is used</qt>"));
    new FlagListItem(wallBox,
                     "-Werror-implicit-function-declaration",
                                             i18n("<qt>Issue an error when a non-declared function is used</qt>"));
    new FlagListItem(wallBox,
                     "-Wmain",               i18n("<qt>Warn if the type of <i>main()</i> is suspicious</qt>"));
    new FlagListItem(wallBox,
                     "-Wmultichar",          i18n("<qt>Warn when multicharacter constants are encountered</qt>"));
    new FlagListItem(wallBox,
                     "-Wmissing-braces",     i18n("<qt>Warn if an aggregate or union initializer is not fully bracketed</qt>"));
    new FlagListItem(wallBox,
                     "-Wparentheses",        i18n("<qt>Warn when parentheses are omitted in certain contexts</qt>"));
    new FlagListItem(wallBox,
                     "-Wsequence-point",     i18n("<qt>Warn about code that may have undefined semantics because of\n"
                                                  "violations of sequence point rules in the C standard</qt>"));
    new FlagListItem(wallBox,
                     "-Wreturn-type",        i18n("<qt>Warn when a function without explicit return type is defined</qt>"));
    new FlagListItem(wallBox,
                     "-Wswitch",             i18n("<qt>Warn whenever a <i>switch</i> statement has an index of enumeral type\n"
                                                  "and lacks a <i>case</i> for one or more of the named codes of that enumeration</qt>"));
    new FlagListItem(wallBox,
                     "-Wtrigraphs",          i18n("<qt>Warn when trigraphs are encountered</qt>"));
    new FlagListItem(wallBox,
                     "-Wunused",             i18n("<qt>Warn when a variable is declared but not used</qt>"));
    new FlagListItem(wallBox,
                     "-Wuninitialized",      i18n("<qt>Warn when a variable is used without being initialized first</qt>"));
    new FlagListItem(wallBox,
                     "-Wunknown-pragmas",    i18n("<qt>Warn when an unknown #pragma statement is encountered</qt>"));
    if (type == GccOptionsPlugin::GPP) {
    new FlagListItem(wallBox,
                     "-Wreorder",            i18n("<qt>Warn when the order of member initializers is different from\n"
                                                  "the order in the class declaration</qt>"));
    }
}


Warnings1Tab::~Warnings1Tab()
{
    delete controller;
}


void Warnings1Tab::readFlags(QStringList *list)
{
    controller->readFlags(list);
    wallBox->readFlags(list);
}


void Warnings1Tab::writeFlags(QStringList *list)
{
    controller->writeFlags(list);
    wallBox->writeFlags(list);
}


Warnings2Tab::Warnings2Tab(GccOptionsPlugin::Type type, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->setAutoAdd(true);

    wrestBox = new FlagListBox(this);

    new FlagListItem(wrestBox,
                     "-W",                    i18n("<qt>Set options not included in -Wall which are very specific</qt>"));
    new FlagListItem(wrestBox,
                     "-Wfloat-equal",         i18n("<qt>Warn if floating point values are used in equality comparisons</qt>"));
    new FlagListItem(wrestBox,
                     "-Wundef",               i18n("<qt>Warn if an undefined identifier is evaluated in an <i>#if</i> directive</qt>"));
    new FlagListItem(wrestBox,
                     "-Wshadow",              i18n("<qt>Warn whenever a local variable shadows another local variable</qt>"));
    new FlagListItem(wrestBox,
                     "-Wpointer-arith",       i18n("<qt>Warn about anything that depends on the <i>sizeof</i> a\n"
                                                   "function type or of <i>void</i></qt>"));
    new FlagListItem(wrestBox,
                     "-Wcast-qual",           i18n("<qt>Warn whenever a pointer is cast so as to remove a type\n"
                                                   "qualifier from the target type</qt>"));
    new FlagListItem(wrestBox,
                     "-Wcast-align",          i18n("<qt>Warn whenever a pointer is cast such that the required\n"
                                                   "alignment of the target is increased</qt>"));
    new FlagListItem(wrestBox,
                     "-Wwrite-strings",       i18n("<qt>Warn when the address of a string constant is cast\n"
                                                   "into a non-const <i>char *</i> pointer</qt>"));
    new FlagListItem(wrestBox,
                     "-Wconversion",          i18n("<qt>Warn if a prototype causes a type conversion that is different\n"
                                                   "from what would happen to the same argument in the absence\n"
                                                   "of a prototype</qt>"));
    new FlagListItem(wrestBox,
                     "-Wsign-compare",        i18n("<qt>Warn when a comparison between signed and unsigned values\n"
                                                   "could produce an incorrect result when the signed value\n"
                                                   "is converted to unsigned</qt>"));
    new FlagListItem(wrestBox,
                     "-Wmissing-noreturn",    i18n("<qt>Warn about functions which might be candidates for attribute 'noreturn'</qt>"));
    new FlagListItem(wrestBox,
                     "-Waggregate-return",    i18n("<qt>Warn if any functions that return structures or unions are\n"
                                                   "defined or called</qt>"));
    new FlagListItem(wrestBox,
                     "-Wmissing-declarations",i18n("<qt>Warn if a global function is defined without a previous declaration</qt>"));
    new FlagListItem(wrestBox,
                     "-Wno-deprecated-declarations",
                                              i18n("<qt>Do not warn about uses of functions, variables, and types marked as\n"
                                                   "deprecated by using the 'deprecated' attribute</qt>"));
    new FlagListItem(wrestBox,
                     "-Wpacked",              i18n("<qt>Warn if a structure is given the packed attribute, but the packed\n"
                                                   "attribute has no effect on the layout or size of the structure</qt>"));
    new FlagListItem(wrestBox,
                     "-Wpadded",              i18n("<qt>Warn if padding is included in a structure, either to align an\n"
                                                   "element of the structure or to align the whole structure</qt>"));
    new FlagListItem(wrestBox,
                     "-Wredundant-decls",     i18n("<qt>Warn if anything is declared more than once in the same scope</qt>"));
    new FlagListItem(wrestBox,
                     "-Wunreachable-code",    i18n("<qt>Warn if the compiler detects that code will never be executed</qt>"));
    new FlagListItem(wrestBox,
                     "-Winline",              i18n("<qt>Warn if an <i>inline</i> function cannot be inlined</qt>"));
    new FlagListItem(wrestBox,
                     "-Wlong-long",           i18n("<qt>Warn if the <i>long long</i> type is used</qt>"));
    new FlagListItem(wrestBox,
                     "-Wdisabled-optimization",i18n("<qt>Warn if a requested optimization pass is disabled</qt>"));


    if (type == GccOptionsPlugin::GCC) {
    new FlagListItem(wrestBox,
                     "-Wtraditional",         i18n("<qt>Warn about certain constructs that behave differently\n"
                                                   "in traditional and ANSI C</qt>"));
    new FlagListItem(wrestBox,
                     "-Wbad-function-cast",   i18n("<qt>Warn whenever a function call is cast to a non-matching type</qt>"));
    new FlagListItem(wrestBox,
                     "-Wstrict-prototypes",   i18n("<qt>Warn if a function is declared or defined without specifying\n"
                                                   "the argument types</qt>"));
    new FlagListItem(wrestBox,
                     "-Wmissing-prototypes",  i18n("<qt>Warn if a global function is defined without a previous prototype declaration</qt>"));
    new FlagListItem(wrestBox,
                     "-Wnested-externs",      i18n("<qt>Warn if an <i>extern</i> declaration is encountered within a function</qt>"));
    }


    if (type == GccOptionsPlugin::GPP) {
    new FlagListItem(wrestBox,
                     "-Woverloaded-virtual",  i18n("<qt>Warn when a function declaration hides virtual\n"
                                                   "functions from a base class</qt>"));
    new FlagListItem(wrestBox,
                     "-Wsynth",               i18n("<qt>Warn when g++'s synthesis behavior does\n"
                                                   "not match that of cfront</qt>"));
    new FlagListItem(wrestBox,
                     "-Wctor-dtor-privacy",   i18n("<qt>Warn when a class seems unusable, because all the constructors or\n"
                                                   "destructors in a class are private and the class has no friends or\n"
                                                   "public static member functions</qt>"));
    new FlagListItem(wrestBox,
                     "-Wnon-virtual-dtor",    i18n("<qt>Warn when a class declares a non-virtual destructor that should\n"
                                                   "probably be virtual, because it looks like the class will be used\n"
                                                   "polymorphically</qt>"));
    new FlagListItem(wrestBox,
                     "-Wsign-promo",          i18n("<qt>Warn when overload resolution chooses a promotion from unsigned or\n"
                                                   "enumeral type to a signed type over a conversion to an unsigned\n"
                                                   "type of the same size. Previous versions of G++ would try to\n"
                                                   "preserve unsignedness, but the standard mandates the current behavior</qt>"));
    new FlagListItem(wrestBox,
                     "-Wabi",                 i18n("<qt>Warn when G++ generates code that is probably not compatible with\n"
                                                   "the vendor-neutral C++ ABI</qt>"));
/*    new FlagListItem(wrestBox,
                     "-Wreorder",             i18n("<qt>Warn when the order of member initializers given in the code does\n"
                                                   "not match the order in which they must be executed.</qt>"));*/
    new FlagListItem(wrestBox,
                     "-Weffc++",              i18n("<qt>Warn about violations of the following style guidelines from Scott\n"
                                                   "Meyers' 'Effective C++' book:\n"
                                                   "* Item 11:  Define a copy constructor and an assignment\n"
                                                   "  operator for classes with dynamically allocated memory;\n"
                                                   "* Item 12:  Prefer initialization to assignment in constructors;\n"
                                                   "* Item 14:  Make destructors virtual in base classes;\n"
                                                   "* Item 15:  Have `operator=' return a reference to `*this';\n"
                                                   "* Item 23:  Do not try to return a reference when you must\n"
                                                   "  return an object\n"
                                                   "\n"
                                                   "and about violations of the following style guidelines from Scott\n"
                                                   "Meyers' 'More Effective C++' book:\n"
                                                   "* Item 6:  Distinguish between prefix and postfix forms of\n"
                                                   "  increment and decrement operators;\n"
                                                   "* Item 7:  Never overload '&&', '||', or ','</qt>"));
    new FlagListItem(wrestBox,
                     "-Wno-deprecated",       i18n("<qt>Do not warn about usage of deprecated features</qt>"));
    new FlagListItem(wrestBox,
                     "-Wno-non-template-friend", i18n("<qt>Disable warnings when non-templatized friend functions are declared\n"
                                                   "within a template</qt>"));
    new FlagListItem(wrestBox,
                     "-Wold-style-cast",      i18n("<qt>Warn if an old-style (C-style) cast to a non-void type is used\n"
                                                   "within a C++ program</qt>"));
    new FlagListItem(wrestBox,
                     "-Wno-pmf-conversions",  i18n("<qt>Disable the diagnostic for converting a bound pointer to member\n"
                                                   "function to a plain pointer</qt>"));
    }
}


Warnings2Tab::~Warnings2Tab()
{}


void Warnings2Tab::readFlags(QStringList *list)
{
    wrestBox->readFlags(list);
}


void Warnings2Tab::writeFlags(QStringList *list)
{
    wrestBox->writeFlags(list);
}


// Last but not least... :-)
GccOptionsDialog::GccOptionsDialog(GccOptionsPlugin::Type type, QWidget *parent, const char *name)
    : KDialogBase(Tabbed, GccOptionsPlugin::captionForType(type), Ok|Cancel, Ok, parent, name, true)
{
    QVBox *vbox;

    vbox = addVBoxPage(i18n("General"));
    general = new GeneralTab(type, vbox, "general tab");

    vbox = addVBoxPage(i18n("Optimization"));
    optimization = new OptimizationTab(type, vbox, "optimization tab");

    if (type == GccOptionsPlugin::G77) {
        vbox = addVBoxPage(i18n("Fortran Specifics"));
        g77 = new G77Tab(vbox, "g77 tab");
    } else
        g77 = 0;

    vbox = addVBoxPage(i18n("Warnings (safe)"));
    warnings1 = new Warnings1Tab(type, vbox, "warnings1 tab");

    vbox = addVBoxPage(i18n("Warnings (unsafe)"));
    warnings2 = new Warnings2Tab(type, vbox, "warnings2 tab");
}


GccOptionsDialog::~GccOptionsDialog()
{
}


void GccOptionsDialog::setFlags(const QString &flags)
{
    QStringList flaglist = QStringList::split(" ", flags);

    // Hand them to 'general' at last, so it can make a line edit
    // with the unprocessed items
    if (g77)
        g77->readFlags(&flaglist);
    optimization->readFlags(&flaglist);
    warnings1->readFlags(&flaglist);
    warnings2->readFlags(&flaglist);
    general->readFlags(&flaglist);
    unrecognizedFlags = flaglist;
}


QString GccOptionsDialog::flags() const
{
    QStringList flaglist;

    if (g77)
        g77->writeFlags(&flaglist);
    optimization->writeFlags(&flaglist);
    warnings1->writeFlags(&flaglist);
    warnings2->writeFlags(&flaglist);
    general->writeFlags(&flaglist);

    QString flags;
    QStringList::ConstIterator li;
    for (li = flaglist.begin(); li != flaglist.end(); ++li) {
        flags += (*li);
        flags += " ";
    }

    for (li = unrecognizedFlags.begin(); li != unrecognizedFlags.end(); ++li) {
        flags += (*li);
        flags += " ";
    }

    flags.truncate(flags.length()-1);
    return flags;
}


GccOptionsPlugin::GccOptionsPlugin(QObject *parent, const char *name, const QStringList &args)
    : KDevCompilerOptions(parent, name)
{
    gcctype = Unknown;

    if ( args.count() == 0 )
	return;

    QString typeStr = args[ 0 ];

    if ( typeStr == "gcc" )
	gcctype = GccOptionsPlugin::GCC;
    else if ( typeStr == "g++" )
	gcctype = GccOptionsPlugin::GPP;
    else if ( typeStr == "g77" )
	gcctype = GccOptionsPlugin::G77;
}


GccOptionsPlugin::~GccOptionsPlugin()
{}


QString GccOptionsPlugin::captionForType(Type type)
{
    switch (type)
        {
        case GCC: return i18n("GNU C Compiler Options");
        case GPP: return i18n("GNU C++ Compiler Options");
        case G77: return i18n("GNU Fortran 77 Compiler Options");
        default: return QString::null;
        }
}


QString GccOptionsPlugin::exec(QWidget *parent, const QString &flags)
{
    if ( gcctype == Unknown ) return QString::null;
    GccOptionsDialog *dlg = new GccOptionsDialog(gcctype, parent, "gcc options dialog");
    QString newFlags = flags;
    dlg->setFlags(flags);
    if (dlg->exec() == QDialog::Accepted)
        newFlags = dlg->flags();
    delete dlg;
    return newFlags;
}

#include "gccoptionsplugin.moc"
