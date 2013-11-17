/* This file is part of KDevelop
 *  Copyright 2002 Harald Fernengel <harry@kdevelop.org>
 *  Copyright 2007 Hamish Rodda <rodda@kde.org>
 *  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef CPPCHECKPLUGIN_H
#define CPPCHECKPLUGIN_H

#include <QPointer>
#include <QVariant>

#include <interfaces/iplugin.h>
#include <interfaces/istatus.h>

#include "imodel.h"

class KJob;
class KUrl;
class QTreeView;

namespace cppcheck
{
class Control;
class Marks;
class WidgetFactory;

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    Plugin(QObject *parent, const QVariantList & = QVariantList());

    virtual ~Plugin();

    virtual void unload();

    void incomingModel(cppcheck::Model* model);

signals:
    void newModel(cppcheck::Model* model);

private slots:
    void loadOutput();
    void runCppcheck(bool allFiles);
    void runCppcheckFile();
    void runCppcheckAll();
    void result();

private:
    QString m_lastExec, m_lastParams, m_lastValExec, m_lastValParams,
            m_lastCtExec, m_lastCtParams, m_lastKcExec;
    cppcheck::WidgetFactory *m_factory;
    cppcheck::Marks         *m_marks;
};
}


#endif // CPPCHECKPLUGIN_H
