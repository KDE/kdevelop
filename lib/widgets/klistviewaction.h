/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef KLISTVIEWACTION_H
#define KLISTVIEWACTION_H

#include <kdeversion.h>
#if KDE_VERSION > 305
# include <kaction.h>
#else
# include <kdevwidgetaction.h>
#endif

class KComboView;

class KListViewAction
#if KDE_VERSION > 305
: public KWidgetAction
#else
: public KDevCompat::KWidgetAction
#endif
{
public:
    KListViewAction(KComboView *view, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name);

    ~KListViewAction();

    KComboView *view() const;
    void setToolTip(const QString & str);
    void setWhatsThis(const QString &str);

private:
    KComboView *m_view;
};

#endif
