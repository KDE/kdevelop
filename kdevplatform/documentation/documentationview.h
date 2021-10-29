/*
    SPDX-FileCopyrightText: 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DOCUMENTATIONVIEW_H
#define KDEVPLATFORM_DOCUMENTATIONVIEW_H

#include <QWidget>
#include <QAbstractListModel>
#include <interfaces/idocumentation.h>
#include "documentationexport.h"

namespace KDevelop {
    class IPlugin;
    class DocumentationFindWidget;
}

class QModelIndex;
class QLineEdit;
class ProvidersModel;
class QComboBox;

class KDEVPLATFORMDOCUMENTATION_EXPORT DocumentationView : public QWidget
{
    Q_OBJECT
public:
    DocumentationView(QWidget* parent, ProvidersModel* m);

public:
    QList<QAction*> contextMenuActions() const;

public Q_SLOTS:
    void initialize();

    void showDocumentation(const KDevelop::IDocumentation::Ptr& doc);
    void emptyHistory();

    void tryBrowseForward();
    void tryBrowseBack();
    void browseForward();
    void browseBack();
    void changedSelection(const QModelIndex& idx);
    void changedProvider(int);
    void showHome();

private:
    void mousePressEvent(QMouseEvent* event) override;
    void setupActions();
    void updateView();
    void returnPressed();

    QAction* mForward;
    QAction* mBack;
    QAction* mHomeAction;
    QAction* mSeparatorBeforeFind;
    QAction* mFind;
    QLineEdit* mIdentifiers;
    QList< KDevelop::IDocumentation::Ptr > mHistory;
    QList< KDevelop::IDocumentation::Ptr >::iterator mCurrent;
    QComboBox* mProviders;
    ProvidersModel* mProvidersModel;
    KDevelop::DocumentationFindWidget* mFindDoc;
};

class KDEVPLATFORMDOCUMENTATION_EXPORT ProvidersModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ProvidersModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& idx = QModelIndex()) const override;
    QList<KDevelop::IDocumentationProvider*> providers();
    KDevelop::IDocumentationProvider* provider(int pos) const;
    int rowForProvider(KDevelop::IDocumentationProvider* provider);

public Q_SLOTS:
    void unloaded(KDevelop::IPlugin* p);
    void loaded(KDevelop::IPlugin* p);
    void reloadProviders();

private:
    void addProvider(KDevelop::IDocumentationProvider* provider);
    void removeProvider(KDevelop::IDocumentationProvider* provider);

    QList<KDevelop::IDocumentationProvider*> mProviders;
Q_SIGNALS:
    void providersChanged();
};

#endif // KDEVPLATFORM_DOCUMENTATIONVIEW_H
