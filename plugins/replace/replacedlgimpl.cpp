#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qstring.h>
#include <qregexp.h>
#include <qlabel.h>

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kregexpeditorinterface.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kdebug.h>

#include "replacedlgimpl.h"

namespace
{
/// @todo This is the same function as in ../grepview/grepviewwidget.cpp and
/// should probably be placed in a common place. For now it seemed like too
/// little code to bother with.
QString escape(const QString &str)
{
    QString escaped("[]{}()\\^$?.+-*");
    QString res;

    for (uint i=0; i < str.length(); ++i)
    {
        if (escaped.find(str[i]) != -1)
            res += "\\";
        res += str[i];
    }

    return res;
}
}


ReplaceDlgImpl::ReplaceDlgImpl(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
        : ReplaceDlg(parent,name, modal,fl), _regexp_dialog( 0 )

{
    connect( find_button, SIGNAL( clicked() ), SLOT( saveComboHistories() ) );
    connect( regexp_button, SIGNAL( clicked() ), SLOT( showRegExpEditor() ) );
    connect( find_combo, SIGNAL( textChanged( const QString & ) ),
        SLOT( validateFind( const QString & ) ) );
    connect( regexp_combo, SIGNAL( textChanged ( const QString & ) ),
             SLOT( validateExpression( const QString & ) ) );
    connect( strings_regexp_radio, SIGNAL( toggled( bool ) ), SLOT( toggleExpression( bool ) ) );

    // disable the editor button if the regexp editor isn't installed
    if ( KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty() )
    {
        strings_regexp_radio->disconnect( regexp_button );
    }

    path_urlreq->completionObject()->setMode(KUrlCompletion::DirCompletion);
    path_urlreq->setMode( KFile::Directory | KFile::LocalOnly );

    expression_varning_label->hide();
}

ReplaceDlgImpl::~ReplaceDlgImpl()
{}

void ReplaceDlgImpl::show( QString const & path )
{
    path_urlreq->lineEdit()->setText( path );

    find_combo->setCurrentText( "" );
    replacement_combo->setCurrentText( "" );
    regexp_combo->setCurrentText( "" );

    strings_all_radio->setChecked( true );
    find_combo->setFocus();
    
    find_button->setEnabled( false );

    QDialog::show();
}


void ReplaceDlgImpl::showRegExpEditor()
{
    _regexp_dialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor" );

    if ( _regexp_dialog )
    {
        KRegExpEditorInterface *editor =
            static_cast<KRegExpEditorInterface *>( _regexp_dialog->qt_cast( "KRegExpEditorInterface" ) );

        editor->setRegExp( regexp_combo->currentText() );

        if ( _regexp_dialog->exec() == QDialog::Accepted )
        {
            regexp_combo->setCurrentText( editor->regExp() );
        }
    }
}

void ReplaceDlgImpl::validateFind( const QString & )
{
    //kDebug(0) << "ReplaceWidget::validateFind()" << endl;

    bool x = find_combo->currentText().isEmpty() && ! strings_regexp_radio->isOn();
    find_button->setEnabled( !x );
}

void ReplaceDlgImpl::validateExpression( const QString & )
{
    //kDebug(0) << "ReplaceWidget::validateExpression()" << endl;

    QString pattern = regexp_combo->currentText();
    QRegExp re( pattern );

    if ( pattern.isEmpty() || !re.isValid() )
    {
        expression_varning_label->show();
        find_button->setEnabled( false );
    }
    else
    {
        expression_varning_label->hide();
        find_button->setEnabled( true );
    }
}

void ReplaceDlgImpl::toggleExpression( bool on )
{
    if ( on )
    {
        validateExpression( QString() );
    }
    else
    {
        expression_varning_label->hide();
        find_button->setEnabled( true );
    }
}

void ReplaceDlgImpl::saveComboHistories()
{
    if ( find_combo->isEnabled() && ! find_combo->currentText().isEmpty() )
    {
        find_combo->addToHistory( find_combo->currentText() );
    }

    if ( ! replacement_combo->currentText().isEmpty() )
    {
        replacement_combo->addToHistory( replacement_combo->currentText() );
    }

    if ( regexp_combo->isEnabled() && ! regexp_combo->currentText().isEmpty() )
    {
        regexp_combo->addToHistory( regexp_combo->currentText() );
    }
}

QRegExp ReplaceDlgImpl::expressionPattern()
{
    QString pattern = escape( find_combo->currentText() );

    QRegExp re;
    re.setCaseSensitive( case_box->isChecked() );
    re.setMinimal( true );

    if ( strings_wholewords_radio->isChecked() )
    {
        pattern = "\\b" + pattern + "\\b";
    }
    else if ( strings_regexp_radio->isChecked() )
    {
        pattern = regexp_combo->currentText();
    }

    re.setPattern( pattern );

    return re;
}

QString ReplaceDlgImpl::replacementString()
{
    return replacement_combo->currentText();
}

#include "replacedlgimpl.moc"

