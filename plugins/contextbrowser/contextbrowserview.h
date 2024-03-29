/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_CONTEXTBROWSERVIEW_H
#define KDEVPLATFORM_PLUGIN_CONTEXTBROWSERVIEW_H

#include <QWidget>
#include <QPointer>

#include <language/duchain/topducontext.h>
#include <interfaces/itoolviewactionlistener.h>

class ContextBrowserPlugin;
class QVBoxLayout;
class QHBoxLayout;
class QAction;
class KToggleAction;

namespace KDevelop {
class IDocument;
}

class ContextBrowserView
    : public QWidget
    , public KDevelop::IToolViewActionListener
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IToolViewActionListener)

public:
    ContextBrowserView(ContextBrowserPlugin*, QWidget* parent);
    ~ContextBrowserView() override;

    //duchain must be locked
    void setContext(KDevelop::DUContext* context);
    void setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, bool force = false);
    void setSpecialNavigationWidget(QWidget*);
    void updateMainWidget(QWidget*);

    /** Allows a single update of the view even if in locked state.
     * This is needed to browse while the locked button is checked. */
    void allowLockedUpdate();

    ///Returns the currently locked declaration, or invalid of none is locked atm.
    KDevelop::IndexedDeclaration lockedDeclaration() const;

    void setNavigationWidget(QWidget* widget);

    QWidget* navigationWidget()
    {
        return m_navigationWidget;
    }

    //duchain must be locked
    KDevelop::AbstractNavigationWidget* createWidget(KDevelop::DUContext* context);

    //duchain must be locked
    KDevelop::AbstractNavigationWidget* createWidget(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext);

    KDevelop::IndexedDeclaration declaration() const;

    ///Returns whether the view is currently locked
    bool isLocked() const;

private Q_SLOTS:
    void declarationMenu();
    void navigationContextChanged(bool wasInitial, bool isInitial);
    void selectNextItem() override;
    void selectPreviousItem() override;

private:
    void showEvent(QShowEvent* event) override;
    bool event(QEvent* event) override;

    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void resetWidget();

private:

    KDevelop::IndexedDeclaration m_declaration;

    ContextBrowserPlugin* m_plugin;
    QVBoxLayout* m_layout;
    KToggleAction* m_lockAction;
    QAction* m_declarationMenuAction;
    QHBoxLayout* m_buttons;

    QPointer<QWidget> m_navigationWidget;
    KDevelop::DeclarationId m_navigationWidgetDeclaration;
    bool m_allowLockedUpdate;
    KDevelop::IndexedTopDUContext m_lastUsedTopContext;

    KDevelop::IndexedDUContext m_context;
    // Whether the lock-button was activated automatically due to user navigation
    bool m_autoLocked;
};

#endif
