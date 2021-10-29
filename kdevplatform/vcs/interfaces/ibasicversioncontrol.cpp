/*
    SPDX-FileCopyrightText: 2011 Sergey Vidyuk <sir.vestnik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ibasicversioncontrol.h"
#include <KTextEdit>

/// Default empty implementation
void KDevelop::IBasicVersionControl::setupCommitMessageEditor(const QUrl&, KTextEdit* edit) const
{
    edit->setCheckSpellingEnabled(true);
}
