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

#include <kaction.h>

class QComboView;

class KListViewAction : public KWidgetAction
{
public:
    KListViewAction(QComboView *view, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name);

    ~KListViewAction();

    QComboView *view() const;

private:
    QComboView *m_view;
};

#endif
