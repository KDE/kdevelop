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
#include <qlistview.h>
#include <qradiobutton.h>
#include <kglobalsettings.h>
#include <klocale.h>


RegexpTestDialog::RegexpTestDialog(/*KDevPart *part*/)
    : RegexpTestDialogBase(0, "regexp test dialog", false)
{
    pattern_edit->setFocus();
    pattern_edit->setFont(KGlobalSettings::fixedFont());
    teststring_edit->setFont(KGlobalSettings::fixedFont());
    subgroups_listview->setSorting(0);
}


RegexpTestDialog::~RegexpTestDialog()
{}


void RegexpTestDialog::somethingChanged()
{
    success_label->clear();
    subgroups_listview->clear();
    
    regex_t compiledPattern;
    regmatch_t matches[20];

    int cflags = extendedposix_button->isChecked()? REG_EXTENDED : 0;
    QCString regexp = pattern_edit->text().latin1();
    int res = regcomp(&compiledPattern, regexp, cflags);
    if (res != 0) {
        QString regcompMessage;
        switch (res)
            {
            case REG_BADRPT:
                regcompMessage = i18n("Repitition operators must not appear as first character");
                break;
            case REG_BADBR:
                regcompMessage = i18n("Invalid use of back reference operator");
                break;
            case REG_EBRACE:
                regcompMessage = i18n("Unmatched brace inverval operators");
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
                regcompMessage = i18n("Invalid back reference to subexpresion");
                break;
            case REG_EESCAPE:
                regcompMessage = i18n("Trailing backslah");
                break;
            case REG_BADPAT:
                regcompMessage = i18n("Invalid use of pattern operators");
                break;
#ifdef REG_ESIZE
            case REG_ESIZE:
                regcompMessage = i18n("Huh? Regular expression too large");
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
    
    QCString testString = teststring_edit->text().latin1();
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
            QCString subGroup = testString.mid(matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so);
            new QListViewItem(subgroups_listview, QString::number(i), subGroup);
        }
    }
    regfree(&compiledPattern);
}

#include "regexptestdlg.moc"
