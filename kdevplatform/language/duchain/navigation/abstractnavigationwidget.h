/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ABSTRACTNAVIGATIONWIDGET_H
#define KDEVPLATFORM_ABSTRACTNAVIGATIONWIDGET_H

#include <language/languageexport.h>
#include <QPointer>
#include <QWidget>

#include "../../interfaces/quickopendataprovider.h"
#include "abstractnavigationcontext.h"

class QTextBrowser;

namespace KDevelop {
class AbstractNavigationWidgetPrivate;

/**
 * This class deleted itself when its part is deleted, so always use a QPointer when referencing it.
 * The duchain must be read-locked for most operations
 * */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractNavigationWidget
    : public QWidget
    , public QuickOpenEmbeddedWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::QuickOpenEmbeddedWidgetInterface)

public:
    enum DisplayHint {
        NoHints = 0x0, // < Normal display
        EmbeddableWidget = 0x1, // < Omit parts which are only useful for the navigation popup
    };
    Q_DECLARE_FLAGS(DisplayHints, DisplayHint)
    AbstractNavigationWidget();
    ~AbstractNavigationWidget() override;

    void setContext(NavigationContextPointer context, int initBrowser = 400);
    void setDisplayHints(DisplayHints hints);

    QSize sizeHint() const override;

    NavigationContextPointer context() const;

public Q_SLOTS:
    /// keyboard navigation support
    bool next() override;
    bool previous() override;
    bool up() override;
    bool down() override;
    void accept() override;
    void back() override;
    void resetNavigationState() override;

    ///These are temporarily for getting these events directly from kate
    ///@todo Do this through a public interface post 4.2
    void embeddedWidgetRight();
    ///Keyboard-action "previous"
    void embeddedWidgetLeft();
    ///Keyboard-action "accept"
    void embeddedWidgetAccept();
    void embeddedWidgetUp();
    void embeddedWidgetDown();

    void navigateDeclaration(const KDevelop::IndexedDeclaration& decl);

Q_SIGNALS:
    void sizeHintChanged();
    /// Emitted whenever the current navigation-context has changed
    /// @param wasInitial whether the previous context was the initial context
    /// @param isInitial whether the current context is the initial context
    void contextChanged(bool wasInitial, bool isInitial);

protected:
    void wheelEvent(QWheelEvent*) override;
    void updateIdealSize() const;

    void initBrowser(int height);
    void update();

private:
    const QScopedPointer<class AbstractNavigationWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractNavigationWidget)
};
}

#endif
