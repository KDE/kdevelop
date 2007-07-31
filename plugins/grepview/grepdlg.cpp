/***************************************************************************
*   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepdlg.h"
#include "grepviewpart.h"
#include <icore.h>
#include <kparts/partmanager.h>
#include <kglobal.h>

#include <QDir>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <QBoxLayout>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstringlist.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qlabel.h>
#include <kcombobox.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kstdguiitem.h>
#include <kparts/part.h>
#include <klineedit.h>


const char *__template_desc[] = {
    "verbatim",
    "assignment",
    "->MEMBER(",
    "class::MEMBER(",
    "OBJECT->member(",
    0
};

const char *__template_str[] = {
    "%s",
    "\\<%s\\>[\\t ]*=[^=]",
    "\\->[\\t ]*\\<%s\\>[\\t ]*\\(",
    "[a-z0-9_$]+[\\t ]*::[\\t ]*\\<%s\\>[\\t ]*\\(",
    "\\<%s\\>[\\t ]*\\->[\\t ]*[a-z0-9_$]+[\\t ]*\\(",
    0
};

const char *__filepatterns[] = {
    "*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.idl,*.c,*.m,*.mm,*.M",
    "*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.c,*.m,*.mm,*.M",
    "*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.idl",
    "*.adb",
    "*.cs",
    "*.f",
    "*.html,*.htm",
    "*.hs",
    "*.java",
    "*.js",
    "*.php,*.php3,*.php4",
    "*.pl",
    "*.pp,*.pas",
    "*.py",
    "*.js,*.css,*.yml,*.rb,*.rhtml,*.rjs,*.rxml",
    "CMakeLists.txt,*.cmake",
    "*",
    0
};

QStringList fromChar( const char *strs[] )
{
    QStringList ret;
    const char *oneline = strs[0];
    int i=0;

    while( oneline != 0 )
    {
        ret << QString( oneline );
        i++;
        oneline = strs[i];
    }
    return ret;
}

GrepDialog::GrepDialog( GrepViewPart * part, QWidget *parent )
    : QDialog(parent ), m_part( part )
{
    QStringList template_desc = fromChar( __template_desc );
    QStringList template_str = fromChar( __template_str );
    QStringList filepatterns = fromChar( __filepatterns );
    setWindowTitle(i18n("Find in Files"));

    KConfigGroup cg = KGlobal::config()->group( "GrepDialog" );

//     QGridLayout *layout = new QGridLayout(this, 9, 2, 10, 4);
    QGridLayout *layout = new QGridLayout(this);

    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 20);

    QLabel *pattern_label = new QLabel(i18n("&Pattern:"), this);
    layout->addWidget(pattern_label, 0, 0, Qt::AlignRight | Qt::AlignVCenter);

    pattern_combo = new KComboBox(true, this);
    pattern_label->setBuddy(pattern_combo);
    pattern_combo->setFocus();
    pattern_combo->addItems( cg.readEntry("LastSearchItems", QStringList()) );
    pattern_combo->setInsertPolicy(QComboBox::NoInsert);
    layout->addWidget(pattern_combo, 0, 1);

    QLabel *template_label = new QLabel(i18n("&Template:"), this);
    layout->addWidget(template_label, 1, 0, Qt::AlignRight | Qt::AlignVCenter);

//     QBoxLayout *template_layout = new QHBoxLayout(4);
    QBoxLayout *template_layout = new QHBoxLayout();
    template_layout->setSpacing(4);
    layout->addLayout(template_layout, 1, 1);

    template_edit = new KLineEdit(this);
    template_label->setBuddy(template_edit);
    template_edit->setText(template_str[0]);
    template_layout->addWidget(template_edit, 1);

    KComboBox *template_combo = new KComboBox(false, this);
    template_combo->addItems(template_desc);
    template_layout->addWidget(template_combo, 0);

    QBoxLayout *search_opts_layout = new QHBoxLayout();
    search_opts_layout->setSpacing(15);
    layout->addLayout(search_opts_layout, 2, 1);

    regexp_box = new QCheckBox(i18n("&Regular Expression"), this);
    regexp_box->setChecked(cg.readEntry("regexp", false ));
    search_opts_layout->addWidget(regexp_box);

    case_sens_box = new QCheckBox(i18n("C&ase sensitive"), this);
    case_sens_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    case_sens_box->setChecked(cg.readEntry("case_sens", true));
    search_opts_layout->addWidget(case_sens_box);

    QLabel *dir_label = new QLabel(i18n("&Directory:"), this);
    layout->addWidget(dir_label, 3, 0, Qt::AlignRight | Qt::AlignVCenter);

    QBoxLayout *dir_layout = new QHBoxLayout();
    dir_layout->setSpacing(4);
    layout->addLayout(dir_layout, 3, 1);

    dir_combo = new KComboBox( true, this );
    dir_combo->addItems( cg.readPathListEntry("LastSearchPaths"));
    dir_combo->setInsertPolicy(QComboBox::NoInsert);
    dir_combo->setEditText(QDir::homePath());

    url_requester = new KUrlRequester( dir_combo, this );
    url_requester->completionObject()->setMode(KUrlCompletion::DirCompletion);
    url_requester->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

    dir_label->setBuddy( url_requester );
    dir_combo->setMinimumWidth(dir_combo->fontMetrics().maxWidth()*25);
    dir_layout->addWidget( url_requester, 10 );

    synch_button = new KPushButton( this );
    KIcon syncIcon("dirsynch");
    synch_button->setIcon(syncIcon);
    synch_button->setShortcut( QKeySequence( "Alt+y") );
    synch_button->setToolTip( i18n("Set directory to that of the current file (Alt+Y)") );
    dir_layout->addWidget( synch_button );

    QBoxLayout *dir_opts_layout = new QHBoxLayout();
    dir_opts_layout->setSpacing(15);
    layout->addLayout(dir_opts_layout, 4, 1);

    recursive_box = new QCheckBox(i18n("Rec&ursive"), this);
    recursive_box->setChecked(cg.readEntry("recursive", true));
    dir_opts_layout->addWidget(recursive_box);

    use_project_box = new QCheckBox(i18n("Limit search to &project files"), this);
    use_project_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    use_project_box->setChecked(cg.readEntry("search_project_files", true));
    dir_opts_layout->addWidget(use_project_box);
    use_project_box->setEnabled(false);

    QLabel *files_label = new QLabel(i18n("&Files:"), this);
    layout->addWidget(files_label, 5, 0, Qt::AlignRight | Qt::AlignVCenter);

    files_combo = new KComboBox(true, this);
    files_label->setBuddy(files_combo->focusProxy());
    files_combo->addItems(filepatterns);
    layout->addWidget(files_combo, 5, 1);

    QLabel *exclude_label = new QLabel(i18n("&Exclude:"), this);
    layout->addWidget(exclude_label, 6, 0, Qt::AlignRight | Qt::AlignVCenter);

    QStringList exclude_list = cg.readEntry("exclude_patterns", QStringList());
    exclude_combo = new KComboBox(true, this);
    exclude_label->setBuddy(files_combo->focusProxy());
    if (exclude_list.count()) {
        exclude_combo->addItems(exclude_list);
    }
    else
    {
        exclude_combo->addItem("/CVS/,/SCCS/,/\\.svn/,/_darcs/");
        exclude_combo->addItem("");
    }
    layout->addWidget(exclude_combo, 6, 1);

    QBoxLayout *other_opts_layout = new QHBoxLayout();
    other_opts_layout->setSpacing(15);
    layout->addLayout(other_opts_layout, 7, 1);

    // no separate outputview for grepview as of kdev4
//     keep_output_box = new QCheckBox(i18n("New view"), this);
// //     keep_output_box->setChecked(config->readBoolEntry("new_view", true));
//     other_opts_layout->addWidget(keep_output_box);

    no_find_err_box = new QCheckBox(i18n("&Suppress find errors"), this);
    no_find_err_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    no_find_err_box->setChecked(cg.readEntry("no_find_errs", true));
    other_opts_layout->addWidget(no_find_err_box);

    QBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setSpacing(4);
    layout->addLayout(button_layout, 8, 1);
    search_button = new KPushButton(KGuiItem(i18n("Sea&rch"),"grep"), this);
    search_button->setDefault(true);
//     KPushButton *done_button = new KPushButton(KGuiItem::cancel(), this);
    KPushButton *done_button = new KPushButton("Cancel", this);
    button_layout->addStretch();
    button_layout->addWidget(search_button);
    button_layout->addWidget(done_button);

    resize(sizeHint());

    pattern_combo->setWhatsThis(
            i18n("Enter the regular expression you want to search for here.<p>"
            "Possible meta characters are:"
            "<ul>"
                "<li><b>.</b> - Matches any character</li>"
                "<li><b>^</b> - Matches the beginning of a line</li>"
                "<li><b>$</b> - Matches the end of a line</li>"
                "<li><b>\\&lt;</b> - Matches the beginning of a word</li>"
                "<li><b>\\&gt;</b> - Matches the end of a word</li>"
            "</ul>"
            "The following repetition operators exist:"
            "<ul>"
                "<li><b>?</b> - The preceding item is matched at most once</li>"
                "<li><b>*</b> - The preceding item is matched zero or more times</li>"
                "<li><b>+</b> - The preceding item is matched one or more times</li>"
                "<li><b>{<i>n</i>}</b> - The preceding item is matched exactly <i>n</i> times</li>"
                "<li><b>{<i>n</i>,}</b> - The preceding item is matched <i>n</i> or more times</li>"
                "<li><b>{,<i>n</i>}</b> - The preceding item is matched at most <i>n</i> times</li>"
                "<li><b>{<i>n</i>,<i>m</i>}</b> - The preceding item is matched at least <i>n</i>, "
                        "but at most <i>m</i> times.</li>"
            "</ul>"
            "Furthermore, backreferences to bracketed subexpressions are "
            "available via the notation \\<i>n</i>.</p>"
            ));
    files_combo->setWhatsThis(
            i18n("Enter the file name pattern of the files to search here. "
            "You may give several patterns separated by commas"));
    template_edit->setWhatsThis(
            i18n("You can choose a template for the pattern from the combo box "
            "and edit it here. The string %s in the template is replaced "
            "by the pattern input field, resulting in the regular expression "
            "to search for."));

    connect( template_combo, SIGNAL(activated(int)), SLOT(templateActivated(int)) );
    connect( search_button, SIGNAL(clicked()), SLOT(slotSearchClicked()) );
    connect( done_button, SIGNAL(clicked()), SLOT(hide()) );
    connect( pattern_combo->lineEdit(), SIGNAL( textChanged ( const QString & ) ),
            SLOT( slotPatternChanged( const QString & ) ) );
    connect( synch_button, SIGNAL(clicked()), this, SLOT(slotSynchDirectory()) );
    slotPatternChanged( pattern_combo->currentText() );
}

// Returns the contents of a QComboBox as a QStringList
static QStringList qCombo2StringList( QComboBox* combo )
{
    QStringList list;
    if (!combo)
    return list;
    for (int i = 0; i < combo->count(); ++i ) {
        list << combo->itemText(i);
    }
    return list;
}

GrepDialog::~GrepDialog()
{
    KConfigGroup cg = KGlobal::config()->group( "GrepDialog" );
    // memorize the last patterns and paths
    cg.writeEntry("LastSearchItems", qCombo2StringList(pattern_combo));
    cg.writePathEntry("LastSearchPaths", qCombo2StringList(dir_combo));
    cg.writeEntry("regexp", regexp_box->isChecked());
    cg.writeEntry("recursive", recursive_box->isChecked());
    cg.writeEntry("search_project_files", use_project_box->isChecked());
    cg.writeEntry("case_sens", case_sens_box->isChecked());
// 	cg.writeEntry("new_view", keep_output_box->isChecked());
    cg.writeEntry("no_find_errs", no_find_err_box->isChecked());
    cg.writeEntry("exclude_patterns", qCombo2StringList(exclude_combo));
    cg.sync();
}

void GrepDialog::slotPatternChanged( const QString & _text )
{
    search_button->setEnabled( !_text.isEmpty() );
}

void GrepDialog::templateActivated(int index)
{
    template_edit->setText(__template_str[index]);
}

// Find out whether the string s is already contained in combo
static bool qComboContains( const QString& s, QComboBox* combo )
{
    if (!combo)
    return false;
    for (int i = 0; i < combo->count(); ++i ) {
        if (combo->itemText(i) == s) {
            return true;
        }
    }
    return false;
}

void GrepDialog::slotSearchClicked()
{
    if (pattern_combo->currentText().isEmpty()) {
        KMessageBox::sorry(this, i18n("Please enter a search pattern"));
        pattern_combo->setFocus();
        return;
    }
    // add the last patterns and paths to the combo
    if (!qComboContains(pattern_combo->currentText(), pattern_combo)) {
        pattern_combo->insertItem(0, pattern_combo->currentText());
    }
    if (pattern_combo->count() > 15) {
        pattern_combo->removeItem(15);
    }
    if (!qComboContains(exclude_combo->currentText(), exclude_combo)) {
        exclude_combo->insertItem(0, exclude_combo->currentText());
    }
    if (exclude_combo->count() > 15) {
        exclude_combo->removeItem(15);
    }
    if (!qComboContains(dir_combo->currentText(), dir_combo)) {
        dir_combo->insertItem(0, dir_combo->currentText() );
    }
    if (dir_combo->count() > 15) {
        dir_combo->removeItem(15);
    }

    emit searchClicked();
    hide();
}

void GrepDialog::show()
{
    // not beautiful, but works with all window
    // managers and Qt versions
    if (isVisible())
        hide();
//         QDialog::hide();
    QDialog::show();
    pattern_combo->setFocus();
}

void GrepDialog::hide()
{
    pattern_combo->setFocus();
    QDialog::hide();
}

void GrepDialog::slotSynchDirectory( )
{
    KParts::ReadOnlyPart * part = dynamic_cast<KParts::ReadOnlyPart*>( m_part->core()->partManager()->activePart() );
    if ( part )
    {
        KUrl url = part->url();
        if ( url.isLocalFile() )
        {
            dir_combo->setEditText( url.upUrl().path( KUrl::LeaveTrailingSlash ) );
        }
    }
}

void GrepDialog::setEnableProjectBox(bool enable)
{
    use_project_box->setEnabled(enable);
    if (!enable) use_project_box->setChecked(false);
}

#include "grepdlg.moc"
