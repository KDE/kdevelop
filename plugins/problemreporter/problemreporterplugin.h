/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROBLEMREPORTERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_PROBLEMREPORTERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariant>

#include <serialization/indexedstring.h>
#include <language/duchain/topducontext.h>

class QUrl;

namespace KDevelop
{
class IDocument;
}

class ProblemReporterModel;

class ProblemReporterPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit ProblemReporterPlugin(QObject* parent, const KPluginMetaData& metaData,
                                   const QVariantList& = QVariantList());
    ~ProblemReporterPlugin() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    // KDevelop::Plugin methods
    void unload() override;

    ProblemReporterModel* model() const;

private Q_SLOTS:
    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext&);
    void updateHighlight(const KDevelop::IndexedString& url);
    void documentClosed(KDevelop::IDocument*);
    void textDocumentCreated(KDevelop::IDocument* document);
    void documentUrlChanged(KDevelop::IDocument* document, const QUrl& previousUrl);
    void documentActivated(KDevelop::IDocument* document);
    void showModel(const QString& id);

private:
    void updateOpenedDocumentsHighlight();
    class ProblemReporterFactory* m_factory;
    ProblemReporterModel* m_model;

    class ProblemVisualizer;
    QHash<KDevelop::IndexedString, ProblemVisualizer*> m_visualizers;
    QSet<KDevelop::IndexedString> m_reHighlightNeeded;
};

#endif // KDEVPLATFORM_PLUGIN_PROBLEMREPORTERPLUGIN_H
