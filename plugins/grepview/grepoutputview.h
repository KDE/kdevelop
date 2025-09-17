/*
    SPDX-FileCopyrightText: 2010 Silvère Lestang <silvere.lestang@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GREPOUTPUTVIEW_H
#define KDEVPLATFORM_PLUGIN_GREPOUTPUTVIEW_H

#include <QList>

#include <interfaces/iuicontroller.h>
#include <interfaces/itoolviewactionlistener.h>

#include "ui_grepoutputview.h"

class QModelIndex;

class GrepViewPlugin;
class GrepOutputModel;
struct GrepJobSettings;

class GrepOutputViewFactory: public KDevelop::IToolViewFactory
{
public:
    explicit GrepOutputViewFactory(GrepViewPlugin* plugin);
    QWidget* create(QWidget* parent = nullptr) override;
    Qt::DockWidgetArea defaultPosition() const override;
    QString id() const override;
private:
    GrepViewPlugin* m_plugin;
};

class GrepOutputView : public QWidget, Ui::GrepOutputView, public KDevelop::IToolViewActionListener
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IToolViewActionListener)

public:
    enum MessageType {
        Information,
        Error
    };

    GrepOutputView(QWidget* parent, GrepViewPlugin* plugin);
    ~GrepOutputView() override;
    GrepOutputModel* model();

    /**
     * Create a model for each search settings history entry.
     *
     * Each search settings history entry is specified by the elements of
     * @p settingsHistory and @p searchDescriptions at matching positions.
     *
     * @pre @p settingsHistory.size() == @p searchDescriptions.size()
     * @pre @p !settingsHistory.empty()
     */
    void addModelsFromHistory(QList<GrepJobSettings>&& settingsHistory, const QStringList& searchDescriptions);

    /**
     * This causes the creation of a new model, the old one is kept in model history.
     * Oldest models are deleted if needed.
     * @return pointer to the new model
     */
    GrepOutputModel* renewModel(const GrepJobSettings& settings, const QString& description);
    
    void setMessage(const QString& msg, MessageType type = Information);

public Q_SLOTS:
    void showErrorMessage( const QString& errorMessage );
    void showMessage(const QString& message);
    void updateApplyState(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void changeModel(int index);

Q_SIGNALS:
    void outputViewIsClosed();
    
private:
    void replacementTextChanged(const QString& replacementText);

    /**
     * Remove oldest model and settings from history if the history size exceeds @a HISTORY_SIZE.
     */
    void removeOldestModelsIfTooMany();

    static const int HISTORY_SIZE;
    QAction* m_next;
    QAction* m_prev;
    QAction* m_collapseAll;
    QAction* m_expandAll;
    QAction* m_refresh;
    QAction* m_clearSearchHistory;
    QLabel*  m_statusLabel;
    GrepViewPlugin *m_plugin;

    /**
     * Invariants:
     * 1. @a m_settingsHistory.size() == @a modelSelector->count()
     * 2. @a m_settingsHistory.size() <= @a HISTORY_SIZE
     * 3. The order of @a m_settingsHistory's elements is the reverse of the order of matching
     *    @a modelSelector's items. Specifically, the oldest search settings are at the front of
     *    @a m_settingsHistory and the oldest model is in the last item of @a modelSelector.
     */
    QVector<GrepJobSettings> m_settingsHistory;

private Q_SLOTS:
    void selectPreviousItem() override;
    void selectNextItem() override;
    void collapseAllItems();
    void expandAllItems();
    void onApply();
    void showDialog();
    void refresh();
    void rowsInserted(const QModelIndex& parent);
    void updateButtonState(bool enable);
    void rowsRemoved();
    void clearSearchHistory();
    void modelSelectorContextMenu(const QPoint& pos);
    void updateScrollArea();
};

#endif // KDEVPLATFORM_PLUGIN_GREPOUTPUTVIEW_H
