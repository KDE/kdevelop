/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "externalscriptview.h"

#include "externalscriptplugin.h"
#include "externalscriptitem.h"
#include "editexternalscript.h"

#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QSortFilterProxyModel>

#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageBox_KDevCompat>

#include <util/scopeddialog.h>
#include <util/wildcardhelpers.h>

ExternalScriptView::ExternalScriptView(ExternalScriptPlugin* plugin, QWidget* parent)
    : QWidget(parent)
    , m_plugin(plugin)
{
    Ui::ExternalScriptViewBase::setupUi(this);
    setFocusProxy(filterText);

    setWindowTitle(i18nc("@title:window", "External Scripts"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-scripts"), windowIcon()));

    m_model = new QSortFilterProxyModel(this);
    m_model->setSourceModel(m_plugin->model());
    m_model->setDynamicSortFilter(true);
    m_model->sort(0);
    connect(filterText, &QLineEdit::textEdited, this, [this](const QString& text) {
        WildcardHelpers::setFilterNonPathWildcard(*m_model, text);
    });

    scriptTree->setModel(m_model);
    scriptTree->setContextMenuPolicy(Qt::CustomContextMenu);
    scriptTree->viewport()->installEventFilter(this);
    scriptTree->header()->hide();
    connect(scriptTree, &QTreeView::customContextMenuRequested,
            this, &ExternalScriptView::contextMenu);

    m_addScriptAction =
        new QAction(QIcon::fromTheme(QStringLiteral("document-new")), i18nc("@action", "Add External Script"), this);
    connect(m_addScriptAction, &QAction::triggered, this, &ExternalScriptView::addScript);
    addAction(m_addScriptAction);

    m_editScriptAction = new QAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18nc("@action",
                                         "Edit External Script"), this);
    connect(m_editScriptAction, &QAction::triggered, this, &ExternalScriptView::editScript);
    addAction(m_editScriptAction);

    m_removeScriptAction = new QAction(QIcon::fromTheme(QStringLiteral("document-close")), i18nc("@action",
                                           "Remove External Script"), this);
    connect(m_removeScriptAction, &QAction::triggered, this, &ExternalScriptView::removeScript);
    addAction(m_removeScriptAction);

    connect(scriptTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ExternalScriptView::validateActions);

    validateActions();
}

ExternalScriptView::~ExternalScriptView()
{
}

ExternalScriptItem* ExternalScriptView::currentItem() const
{
    return itemForIndex(scriptTree->currentIndex());
}

ExternalScriptItem* ExternalScriptView::itemForIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    const QModelIndex mappedIndex = m_model->mapToSource(index);
    return static_cast<ExternalScriptItem*>(m_plugin->model()->itemFromIndex(mappedIndex));
}

void ExternalScriptView::validateActions()
{
    bool itemSelected = currentItem();

    m_removeScriptAction->setEnabled(itemSelected);
    m_editScriptAction->setEnabled(itemSelected);
}

void ExternalScriptView::contextMenu(const QPoint& pos)
{
    QMenu menu(this);
    menu.addActions(actions());

    menu.exec(scriptTree->viewport()->mapToGlobal(pos));
}

bool ExternalScriptView::eventFilter(QObject* obj, QEvent* e)
{
    // no, listening to activated() is not enough since that would also trigger the edit mode which we do _not_ want here
    // users may still rename stuff via select + F2 though
    if (obj == scriptTree->viewport()) {
//     const bool singleClick = KGlobalSettings::singleClick();
        const bool singleClick = true; //FIXME: enable singleClick for the sake of porting, should find a proper way
        if ((!singleClick && e->type() == QEvent::MouseButtonDblClick) ||
            (singleClick && e->type() == QEvent::MouseButtonRelease)) {
            auto* mouseEvent = dynamic_cast<QMouseEvent*>(e);
            Q_ASSERT(mouseEvent);
            ExternalScriptItem* item = itemForIndex(scriptTree->indexAt(mouseEvent->pos()));
            if (item) {
                m_plugin->execute(item);
                e->accept();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, e);
}

void ExternalScriptView::addScript()
{
    auto* item = new ExternalScriptItem;
    KDevelop::ScopedDialog<EditExternalScript> dlg(item, this);
    if (dlg->exec() == QDialog::Accepted) {
        m_plugin->model()->appendRow(item);
    } else {
        delete item;
    }
}

void ExternalScriptView::removeScript()
{
    ExternalScriptItem* item = currentItem();
    if (!item) {
        return;
    }

    int ret = KMessageBox::questionTwoActions(
        this,
        i18n("<p>Do you really want to remove the external script configuration for <i>%1</i>?</p>"
             "<p><i>Note:</i> The script itself will not be removed.</p>",
             item->text()),
        i18nc("@title:window", "Confirm External Script Removal"),
        KGuiItem(i18nc("@action:button", "Remove"), QStringLiteral("document-close")), KStandardGuiItem::cancel());
    if (ret == KMessageBox::PrimaryAction) {
        m_plugin->model()->removeRow(
            m_plugin->model()->indexFromItem(item).row()
        );
    }
}

void ExternalScriptView::editScript()
{
    ExternalScriptItem* item = currentItem();
    if (!item) {
        return;
    }

    KDevelop::ScopedDialog<EditExternalScript> dlg(item, this);
    if (dlg->exec() == QDialog::Accepted) {
        item->save();
    }
}

#include "moc_externalscriptview.cpp"
