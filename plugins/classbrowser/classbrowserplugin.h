/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_CLASSBROWSERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_CLASSBROWSERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariant>
#include <language/duchain/duchainpointer.h>

class ClassTree;

namespace KDevelop {
class Declaration;
}

class ClassBrowserPlugin
    : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit ClassBrowserPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~ClassBrowserPlugin() override;

    void setActiveClassTree(ClassTree* a_classTree) { m_activeClassTree = a_classTree; }

public: // KDevelop::Plugin overrides
    void unload() override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    // The duchain must not be locked when this is called!
    void showDefinition(const KDevelop::DeclarationPointer& declaration);

private Q_SLOTS:
    void findInClassBrowser();

private:
    class ClassBrowserFactory* m_factory;
    ClassTree* m_activeClassTree;
    QAction* m_findInBrowser;
};

#endif // KDEVPLATFORM_PLUGIN_CLASSBROWSERPLUGIN_H
