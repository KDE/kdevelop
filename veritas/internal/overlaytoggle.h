/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_OVERLAYTOGGLE_H
#define VERITAS_OVERLAYTOGGLE_H

#include <QAbstractButton>
#include <QModelIndex>
#include "../veritasexport.h"

class QTimeLine;

namespace Veritas
{
class Test;

/*! @unittest Veritas::OverlayButton */
class OverlayButton : public QAbstractButton
{
Q_OBJECT

public:
    OverlayButton(QWidget* parent);
    virtual ~OverlayButton();

    /*! Resets the selection toggle so that it is hidden and stays
     * visually invisible for at least one second after it is shown again. */
    virtual void reset();

    /*! Distance between the icon and right border of the widget it is
     *  attached to. */
    virtual int offset(Test*) = 0;

    virtual QModelIndex index();
    virtual void setIndex(const QModelIndex&);


    virtual bool shouldShow(Test*) = 0;
    virtual QSize sizeHint() const;

public slots:
    void setVisible(bool visible);

protected:
    virtual void leaveEvent(QEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    virtual void enterEvent(QEvent* event);

    bool eventFilter(QObject* obj, QEvent* event);

protected:
    QModelIndex m_index;
    QPixmap m_icon;

private slots:
    /**
     * Sets the alpha value for the fading animation and is
     * connected with m_fadingTimeLine.
     */
    void setFadingValue(int value);

private:
    void startFading();
    void stopFading();

private:
    bool m_isHovered;
    int m_fadingValue;
    QTimeLine* m_fadingTimeLine;
};

/**
 * @brief Toggle button for changing the selection of an hovered item.
 *
 * The toggle button is visually invisible until it is displayed at least
 * for one second.
 *
 * @see SelectionManager
 */
class SelectionToggle : public OverlayButton
{
    Q_OBJECT

public:
    explicit SelectionToggle(QWidget* parent);
    virtual ~SelectionToggle();

    virtual int offset(Test*) { return 17; }
    bool shouldShow(Test*);

private slots:
    void setIconOverlay(bool checked);
    void refreshIcon();

};

/*! Opens verbose test output, typically in a seperate outputview */
class VerboseToggle : public OverlayButton
{
Q_OBJECT

public:
    explicit VerboseToggle(QWidget* parent);
    virtual ~VerboseToggle();

    virtual bool shouldShow(Test*);
    virtual int offset(Test*);

private slots:
    void setIconOverlay();
};

/*! Open source file associated with this test-item */
class ToSourceToggle : public OverlayButton
{
Q_OBJECT

public:
    explicit ToSourceToggle(QWidget* parent);
    virtual ~ToSourceToggle();
 
    virtual int offset(Test*);
    bool shouldShow(Test*);

private slots:
    void setIcon();
    void refreshIcon();

};

}

#endif // VERITAS_OVERLAYTOGGLE_H
