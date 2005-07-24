/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcheckbox.h>
#include <qlabel.h>
#include <qregexp.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <klocale.h>
#include "koReplace.h"
#include <kmessagebox.h>

KoReplaceDialog::KoReplaceDialog(QWidget *parent, const char *name, long options, const QStringList &findStrings, const QStringList &replaceStrings, bool hasSelection) :
    KoFindDialog(parent, name, true)
{
    init(true, findStrings, hasSelection);
    setOptions(options);
    setReplacementHistory(replaceStrings);
}

KoReplaceDialog::~KoReplaceDialog()
{
}

bool KoReplace::validateMatch( const QString & /*text*/, int /*index*/, int /*matchedlength*/ )
{
    return true;
}

long KoReplaceDialog::options() const
{
    long options = 0;

    options = KoFindDialog::options();
    if (m_promptOnReplace->isChecked())
        options |= PromptOnReplace;
    if (m_backRef->isChecked())
        options |= BackReference;
    return options;
}

QWidget *KoReplaceDialog::replaceExtension()
{
    return m_replaceExtension;
}

QString KoReplaceDialog::replacement() const
{
    return m_replace->currentText();
}

QStringList KoReplaceDialog::replacementHistory() const
{
    return m_find->historyItems();
}

void KoReplaceDialog::setOptions(long options)
{
    KoFindDialog::setOptions(options);
    m_promptOnReplace->setChecked(options & PromptOnReplace);
    m_backRef->setChecked(options & BackReference);
}

void KoReplaceDialog::setReplacementHistory(const QStringList &strings)
{
    if (strings.count() > 0)
        m_replace->setHistoryItems(strings, true);
    else
        m_replace->clearHistory();
}

void KoReplaceDialog::slotOk()
{
    KoFindDialog::slotOk();
    m_replace->addToHistory(replacement());
}

// Create the dialog.
KoReplace::KoReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent) :
    KDialogBase(parent, __FILE__, false,  // non-modal!
        i18n("Replace"),
        User3 | User2 | User1 | Close,
        User3,
        false,
        i18n("&All"), i18n("&Skip"), i18n("Replace"))
{
    setMainWidget( new QLabel( i18n("Replace '%1' with '%2'?").arg(pattern).arg(replacement), this ) );
    m_cancelled = false;
    m_options = options;
    m_parent = parent;
    m_replacements = 0;
    if (m_options & KoReplaceDialog::RegularExpression)
        m_regExp = new QRegExp(pattern, m_options & KoReplaceDialog::CaseSensitive);
    else
        m_pattern = pattern;
    m_replacement = replacement;
    resize(minimumSize());
}

KoReplace::~KoReplace()
{
    if (!m_replacements && !m_cancelled)
        KMessageBox::information(m_parent, i18n("No text was replaced."));
}

void KoReplace::slotClose()
{
    m_cancelled = true;
    kapp->exit_loop();
}

void KoReplace::abort()
{
    slotClose();
}

bool KoReplace::replace(QString &text, const QRect &expose)
{
    if (m_options & KoFindDialog::FindBackwards)
    {
        m_index = text.length();
    }
    else
    {
        m_index = 0;
    }
    m_text = text;
    m_expose = expose;
    do
    {
        // Find the next match.
        if (m_options & KoReplaceDialog::RegularExpression)
            m_index = KoFind::find(m_text, *m_regExp, m_index, m_options, &m_matchedLength);
        else
            m_index = KoFind::find(m_text, m_pattern, m_index, m_options, &m_matchedLength);
        if (m_index != -1)
        {
            if (m_options & KoReplaceDialog::PromptOnReplace)
            {
                // Tell the world about the match we found, in case someone wants to
                // highlight it.
                if ( validateMatch( m_text, m_index, m_matchedLength ))
                {
                    emit highlight(m_text, m_index, m_matchedLength, m_expose);
                    show();
                    kapp->enter_loop();
                }
                else
                    m_index = m_index+m_matchedLength;
            }
            else
            {
                doReplace();
            }
        }
    }
    while ((m_index != -1) && !m_cancelled);
    text = m_text;

    // Should the user continue?
    return !m_cancelled;
}

int KoReplace::replace(QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KoFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replace(text, replacement, index, matchedLength);
        if (options & KoReplaceDialog::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

int KoReplace::replace(QString &text, const QRegExp &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KoFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replace(text, replacement, index, matchedLength);
        if (options & KoReplaceDialog::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

int KoReplace::replace(QString &text, const QString &replacement, int index, int length)
{
    // TBD: implement backreferences.
    text.replace(index, length, replacement);
    return replacement.length();
}

// All.
void KoReplace::slotUser1()
{
    int replacedLength;

    replacedLength = KoReplace::replace(m_text, m_replacement, m_index, m_matchedLength);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    emit replace(m_text, m_index, replacedLength,m_matchedLength , m_expose);
    m_replacements++;
    if (m_options & KoReplaceDialog::FindBackwards)
        m_index--;
    else
        m_index += replacedLength;
    m_options &= ~KoReplaceDialog::PromptOnReplace;
    kapp->exit_loop();
}

// Skip.
void KoReplace::slotUser2()
{
    if (m_options & KoReplaceDialog::FindBackwards) m_index--;
       else m_index++;
    kapp->exit_loop();
}

// Yes.
void KoReplace::slotUser3()
{
    doReplace();
    kapp->exit_loop();
}

void KoReplace::doReplace()
{
    int replacedLength;

    replacedLength = KoReplace::replace(m_text, m_replacement, m_index, m_matchedLength);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    emit replace(m_text, m_index, replacedLength,m_matchedLength, m_expose);
    m_replacements++;
    if (m_options & KoReplaceDialog::FindBackwards)
        m_index--;
    else
        m_index += replacedLength;
}

#include "koReplace.moc"
