/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "documentationview.h"

#include <QWidgetAction>
#include <QAction>
#include <QIcon>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCompleter>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QShortcut>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentationprovider.h>
#include <interfaces/idocumentationproviderprovider.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>
#include "documentationfindwidget.h"
#include "debug.h"

using namespace KDevelop;

DocumentationView::DocumentationView(QWidget* parent, ProvidersModel* model)
    : QWidget(parent), mProvidersModel(model)
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("documentation"), windowIcon()));
    setWindowTitle(i18n("Documentation"));

    setLayout(new QVBoxLayout(this));
    layout()->setMargin(0);
    layout()->setSpacing(0);

    mFindDoc = new DocumentationFindWidget;
    mFindDoc->hide();

    // insert placeholder widget at location of doc view
    layout()->addWidget(new QWidget(this));
    layout()->addWidget(mFindDoc);

    setupActions();

    mCurrent = mHistory.end();

    setFocusProxy(mIdentifiers);

    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

QList<QAction*> DocumentationView::contextMenuActions() const
{
    // TODO: also show providers
    return {mBack, mForward, mHomeAction, mSeparatorBeforeFind, mFind};
}

void DocumentationView::setupActions()
{
    // use custom QAction's with createWidget for mProviders and mIdentifiers
    mBack = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), i18n("Back"), this);
    mBack->setEnabled(false);
    connect(mBack, &QAction::triggered, this, &DocumentationView::browseBack);
    addAction(mBack);

    mForward = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), i18n("Forward"), this);
    mForward->setEnabled(false);
    connect(mForward, &QAction::triggered, this, &DocumentationView::browseForward);
    addAction(mForward);

    mHomeAction = new QAction(QIcon::fromTheme(QStringLiteral("go-home")), i18n("Home"), this);
    mHomeAction->setEnabled(false);
    connect(mHomeAction, &QAction::triggered, this, &DocumentationView::showHome);
    addAction(mHomeAction);

    mProviders = new QComboBox(this);
    auto providersAction = new QWidgetAction(this);
    providersAction->setDefaultWidget(mProviders);
    addAction(providersAction);

    mIdentifiers = new QLineEdit(this);
    mIdentifiers->setEnabled(false);
    mIdentifiers->setClearButtonEnabled(true);
    mIdentifiers->setPlaceholderText(i18n("Search..."));
    mIdentifiers->setCompleter(new QCompleter(mIdentifiers));
//     mIdentifiers->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mIdentifiers->completer()->setCaseSensitivity(Qt::CaseInsensitive);

    /* vertical size policy should be left to the style. */
    mIdentifiers->setSizePolicy(QSizePolicy::Expanding, mIdentifiers->sizePolicy().verticalPolicy());
    connect(mIdentifiers->completer(), QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, &DocumentationView::changedSelection);
    connect(mIdentifiers, &QLineEdit::returnPressed, this, &DocumentationView::returnPressed);
    auto identifiersAction = new QWidgetAction(this);
    identifiersAction->setDefaultWidget(mIdentifiers);
    addAction(identifiersAction);

    mSeparatorBeforeFind = new QAction(this);
    mSeparatorBeforeFind->setSeparator(true);
    addAction(mSeparatorBeforeFind);

    mFind = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("Find in Text..."), this);
    mFind->setToolTip(i18n("Find in text of current documentation page."));
    mFind->setEnabled(false);
    connect(mFind, &QAction::triggered, mFindDoc, &DocumentationFindWidget::startSearch);
    addAction(mFind);

    auto closeFindBarShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    closeFindBarShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(closeFindBarShortcut, &QShortcut::activated, mFindDoc, &QWidget::hide);
}

void DocumentationView::initialize()
{
    mProviders->setModel(mProvidersModel);
    connect(mProviders, QOverload<int>::of(&QComboBox::activated), this, &DocumentationView::changedProvider);
    connect(mProvidersModel, &ProvidersModel::providersChanged, this, &DocumentationView::emptyHistory);

    const bool hasProviders = (mProviders->count() > 0);
    mHomeAction->setEnabled(hasProviders);
    mIdentifiers->setEnabled(hasProviders);
    if (hasProviders) {
        changedProvider(0);
    }
}

void DocumentationView::browseBack()
{
    --mCurrent;
    mBack->setEnabled(mCurrent != mHistory.begin());
    mForward->setEnabled(true);

    updateView();
}

void DocumentationView::browseForward()
{
    ++mCurrent;
    mForward->setEnabled(mCurrent+1 != mHistory.end());
    mBack->setEnabled(true);

    updateView();
}

void DocumentationView::showHome()
{
    auto prov = mProvidersModel->provider(mProviders->currentIndex());

    showDocumentation(prov->homePage());
}

void DocumentationView::returnPressed()
{
    // Exit if search text is empty. It's necessary because of empty
    // line edit text not leads to "empty" completer indexes.
    if (mIdentifiers->text().isEmpty())
        return;

    // Exit if completer popup has selected item - in this case 'Return'
    // key press emits QCompleter::activated signal which is already connected.
    if (mIdentifiers->completer()->popup()->currentIndex().isValid())
        return;

    // If user doesn't select any item in popup we will try to use the first row.
    if (mIdentifiers->completer()->setCurrentRow(0))
        changedSelection(mIdentifiers->completer()->currentIndex());
}

void DocumentationView::changedSelection(const QModelIndex& idx)
{
    if (idx.isValid()) {
        // Skip view update if user try to show already opened documentation
        mIdentifiers->setText(idx.data(Qt::DisplayRole).toString());
        if (mIdentifiers->text() == (*mCurrent)->name()) {
            return;
        }

        IDocumentationProvider* prov = mProvidersModel->provider(mProviders->currentIndex());
        auto doc = prov->documentationForIndex(idx);
        if (doc) {
            showDocumentation(doc);
        }
    }
}

void DocumentationView::showDocumentation(const IDocumentation::Ptr& doc)
{
    qCDebug(DOCUMENTATION) << "showing" << doc->name();

    mBack->setEnabled(!mHistory.isEmpty());
    mForward->setEnabled(false);

    // clear all history following the current item, unless we're already
    // at the end (otherwise this code crashes when history is empty, which
    // happens when addHistory is first called on startup to add the
    // homepage)
    if (mCurrent+1 < mHistory.end()) {
        mHistory.erase(mCurrent+1, mHistory.end());
    }

    mHistory.append(doc);
    mCurrent = mHistory.end()-1;

    // NOTE: we assume an existing widget was used to navigate somewhere
    //       but this history entry actually contains the new info for the
    //       title... this is ugly and should be refactored somehow
    if (mIdentifiers->completer()->model() == (*mCurrent)->provider()->indexModel()) {
        mIdentifiers->setText((*mCurrent)->name());
    }

    updateView();
}

void DocumentationView::emptyHistory()
{
    mHistory.clear();
    mCurrent = mHistory.end();
    mBack->setEnabled(false);
    mForward->setEnabled(false);
    const bool hasProviders = (mProviders->count() > 0);
    mHomeAction->setEnabled(hasProviders);
    mIdentifiers->setEnabled(hasProviders);
    if (hasProviders) {
        mProviders->setCurrentIndex(0);
        changedProvider(0);
    } else {
        updateView();
    }
}

void DocumentationView::updateView()
{
    if (mCurrent != mHistory.end()) {
        mProviders->setCurrentIndex(mProvidersModel->rowForProvider((*mCurrent)->provider()));
        mIdentifiers->completer()->setModel((*mCurrent)->provider()->indexModel());
        mIdentifiers->setText((*mCurrent)->name());
        mIdentifiers->completer()->setCompletionPrefix((*mCurrent)->name());
    } else {
        mIdentifiers->clear();
    }

    QLayoutItem* lastview = layout()->takeAt(0);
    Q_ASSERT(lastview);

    if (lastview->widget()->parent() == this) {
        lastview->widget()->deleteLater();
    }

    delete lastview;

    mFindDoc->setEnabled(false);
    QWidget* w;
    if (mCurrent != mHistory.end()) {
        w = (*mCurrent)->documentationWidget(mFindDoc, this);
        Q_ASSERT(w);
        QWidget::setTabOrder(mIdentifiers, w);
    } else {
        // placeholder widget at location of doc view
        w = new QWidget(this);
    }

    mFind->setEnabled(mFindDoc->isEnabled());
    if (!mFindDoc->isEnabled()) {
        mFindDoc->hide();
    }

    QLayoutItem* findWidget = layout()->takeAt(0);
    layout()->addWidget(w);
    layout()->addItem(findWidget);
}

void DocumentationView::changedProvider(int row)
{
    mIdentifiers->completer()->setModel(mProvidersModel->provider(row)->indexModel());
    mIdentifiers->clear();

    showHome();
}

////////////// ProvidersModel //////////////////

ProvidersModel::ProvidersModel(QObject* parent)
    : QAbstractListModel(parent)
    , mProviders(ICore::self()->documentationController()->documentationProviders())
{
    connect(ICore::self()->pluginController(), &IPluginController::unloadingPlugin, this, &ProvidersModel::unloaded);
    connect(ICore::self()->pluginController(), &IPluginController::pluginLoaded, this, &ProvidersModel::loaded);
    connect(ICore::self()->documentationController(), &IDocumentationController::providersChanged, this, &ProvidersModel::reloadProviders);
}

void ProvidersModel::reloadProviders()
{
    beginResetModel();
    mProviders = ICore::self()->documentationController()->documentationProviders();

    std::sort(mProviders.begin(), mProviders.end(),
              [](const KDevelop::IDocumentationProvider* a, const KDevelop::IDocumentationProvider* b) {
        return a->name() < b->name();
    });

    endResetModel();
    emit providersChanged();
}

QVariant ProvidersModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= mProviders.count() || index.row() < 0)
        return QVariant();

    QVariant ret;
    switch (role)
    {
    case Qt::DisplayRole:
        ret = provider(index.row())->name();
        break;
    case Qt::DecorationRole:
        ret = provider(index.row())->icon();
        break;
    }
    return ret;
}

void ProvidersModel::addProvider(IDocumentationProvider* provider)
{
    if (!provider || mProviders.contains(provider))
        return;

    int pos = 0;
    while (pos < mProviders.size() && mProviders[pos]->name() < provider->name())
        ++pos;

    beginInsertRows(QModelIndex(), pos, pos);
    mProviders.insert(pos, provider);
    endInsertRows();

    emit providersChanged();
}

void ProvidersModel::removeProvider(IDocumentationProvider* provider)
{
    int pos;
    if (!provider || (pos = mProviders.indexOf(provider)) < 0)
        return;

    beginRemoveRows(QModelIndex(), pos, pos);
    mProviders.removeAt(pos);
    endRemoveRows();

    emit providersChanged();
}

void ProvidersModel::unloaded(IPlugin* plugin)
{
    removeProvider(plugin->extension<IDocumentationProvider>());

    auto* providerProvider = plugin->extension<IDocumentationProviderProvider>();
    if (providerProvider) {
        const auto providers = providerProvider->providers();
        for (IDocumentationProvider* provider : providers) {
            removeProvider(provider);
        }
    }
}

void ProvidersModel::loaded(IPlugin* plugin)
{
    addProvider(plugin->extension<IDocumentationProvider>());

    auto* providerProvider = plugin->extension<IDocumentationProviderProvider>();
    if (providerProvider) {
        const auto providers = providerProvider->providers();
        for (IDocumentationProvider* provider : providers) {
            addProvider(provider);
        }
    }
}

int ProvidersModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : mProviders.count();
}

int ProvidersModel::rowForProvider(IDocumentationProvider* provider)
{
    return mProviders.indexOf(provider);
}

IDocumentationProvider* ProvidersModel::provider(int pos) const
{
    return mProviders[pos];
}

QList<IDocumentationProvider*> ProvidersModel::providers()
{
    return mProviders;
}
