/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2012 Milian Wolff <mail@milianw.de>

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

#ifndef KDEVPLATFORM_ASSISTANTPOPUP_H
#define KDEVPLATFORM_ASSISTANTPOPUP_H

#include <QDeclarativeView>
#include <interfaces/iassistant.h>
#include <ksharedptr.h>
#include <QDebug>
#include <KAction>

#include <memory>

namespace KTextEditor
{
class View;
class Cursor;
}

class AssistantButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    AssistantButton(QAction* action, const QString& text, QObject* parent)
        : QObject(parent)
        , m_name(text)
        , m_action(action)
    { }

    QString name() const { return m_name; }
    Q_INVOKABLE void trigger() { m_action->trigger(); }

private:
    QString m_name;
    QAction* m_action;
};

class AssistantPopupConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor foreground READ foreground CONSTANT)
    Q_PROPERTY(QColor background READ background CONSTANT)
    Q_PROPERTY(QColor highlight READ highlight CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QList<QObject*> model READ model CONSTANT)

public:
    explicit AssistantPopupConfig(QObject *parent = 0);
    QColor foreground() const { return m_foreground; }
    QColor background() const { return m_background; }
    QColor highlight() const { return m_highlight; }
    QString title() const { return m_title; }
    QList<QObject*> model() const { return m_model; }
    void setTitle(const QString& text) { m_title = text; }
    void setModel(const QList<QObject*>& model) { m_model = model; }

    void setColorsFromView(QObject *view);

signals:
    void shouldShowHighlight(bool show);
    void shouldCancelAnimation();

private:
    QColor m_foreground;
    QColor m_background;
    QColor m_highlight;
    QString m_title;
    QList<QObject*> m_model;

    friend class AssistantPopup;
};

Q_DECLARE_METATYPE(AssistantPopupConfig*)

class AssistantPopup : public QDeclarativeView
{
    Q_OBJECT

public:
    typedef KSharedPtr<AssistantPopup> Ptr;
    /**
     * @p widget The widget below which the assistant should be shown.
     * The current main window will be used as parent widget for the popup.
     * This is to make use of the maximal space available and prevent any lines
     * in e.g. the editor to be hidden by the popup.
     */
    AssistantPopup(KTextEditor::View *widget, const KDevelop::IAssistant::Ptr &assistant);
    /**
     * @brief Like creating a new assistant, but faster.
     */
    void reset(KTextEditor::View *widget, const KDevelop::IAssistant::Ptr &assistant);
    KDevelop::IAssistant::Ptr assistant() const;
    virtual bool viewportEvent(QEvent *event);

public slots:
    void executeHideAction();
    void notifyReopened(bool reopened=true);

private slots:
    void updatePosition(KTextEditor::View* view, const KTextEditor::Cursor& newPos);

private:
    virtual bool eventFilter(QObject* object, QEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    /**
     * @brief Get the geometry of the inner part (with the text) of the KTextEditor::View being used.
     */
    QRect textWidgetGeometry(KTextEditor::View *view) const;

    void updateActions();
    QWidget* widgetForAction(const KDevelop::IAssistantAction::Ptr& action, int& mnemonic);
    KDevelop::IAssistant::Ptr m_assistant;
    QList<KDevelop::IAssistantAction::Ptr> m_assistantActions;
    QPointer<KTextEditor::View> m_view;
    std::unique_ptr<AssistantPopupConfig> m_config;
    bool m_shownAtBottom;
    bool m_reopening;
};

#endif // KDEVPLATFORM_ASSISTANTPOPUP_H
