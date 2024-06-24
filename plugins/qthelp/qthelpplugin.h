/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPPLUGIN_H
#define QTHELPPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/idocumentationproviderprovider.h>

class QtHelpProvider;
class QtHelpQtDoc;
class QtHelpDocumentation;

class QtHelpPlugin : public KDevelop::IPlugin, public KDevelop::IDocumentationProviderProvider
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDocumentationProviderProvider )
public:
    QtHelpPlugin(QObject* parent, const QVariantList& args);
    ~QtHelpPlugin() override;

    QList<KDevelop::IDocumentationProvider*> providers() override;
    QList<QtHelpProvider*> qtHelpProviderLoaded();
    bool isQtHelpQtDocLoaded() const;
    bool isQtHelpAvailable() const;
    bool isInitialized() const;

    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

public Q_SLOTS:
    void readConfig();

Q_SIGNALS:
    void changedProvidersList() override;

private:
    void loadQtDocumentation(bool loadQtDoc);
    void searchHelpDirectory(QStringList& pathList, QStringList& nameList, QStringList& iconList,
                             const QString& searchDir);
    void loadQtHelpProvider(const QStringList& pathList, const QStringList& nameList, const QStringList& iconList);

    QList<QtHelpProvider*> m_qtHelpProviders;
    QtHelpQtDoc* m_qtDoc;
    bool m_loadSystemQtDoc;
};

#endif // QTHELPPLUGIN_H
