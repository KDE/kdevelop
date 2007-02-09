/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef IUICONTROLLER_H
#define IUICONTROLLER_H

#include "kdevexport.h"

#include <QStringList>
#include <kurl.h>

namespace KDevelop {

class IDocument;

class KDEVPLATFORM_EXPORT IToolViewFactory {
public:
    virtual ~IToolViewFactory() {}
    virtual QWidget* create(QWidget *parent = 0) = 0;

};

class KDEVPLATFORM_EXPORT IUiController {
public:
    virtual ~IUiController();

    enum SwitchMode { ThisWindow, NewWindow };
    enum ShowMode { ShowByDefault, HideByDefault };

    virtual void switchToArea(const QString &areaName, SwitchMode switchMode) = 0;

    virtual void addToolView(const QString &name, IToolViewFactory *factory) = 0;
    virtual void removeToolView(IToolViewFactory *factory) = 0;

    virtual void openUrl(const KUrl &url) = 0;

protected:
    IUiController();
};

}

#endif

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
