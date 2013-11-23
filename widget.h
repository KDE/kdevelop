/* This file is part of KDevelop
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

#ifndef CPPCHECKWIDGET_H
#define CPPCHECKWIDGET_H

#include <QTabWidget>
#include <QMap>

class QResizeEvent;

namespace cppcheck
{
class Model;
class Plugin;
class Job;

class Widget : public QTabWidget
{
    Q_OBJECT

public:
    Widget(cppcheck::Plugin* plugin, QWidget* parent);
    cppcheck::Plugin* plugin() const;

protected:
    void resizeEvent ( QResizeEvent * event );
public slots:
    void updateTabText(cppcheck::Model * model, const QString & text);

private Q_SLOTS:
    void newModel(cppcheck::Model* model);
    void destroyRequestedTab(int tab);

private:
    cppcheck::Plugin* m_plugin;
};
}

#endif // CPPCHECKWIDGET_H
