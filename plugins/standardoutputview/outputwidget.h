/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_OUTPUTWIDGET_H
#define KDEVPLATFORM_PLUGIN_OUTPUTWIDGET_H

#include <QHash>
#include <QRegularExpression>
#include <QWidget>

#include <interfaces/itoolviewactionlistener.h>

class KExpandableLineEdit;
class KToggleAction;
class StandardOutputViewTest;
class QAction;
class QAbstractItemView;
class QLineEdit;
class QModelIndex;
class QSortFilterProxyModel;
class QStackedWidget;
class QString;
class QTabWidget;
class QToolButton;
class QTreeView;
class QWidgetAction;
class ToolViewData;
class OutputWidgetConfig;

namespace KDevelop {
class IOutputViewModel;
}

class OutputWidget : public QWidget, public KDevelop::IToolViewActionListener
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IToolViewActionListener)

    friend class StandardOutputViewTest;

public:
    OutputWidget(QWidget* parent, const ToolViewData* data);
    ~OutputWidget() override;

    void removeOutput( int id );
    void raiseOutput( int id );
public Q_SLOTS:
    void addOutput( int id );
    void changeModel( int id );
    void changeDelegate( int id );
    void closeActiveView();
    void closeOtherViews();
    void selectFirstItem();
    void selectNextItem() override;
    void selectPreviousItem() override;
    void selectLastItem();
    void activate(const QModelIndex&);
    void setTitle(int outputId, const QString& title);

Q_SIGNALS:
    void outputRemoved(int id);

private Q_SLOTS:
    void nextOutput();
    void previousOutput();
    void setWordWrap(bool);
    void copySelection();
    void selectAll();
    void outputFilter(const QString& filter);
    void updateFilter(int index);
    void currentViewChanged(int index);
    void clearModel();

private:
    enum SelectionMode {
        Last,
        Next,
        Previous,
        First
    };
    void selectItem(SelectionMode selectionMode);

    QTreeView* createListView(int id);
    void setCurrentWidget( QTreeView* view );
    QWidget* currentWidget() const;
    void enableActions();
    KDevelop::IOutputViewModel* outputViewModel() const;
    QAbstractItemView* outputView() const;
    void activateIndex(const QModelIndex& index, QAbstractItemView* view, KDevelop::IOutputViewModel* iface);
    void eventuallyDoFocus();

    /**
     * Closes @p view and destroys all associated objects
     * @param view a nonnull widget in @a m_tabwidget or @a m_stackwidget
     * @return @c true if the view was closed successfully, @c false otherwise
     */
    bool closeView(const QWidget* view);

    template<class ViewContainer>
    void closeFirstViewIfTooMany(const ViewContainer& viewContainer);
    template<class ViewContainer>
    void closeFirstViewsWhileTooMany(const ViewContainer& viewContainer, int maxViewCount);

    struct FilteredView {
        QTreeView* view = nullptr;
        QSortFilterProxyModel* proxyModel = nullptr;
        /// Contains possibly invalid pattern entered by the user verbatim, which may differ
        /// from always valid (if proxyModel != nullptr) proxyModel->filterRegularExpression().
        QRegularExpression filter;
    };
    using FilteredViews = QHash<int, FilteredView>;

    static FilteredViews::const_iterator constIterator(FilteredViews::iterator it);
    template<typename ForwardIt>
    static ForwardIt findFilteredView(ForwardIt first, ForwardIt last, const QAbstractItemView* view);

    FilteredViews::iterator findFilteredView(const QAbstractItemView* view);
    FilteredViews::const_iterator constFindFilteredView(const QAbstractItemView* view) const;

    void updateFilterInputAppearance(FilteredViews::const_iterator currentView);

    FilteredViews m_views;
    QTabWidget* m_tabwidget;
    QStackedWidget* m_stackwidget;
    const ToolViewData* data;
    QToolButton* m_closeButton;
    QAction* m_closeOthersAction;
    QAction* m_nextAction;
    QAction* m_previousAction;
    KToggleAction* m_activateOnSelect;
    KToggleAction* m_focusOnSelect;
    KExpandableLineEdit* m_filterInput;
    QWidgetAction* m_filterAction;
    OutputWidgetConfig* m_outputWidgetConfig;
    bool m_wordWrap = false;
};

#endif

