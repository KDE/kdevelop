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
    Q_PROPERTY(QColor foreground READ foreground NOTIFY colorsChanged)
    Q_PROPERTY(QColor background READ background NOTIFY colorsChanged)
    Q_PROPERTY(QColor highlight READ highlight NOTIFY colorsChanged)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QList<QObject*> model READ model NOTIFY modelChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

public:
    explicit AssistantPopupConfig(QObject *parent = 0);

    QColor foreground() const { return m_foreground; }
    QColor background() const { return m_background; }
    QColor highlight() const { return m_highlight; }

    QString title() const { return m_title; }
    void setTitle(const QString& title);
    QList<QObject*> model() const { return m_model; }
    void setModel(const QList<QObject*>& model);

    void setColorsFromView(QObject *view);

    bool isActive() const;
    void setActive(bool active);

signals:
    void colorsChanged();

    void titleChanged(const QString& title);
    void modelChanged(const QList<QObject*>& model);
    void activeChanged(bool active);

private:
    QColor m_foreground;
    QColor m_background;
    QColor m_highlight;

    QString m_title;
    QList<QObject*> m_model;
    bool m_active;
};

Q_DECLARE_METATYPE(AssistantPopupConfig*)

class AssistantPopup : public QDeclarativeView
{
    Q_OBJECT

public:
    typedef KSharedPtr<AssistantPopup> Ptr;

    /**
     * The current main window will be used as parent widget for the popup.
     * This is to make use of the maximal space available and prevent any lines
     * in e.g. the editor to be hidden by the popup.
     */
    AssistantPopup();

    /**
     * Reset this popup for view @p view and show assistant @p assistant
     *
     * @p view The widget below which the assistant should be shown.
     */
    void reset(KTextEditor::View *view, const KDevelop::IAssistant::Ptr &assistant);

    KDevelop::IAssistant::Ptr assistant() const;

private slots:
    void updatePosition(KTextEditor::View* view, const KTextEditor::Cursor& newPos);
    void updateState();

    void executeHideAction();
    void hideAssistant();

protected:
    virtual bool eventFilter(QObject* object, QEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual bool viewportEvent(QEvent *event);

private:
    void setView(KTextEditor::View* view);
    void setAssistant(const KDevelop::IAssistant::Ptr& assistant);

    /// Give the AssistantPopup instance widget focus
    void grabFocus();
    /// Return focus back to the editor view
    void ungrabFocus();

    KDevelop::IAssistant::Ptr m_assistant;
    QPointer<KTextEditor::View> m_view;
    AssistantPopupConfig* m_config;
    bool m_shownAtBottom;
    bool m_reopening;
    QTimer* m_updateTimer;
};

#endif // KDEVPLATFORM_ASSISTANTPOPUP_H
