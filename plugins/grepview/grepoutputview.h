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

    static const int HISTORY_SIZE;
    QAction* m_next;
    QAction* m_prev;
    QAction* m_collapseAll;
    QAction* m_expandAll;
    QAction* m_refresh;
    QAction* m_clearSearchHistory;
    QLabel*  m_statusLabel;
    GrepViewPlugin *m_plugin;
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
    void updateCheckable();
};

#endif // KDEVPLATFORM_PLUGIN_GREPOUTPUTVIEW_H
