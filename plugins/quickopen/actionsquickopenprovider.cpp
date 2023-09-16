/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "actionsquickopenprovider.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QIcon>
#include <QAction>
#include <QRegularExpression>

using namespace KDevelop;

class ActionsQuickOpenItem
    : public QuickOpenDataBase
{
public:
    ActionsQuickOpenItem(const QString& display, QAction* action)
        : QuickOpenDataBase()
        , m_action(action)
        , m_display(display)
    {}

    QString text() const override
    {
        return m_display;
    }
    QString htmlDescription() const override
    {
        QString desc = m_action->toolTip();
        const QKeySequence shortcut = m_action->shortcut();
        if (!shortcut.isEmpty()) {
            desc = i18nc("description (shortcut)", "%1 (%2)", desc, shortcut.toString());
        }
        return desc;
    }
    bool execute(QString&) override
    {
        m_action->trigger();
        return true;
    }
    QIcon icon() const override
    {
        const QIcon icon = m_action->icon();
        if (icon.isNull()) {
            // note: not the best fallback icon, but can't find anything better
            return QIcon::fromTheme(QStringLiteral("system-run"));
        }

        return icon;
    }

private:
    QAction* const m_action;

    ///needed because it won't have the "E&xit" ampersand
    const QString m_display;
};

ActionsQuickOpenProvider::ActionsQuickOpenProvider()
{
}

void ActionsQuickOpenProvider::setFilterText(const QString& text)
{
    if (text.size() < 2) {
        return;
    }
    m_results.clear();
    const QList<KActionCollection*> collections = KActionCollection::allCollections();
    QRegularExpression mnemonicRx(QStringLiteral("^(.*)&(.+)$"));
    for (KActionCollection* c : collections) {
        const QList<QAction*> actions = c->actions();
        for (QAction* action : actions) {
            if (!action->isEnabled()) {
                continue;
            }

            QString display = action->text();
            QRegularExpressionMatch match = mnemonicRx.match(display);
            if (match.hasMatch()) {
                display = match.capturedView(1) + match.capturedView(2);
            }

            if (display.contains(text, Qt::CaseInsensitive)) {
                m_results += QuickOpenDataPointer(new ActionsQuickOpenItem(display, action));
            }
        }
    }
}

uint ActionsQuickOpenProvider::unfilteredItemCount() const
{
    uint ret = 0;
    const QList<KActionCollection*> collections = KActionCollection::allCollections();
    for (KActionCollection* c : collections) {
        ret += c->count();
    }

    return ret;
}

QuickOpenDataPointer ActionsQuickOpenProvider::data(uint row) const
{
    return m_results.at(row);
}

uint ActionsQuickOpenProvider::itemCount() const
{
    return m_results.count();
}

void ActionsQuickOpenProvider::reset()
{
    m_results.clear();
}

#include "moc_actionsquickopenprovider.cpp"
