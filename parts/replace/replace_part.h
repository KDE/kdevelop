/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_REPLACE_H__
#define __KDEVPART_REPLACE_H__


#include <qguardedptr.h>
#include <kdevplugin.h>
#include <qpopupmenu.h>


class ReplaceWidget;
class Context;

class ReplacePart : public KDevPlugin
{
    Q_OBJECT

public:
    ReplacePart(QObject *parent, const char *name, const QStringList &);
    ~ReplacePart();

public slots:
    void slotReplace();
	void contextMenu(QPopupMenu *popup, const Context *context);

private:
    QGuardedPtr<ReplaceWidget> m_widget;
	QString m_popupstr;

};


#endif
