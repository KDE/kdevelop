/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVQMLJSPLUGIN_H
#define KDEVQMLJSPLUGIN_H

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>

class ModelManager;

class KDevQmlJsPlugin : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    explicit KDevQmlJsPlugin(QObject* parent, const KPluginMetaData& metaData,
                             const QVariantList& args = QVariantList());
    ~KDevQmlJsPlugin() override;

    KDevelop::ParseJob* createParseJob(const KDevelop::IndexedString& url) override;
    QString name() const override;

    KDevelop::ICodeHighlighting* codeHighlighting() const override;
    KDevelop::BasicRefactoring* refactoring() const override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;
    QPair<QWidget*, KTextEditor::Range> specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position) override;

private:
    KDevelop::ICodeHighlighting* m_highlighting;
    KDevelop::BasicRefactoring* m_refactoring;

    ModelManager* m_modelManager;
};

#endif // KDEVQMLJSPLUGIN_H
