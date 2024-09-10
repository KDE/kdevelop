/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariant>
#include <QMimeType>
#include <unordered_map>

namespace Sublime
{
    class MainWindow;
}

namespace KDevelop
{
    class IDocument;
}

class DocumentSwitcherTreeView;

class QStandardItemModel;
class QModelIndex;
class QAction;

class DocumentSwitcherPlugin: public KDevelop::IPlugin {
    Q_OBJECT
public:
    explicit DocumentSwitcherPlugin(QObject* parent, const KPluginMetaData& metaData,
                                    const QVariantList& args = QVariantList());
    ~DocumentSwitcherPlugin() override;

    void unload() override;
public Q_SLOTS:
    void itemActivated( const QModelIndex& );
    void switchToClicked(const QModelIndex& );
    void walkForward();
    void walkBackward();
    void documentOpened(KDevelop::IDocument *document);
    void documentActivated(KDevelop::IDocument *document);
    void documentClosed(KDevelop::IDocument *document);
    void documentUrlChanged(KDevelop::IDocument* document, QUrl const&);

protected:
    bool eventFilter( QObject*, QEvent* ) override;
private:
    void setViewGeometry(Sublime::MainWindow* window);
    void enableActions();
    void fillModel();
    void walk(const int from, const int to);

    // List of opened document sorted activation.
    QList<KDevelop::IDocument *> documentLists;
    std::unordered_map<KDevelop::IDocument*, QMimeType> cachedMimeTypes;
    DocumentSwitcherTreeView* view;
    QStandardItemModel* model;
    QAction* forwardAction;
    QAction* backwardAction;
};

#endif

