/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_USESWIDGET_H
#define KDEVPLATFORM_USESWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QSharedPointer>

#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/indexedtopducontext.h>
#include <language/languageexport.h>
#include "usescollector.h"
#include <language/editor/persistentmovingrange.h>

class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;
class QProgressBar;

namespace KDevelop {
class CodeRepresentation;

class IndexedDeclaration;
///A widget representing one use of a Declaration in a specific context
class KDEVPLATFORMLANGUAGE_EXPORT OneUseWidget
    : public QWidget
{
    Q_OBJECT

public:
    OneUseWidget(IndexedDeclaration declaration, const IndexedString& document, KTextEditor::Range range,
                 const CodeRepresentation& code);
    ~OneUseWidget() override;

    void setHighlighted(bool highlight);
    bool isHighlighted() const;
    void activateLink();

private:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent (QResizeEvent* event) override;

    PersistentMovingRange::Ptr m_range;
    IndexedDeclaration m_declaration;
    IndexedString m_document;
    QString m_sourceLine;

    QLabel* m_label;
    QLabel* m_icon;
    QHBoxLayout* m_layout;
    bool m_isHighlighted = false;
};

class KDEVPLATFORMLANGUAGE_EXPORT NavigatableWidgetList
    : public QScrollArea
{
    Q_OBJECT

public:
    /// High-priority items are displayed at the top of the widget,
    /// then Medium-priority, and at the bottom - Low-priority ones.
    enum class ItemPriority { High, Medium, Low };
    explicit NavigatableWidgetList(bool allowScrolling = false, uint maxHeight = 0, bool vertical = true);
    ~NavigatableWidgetList() override;
    void addItem(QWidget* widget, ItemPriority priority = ItemPriority::Medium);
    void addHeaderItem(QWidget* widget, Qt::Alignment alignment = {});
    ///Whether items were added to this list using addItem(..)
    bool hasItems() const;
    ///Deletes all items that were added using addItem
    void deleteItems();
    QList<QWidget*> items() const;
    void setShowHeader(bool show);

protected:
    QBoxLayout* m_itemLayout;
    QVBoxLayout* m_layout;

private:
    QHBoxLayout* m_headerLayout;
    bool m_allowScrolling, m_useArrows;
    int m_mediumPriorityPosition = 0; ///< index to insert Medium-priority items at
};

class KDEVPLATFORMLANGUAGE_EXPORT ContextUsesWidget
    : public NavigatableWidgetList
{
    Q_OBJECT

public:
    ContextUsesWidget(const CodeRepresentation& code, const QList<IndexedDeclaration>& usedDeclaration,
                      IndexedDUContext context);
Q_SIGNALS:
    void navigateDeclaration(KDevelop::IndexedDeclaration);

private Q_SLOTS:
    void linkWasActivated(const QString&);

private:
    IndexedDUContext m_context;
};

class KDEVPLATFORMLANGUAGE_EXPORT DeclarationWidget
    : public NavigatableWidgetList
{
    Q_OBJECT

public:
    DeclarationWidget(const KDevelop::CodeRepresentation& code, const KDevelop::IndexedDeclaration& declaration);
};

/**
 * Represents the uses of a declaration within one top-context
 */
class KDEVPLATFORMLANGUAGE_EXPORT TopContextUsesWidget
    : public NavigatableWidgetList
{
    Q_OBJECT

public:
    TopContextUsesWidget(IndexedDeclaration declaration, const QList<IndexedDeclaration>& localDeclarations,
                         IndexedTopDUContext topContext);
    void setExpanded(bool);
    int usesCount() const;

private Q_SLOTS:
    void labelClicked();

private:
    IndexedTopDUContext m_topContext;
    IndexedDeclaration m_declaration;
    QLabel* m_icon;
    QLabel* m_toggleButton;
    QList<IndexedDeclaration> m_allDeclarations;
    int m_usesCount;
};

/**
 * A widget that allows browsing through all the uses of a declaration, and also through all declarations of it.
 */
class KDEVPLATFORMLANGUAGE_EXPORT UsesWidget
    : public NavigatableWidgetList
{
    Q_OBJECT

public:
    ///This class can be overridden to do additional processing while the uses-widget shows the uses.
    struct KDEVPLATFORMLANGUAGE_EXPORT UsesWidgetCollector
        : public UsesCollector
    {
public:
        void setWidget(UsesWidget* widget);
        explicit UsesWidgetCollector(IndexedDeclaration decl);
        void processUses(KDevelop::ReferencedTopDUContext topContext) override;
        void maximumProgress(uint max) override;
        void progress(uint processed, uint total) override;
        UsesWidget* m_widget;
    };
    QSize sizeHint () const override;
    ///@param customCollector allows specifying an own subclass of UsesWidgetCollector.
    explicit UsesWidget(const IndexedDeclaration& declaration,
                        const QSharedPointer<UsesWidgetCollector>& customCollector = {});
    ~UsesWidget() override;
    void setAllExpanded(bool expanded);
    unsigned int countAllUses() const;
Q_SIGNALS:
    void navigateDeclaration(KDevelop::IndexedDeclaration);

private:
    const QString headerLineText() const;
    QLabel* m_headerLine;
    QSharedPointer<UsesWidgetCollector> m_collector;
    QProgressBar* m_progressBar;

public Q_SLOTS:
    void headerLinkActivated(const QString& linkName);
    void redrawHeaderLine();
};
}

#endif
