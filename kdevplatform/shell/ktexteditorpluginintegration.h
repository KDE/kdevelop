/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEVPLATFORM_KTEXTEDITOR_PLUGIN_INTEGRATION_H
#define KDEVPLATFORM_KTEXTEDITOR_PLUGIN_INTEGRATION_H

#include <QObject>
#include <QPointer>
#include <QHash>

#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>

#include <interfaces/iplugin.h>

namespace KDevelop {
class ObjectListTracker;
class MainWindow;
}

namespace KTextEditorIntegration {

class ShowMessagesJob;

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
    KTextEditor::Document *findUrl(const QUrl &url) const;

    bool closeDocument(KTextEditor::Document *document) const;

    bool quit() const;
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
    bool closeView(KTextEditor::View *kteView);
    bool closeSplitView(KTextEditor::View *kteView);
    bool viewsInSameSplitView(KTextEditor::View* kteView1, KTextEditor::View* kteView2) const;

    QWidget *createViewBar(KTextEditor::View *view);
    void deleteViewBar(KTextEditor::View *view);
    void showViewBar(KTextEditor::View *view);
    void hideViewBar(KTextEditor::View *view);
    void addWidgetToViewBar(KTextEditor::View *view, QWidget *widget);

    KTextEditor::View *openUrl(const QUrl &url, const QString &encoding = QString());
    bool showToolView(QWidget *widget);

    void showMessage(QVariantMap);

public:
    KTextEditor::MainWindow *interface() const;

    void addPluginView(const QString &id, QObject *pluginView);
    void removePluginView(const QString &id);

private:
    KDevelop::MainWindow* const m_mainWindow;
    KTextEditor::MainWindow *m_interface;
    QHash<QString, QPointer<QObject>> m_pluginViews;
    QHash<KTextEditor::View*, QWidget*> m_viewBars;
    std::shared_ptr<ShowMessagesJob> m_showMessagesOutputJob;
};

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit Plugin(KTextEditor::Plugin* plugin, QObject* parent, const KPluginMetaData& metaData);
    ~Plugin() override;

    KXMLGUIClient* createGUIForMainWindow(Sublime::MainWindow *window) override;

    void unload() override;

    KTextEditor::Plugin *interface() const;

    KDevelop::ConfigPage* configPage(int number, QWidget *parent) override;
    int configPages() const override;

    QString pluginId() const;

private:
    QPointer<KTextEditor::Plugin> m_plugin;
    // view objects and tool views that should get deleted when the plugin gets unloaded
    KDevelop::ObjectListTracker *m_tracker;
};

void initialize();

}
#endif
