/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

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
#ifndef KLISTVIEWACTION_H
#define KLISTVIEWACTION_H

#include <kdeversion.h>
#include <kaction.h>

class KComboView;

/**
@file klistviewaction.h
Widget action with KComboView.
*/


/**
Widget action with KComboView.
Can be used on toolbars. It appears as @ref ResizableCombo.
*/
class KListViewAction: public KWidgetAction
{
public:
    KListViewAction(KComboView *view, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent);
    KListViewAction(KComboView *view, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, const bool);

    ~KListViewAction();

    KComboView *view() const;
    void setToolTip(const QString & str);
    void setWhatsThis(const QString &str);

private:
    void loadComboWidth();

    KComboView *m_view;
};

#endif
