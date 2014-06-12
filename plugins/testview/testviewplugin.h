/*  This file is part of KDevelop
    Copyright 2012 Miha ?an?ula <miha@noughmad.eu>

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

#ifndef KDEVPLATFORM_PLUGIN_TESTVIEWPLUGIN_H
#define KDEVPLATFORM_PLUGIN_TESTVIEWPLUGIN_H

#include "interfaces/iplugin.h"
#include <QtCore/QVariantList>

class TestToolViewFactory;

class TestViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    TestViewPlugin(QObject* parent, const QVariantList& args = QVariantList());
    virtual ~TestViewPlugin();

    virtual void unload();

private:
    TestToolViewFactory* m_viewFactory;

private slots:
    void runAllTests();
    void stopRunningTests();
    void jobStateChanged();
};

#endif // KDEVPLATFORM_PLUGIN_TESTVIEWPLUGIN_H
