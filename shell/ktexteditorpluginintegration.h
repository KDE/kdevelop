/* This file is part of the KDE project

Copyright 2015 Milian Wolff <mail@milianw.de>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_KTEXTEDITOR_PLUGIN_INTEGRATION_H
#define KDEVPLATFORM_KTEXTEDITOR_PLUGIN_INTEGRATION_H

#include <QObject>
#include <QVector>

#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>

#include <interfaces/iplugin.h>

namespace KDevelop {
class ObjectListTracker;
class MainWindow;
}

namespace KTextEditorIntegration {

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    ~Application();

public slots:
    KTextEditor::MainWindow *activeMainWindow() const;
    QList<KTextEditor::MainWindow *> mainWindows() const;
    KTextEditor::Plugin *plugin(const QString &id) const;
};

class MainWindow : public QObject
{
    Q_OBJECT
public:
    explicit MainWindow(KDevelop::MainWindow *mainWindow);
    ~MainWindow();

public slots:
    QWidget *createToolView(KTextEditor::Plugin *plugin, const QString &identifier,
                            KTextEditor::MainWindow::ToolViewPosition pos,
                            const QIcon &icon, const QString &text);

    KXMLGUIFactory *guiFactory() const;

    QWidget *window() const;

    QList<KTextEditor::View *> views() const;

    KTextEditor::View *activeView() const;

    QObject *pluginView(const QString &id) const;

public:
    KTextEditor::MainWindow *interface() const;

    void addPluginView(const QString &id, QObject *pluginView);
    void removePluginView(const QString &id);

private:
    KDevelop::MainWindow *m_mainWindow;
    KTextEditor::MainWindow *m_interface;
    QHash<QString, QPointer<QObject>> m_pluginViews;
};

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit Plugin(KTextEditor::Plugin *plugin, QObject *parent = nullptr);
    ~Plugin();

    KXMLGUIClient* createGUIForMainWindow(Sublime::MainWindow *window) override;

    void unload() override;

    KTextEditor::Plugin *interface() const;

    QString pluginId() const;

private:
    QScopedPointer<KTextEditor::Plugin> m_plugin;
    // view objects and toolviews that should get deleted when the plugin gets unloaded
    KDevelop::ObjectListTracker *m_tracker;
};

void initialize();

}
#endif
