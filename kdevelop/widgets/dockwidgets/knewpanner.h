// -*- c++ -*-

#ifndef KNEWPANNER_H
#define KNEWPANNER_H

/* This file is part of the KDE libraries
    Copyright (C) 1997 Richard Moore (moorer@cs.man.ac.uk)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>

/**
 * KNewPanner is a simple widget for managing two children which
 * are seperated by a draggable divider. The user can resize both
 * children by moving the bar. You can also label the two children
 * and the labels will be adjusted as the divider is moved.
 *
 * This widget is considerably easier to use than the old one, simply
 * set the minimum and maximum sizes of the two children then call
 * activate(). Note that the widgets to be panned should be children
 * of the panner widget.
 *
 * This widget fixes a number of design
 * problems in the original KPanner class which show up particularly
 * if you try to use it under a layout manager (such as QBoxLayout).
 *
 * PLEASE NOTE: This panner is NOT source or binary compatable with
 * the old one.
 *
 * @version $Id$
 * @author Richard Moore rich@kde.org
 */
class KNewPanner : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constants used to specify the orientation.
     */
    enum Orientation { Vertical, Horizontal };

    /**
     * Constants used to choose between absolute (pixel) sizes and
     * percentages.
     */
    enum Units { Percent, Absolute };

    /**
     * Construct a KNewPanner widget.
     *
     * @param parent  The parent widget
     * @param name  The name of the panner widget.
     * @param orient  The orientation of the panner, one of the constants
     *   Vertical or Horizontal.
     * @param units  The units in which you want to specify the seperator position. This must be one of the constants Percent or Absolute.
     * @param pos  The initial seperator position in the selected units.
     */
    KNewPanner(QWidget *parent= 0, const char *name= 0,
		 Orientation orient= Vertical, Units units= Percent, int pos= 50);

    /**
     * Clean up
     */
    virtual ~KNewPanner();

  /**
   * Begin managing these two widgets. If you want to set the minimum or
   * maximum sizes of the children then you should do it before calling this
   * method.
   */
  void activate(QWidget *c0, QWidget *c1);

  /**
   * Call this method to restore the panner to it's initial state. This allows you
   * to call activate() a second time with different children.
   */
  void deactivate();

  /**
   * This gets the current position of the separator in the current
   * units.
   */
  int separatorPos();

  /**
   * This sets the position of the seperator to the specified position.
   * The position is specified in the currently selected units.
   */
  void setSeparatorPos(int pos);

  /**
   * This gets the current position of the separator in absolute 
   * (pixel) units.
   */
  int absSeparatorPos();

  /**
   * This sets the position of the seperator to the specified position.
   * The position is specified in absolute units (pixels) irrespective
   * of the the currently selected units.
   */
  void setAbsSeparatorPos(int pos, bool do_resize = true);

    /**
     * Get the current units.
     */
    Units units();

    /**
     * Set the current units.
     */
    void setUnits(Units);

protected:
    /**
     * This returns the closest valid absolute seperator position to the
     * position specified in the parameter.
     */
    int checkValue(int);

    /**
     * This method handles changes in the panner size, it will automatically resize
     * the child widgets as appropriate.
     */
    void resizeEvent(QResizeEvent *);

  /**
   * Filter the events from the divider
   */
  bool eventFilter(QObject *, QEvent *);

private:
  // The managed children
  QWidget *child0, *child1;

  // The height at which the children start
  int startHeight;

  // Have we started yet?
  bool initialised;

  // The divider widget
  QFrame *divider;

  // The position in pixel units
  int position;
  Units currentunits;
  Orientation orientation;
};

#endif





