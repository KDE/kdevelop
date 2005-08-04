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

#include "regexptestdlg.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3listview.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QShowEvent>
#include <Q3CString>
#include <kdeversion.h>
#include <kregexp.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kparts/part.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <kparts/componentfactory.h>
#include <kregexpeditorinterface.h>

#include "kdevplugin.h"
#include "kdevpartcontroller.h"


RegexpTestDialog::RegexpTestDialog(KDevPlugin *part)
    : RegexpTestDialogBase(0, "regexp test dialog", false), _regexp_dialog(0)
{
    pattern_edit->setFocus();
    pattern_edit->setFont(KGlobalSettings::fixedFont());
    teststring_edit->setFont(KGlobalSettings::fixedFont());
    subgroups_listview->setSorting(0);

    m_part = part;
}


RegexpTestDialog::~RegexpTestDialog()
{}


void RegexpTestDialog::showEvent(QShowEvent *)
{
    KParts::ReadWritePart *rwpart = dynamic_cast<KParts::ReadWritePart*>
        (m_part->partController()->activePart());
    insertbutton->setEnabled(rwpart);
}


void RegexpTestDialog::somethingChanged()
{
    success_label->clear();
    subgroups_listview->clear();

    if ( qregexp_button->isChecked() || qregexp_min_button->isChecked() )
        checkQRegExp();
    else if ( kregexp_button->isChecked() )
	checkKRegExp();
    else
        checkPOSIX();
}

void RegexpTestDialog::checkQRegExp()
{
    QRegExp rx( pattern_edit->text() );
    rx.setMinimal( qregexp_min_button->isChecked() );
    if ( !rx.isValid() ) {
#if QT_VERSION >= 0x030100
	success_label->setText( rx.errorString() );
#else
	success_label->setText( i18n("Error compiling the regular expression.") );
#endif
	return;
    }
    if ( rx.search( teststring_edit->text() ) < 0 ) {
	success_label->setText( i18n( "No match" ) );
	return;
    }
    success_label->setText( i18n("Successfully matched") );
#if QT_VERSION >= 0x030100
    int numCaptures = rx.numCaptures() + 1;
#else
    int numCaptures = 10;
#endif
    for ( int i = 0; i < numCaptures; ++i ) {
	new Q3ListViewItem( subgroups_listview, QString::number( i ), rx.cap( i ) );
    }
}

void RegexpTestDialog::checkKRegExp()
{
    KRegExp rx;
    if ( !rx.compile( pattern_edit->text().latin1() ) ) {
	success_label->setText( i18n( "Compile error, your regexp is invalid" ) );
	return;
    }
    if ( !rx.match( teststring_edit->text().latin1() ) ) {
        success_label->setText( i18n( "No match" ) );
        return;
    }
    success_label->setText( i18n("Successfully matched") );
    for ( int i = 0; i <= 9; ++i ) {
	const char* grp = rx.group( i );
	if ( grp )
	    new Q3ListViewItem( subgroups_listview, QString::number( i ), QString( grp ) );
    }
}

void RegexpTestDialog::checkPOSIX()
{
    regex_t compiledPattern;
    regmatch_t matches[20];
    int cflags = extendedposix_button->isChecked()? REG_EXTENDED : 0;
    Q3CString regexp = pattern_edit->text().local8Bit();
    int res = regcomp(&compiledPattern, regexp, cflags);
    if (res != 0) {
        QString regcompMessage;
        switch (res)
            {
            case REG_BADRPT:
                regcompMessage = i18n("Repetition operators must not appear as first character");
                break;
            case REG_BADBR:
                regcompMessage = i18n("Invalid use of back reference operator");
                break;
            case REG_EBRACE:
                regcompMessage = i18n("Unmatched brace interval operators");
                break;
            case REG_EBRACK:
                regcompMessage = i18n("Unmatched bracket list operators");
                break;
            case REG_ERANGE:
                regcompMessage = i18n("Invalid use of range operator");
                break;
            case REG_ECTYPE:
                regcompMessage = i18n("Unknown character class");
                break;
            case REG_ECOLLATE:
                regcompMessage = i18n("Invalid collating element");
                break;
            case REG_EPAREN:
                regcompMessage = i18n("Unmatched parenthesis group operators");
                break;
            case REG_ESUBREG:
                regcompMessage = i18n("Invalid back reference to subexpression");
                break;
            case REG_EESCAPE:
                regcompMessage = i18n("Trailing backslash");
                break;
            case REG_BADPAT:
                regcompMessage = i18n("Invalid use of pattern operators");
                break;
#ifdef REG_ESIZE
            case REG_ESIZE:
                regcompMessage = i18n("Regular expression too large");
                break;
#endif
            default:
                regcompMessage = i18n("Unknown error");
                break;
            }
        success_label->setText(regcompMessage);
        return;
    }

    for (int i = 0; i < 20; ++i) {
        matches[i].rm_so = -1;
        matches[i].rm_eo = -1;
    }

    Q3CString testString = teststring_edit->text().local8Bit();
    res = regexec(&compiledPattern, testString, 20, matches, 0);
    if (res != 0) {
        success_label->setText(i18n("No match"));
        return;
    }

    success_label->setText(i18n("Successfully matched"));
    int len = testString.length();
    for (int i = 0; i < 20; ++i) {
        if (matches[i].rm_so >= 0 && matches[i].rm_so <= len &&
            matches[i].rm_eo >= 0 && matches[i].rm_eo <= len &&
            matches[i].rm_so <= matches[i].rm_eo) {
            Q3CString subGroup = testString.mid(matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so);
            new Q3ListViewItem(subgroups_listview, QString::number(i), subGroup);
        }
    }
    regfree(&compiledPattern);
}


void RegexpTestDialog::insertQuoted()
{
    QString rawstr = pattern_edit->text();

    QString str;

    int len = rawstr.length();
    for (int i=0; i < len; ++i) {
        QChar ch = rawstr[i];
        if (ch == '"')
            str += "\\\"";
        else if (ch == '\\')
            str += "\\\\";
        else
            str += ch;
    }

    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(m_part->partController()->activePart());
    QWidget *view = m_part->partController()->activeWidget();

    KTextEditor::EditInterface *editiface
        = dynamic_cast<KTextEditor::EditInterface*>(rwpart);
    if (!editiface) {
        kdDebug() << "no edit" << endl;
        return;
    }
    KTextEditor::ViewCursorInterface *cursoriface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
    if (!cursoriface) {
        kdDebug() << "no viewcursor" << endl;
        return;
    }

    uint line, col;
    cursoriface->cursorPositionReal(&line, &col);
    editiface->insertText(line, col, str);
    reject();
}

void RegexpTestDialog::showRegExpEditor( )
{
    _regexp_dialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor" );

    if ( _regexp_dialog )
    {
        KRegExpEditorInterface *editor =
            static_cast<KRegExpEditorInterface *>( _regexp_dialog->qt_cast( "KRegExpEditorInterface" ) );

        editor->setRegExp( pattern_edit->text() );

        if ( _regexp_dialog->exec() == QDialog::Accepted )
        {
            pattern_edit->setText( editor->regExp() );
        }
    }
}

#include "regexptestdlg.moc"
