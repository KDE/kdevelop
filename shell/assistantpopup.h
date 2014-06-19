/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2012 Milian Wolff <mail@milianw.de>
   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

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

namespace KTextEditor
{
class View;
class Cursor;
}

class AssistantPopupConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor foreground READ foreground CONSTANT)
    Q_PROPERTY(QColor background READ background CONSTANT)
    Q_PROPERTY(QColor highlight READ highlight CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QList<QObject*> model READ model CONSTANT)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

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

    bool isActive() const;
    void setActive(bool active);

signals:
    void activeChanged(bool active);

private:
    QColor m_foreground;
    QColor m_background;
    QColor m_highlight;
    QString m_title;
    QList<QObject*> m_model;
    bool m_active;

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

public slots:
    void executeHideAction();
    void notifyReopened(bool reopened=true);

private slots:
    void updatePosition(KTextEditor::View* view, const KTextEditor::Cursor& newPos);

protected:
    virtual bool eventFilter(QObject* object, QEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual bool viewportEvent(QEvent *event);

private:
    void updateActions();

    /**
     * @brief Get the geometry of the inner part (with the text) of the KTextEditor::View being used.
     */
    QRect textWidgetGeometry(KTextEditor::View *view) const;

    KDevelop::IAssistant::Ptr m_assistant;
    QList<KDevelop::IAssistantAction::Ptr> m_assistantActions;
    QPointer<KTextEditor::View> m_view;
    QScopedPointer<AssistantPopupConfig> m_config;
    bool m_shownAtBottom;
    bool m_reopening;
};

#endif // KDEVPLATFORM_ASSISTANTPOPUP_H
