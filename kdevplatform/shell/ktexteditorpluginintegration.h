/*
    Copyright 2015 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDEVPLATFORM_KTEXTEDITOR_PLUGIN_INTEGRATION_H
#define KDEVPLATFORM_KTEXTEDITOR_PLUGIN_INTEGRATION_H

#include <QObject>
#include <QPointer>

#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>

#include <interfaces/iplugin.h>

namespace KDevelop {
class ObjectListTracker;
class MainWindow;
}

namespace KTextEditorIntegration {

/**
 * Class mimicking the KTextEditor::Application interface
 */
class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

public Q_SLOTS:
    KTextEditor::MainWindow *activeMainWindow() const;
    QList<KTextEditor::MainWindow *> mainWindows() const;

    KTextEditor::Plugin *plugin(const QString &id) const;

    QList<KTextEditor::Document *> documents();

    KTextEditor::Document *openUrl(const QUrl &url, const QString &encoding = QString());

    bool closeDocument(KTextEditor::Document *document) const;
};

class MainWindow : public QObject
{
    Q_OBJECT
public:
    explicit MainWindow(KDevelop::MainWindow *mainWindow);
    ~MainWindow() override;

public Q_SLOTS:
    QWidget *createToolView(KTextEditor::Plugin *plugin, const QString &identifier,
                            KTextEditor::MainWindow::ToolViewPosition pos,
                            const QIcon &icon, const QString &text);

    KXMLGUIFactory *guiFactory() const;

    QWidget *window() const;

    QList<KTextEditor::View *> views() const;

    KTextEditor::View *activeView() const;
    KTextEditor::View *activateView(KTextEditor::Document *doc);

    QObject *pluginView(const QString &id) const;
    void splitView(Qt::Orientation orientation);

    QWidget *createViewBar(KTextEditor::View *view);
    void deleteViewBar(KTextEditor::View *view);
    void showViewBar(KTextEditor::View *view);
    void hideViewBar(KTextEditor::View *view);
    void addWidgetToViewBar(KTextEditor::View *view, QWidget *widget);

    KTextEditor::View *openUrl(const QUrl &url, const QString &encoding = QString());
    bool showToolView(QWidget *widget);

public:
    KTextEditor::MainWindow *interface() const;

    void addPluginView(const QString &id, QObject *pluginView);
    void removePluginView(const QString &id);

private:
    KDevelop::MainWindow* const m_mainWindow;
    KTextEditor::MainWindow *m_interface;
    QHash<QString, QPointer<QObject>> m_pluginViews;
    QHash<KTextEditor::View*, QWidget*> m_viewBars;
};

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit Plugin(KTextEditor::Plugin *plugin, QObject *parent = nullptr);
    ~Plugin() override;

    KXMLGUIClient* createGUIForMainWindow(Sublime::MainWindow *window) override;

    void unload() override;

    KTextEditor::Plugin *interface() const;

    QString pluginId() const;

private:
    QPointer<KTextEditor::Plugin> m_plugin;
    // view objects and tool views that should get deleted when the plugin gets unloaded
    KDevelop::ObjectListTracker *m_tracker;
};

void initialize();

}
#endif
