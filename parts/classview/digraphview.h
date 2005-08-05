/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIGRAPHVIEW_H_
#define _DIGRAPHVIEW_H_

#include <qptrlist.h>
#include <qscrollview.h>
#include <qstringlist.h>

class DigraphNode;
class DigraphEdge;


class DigraphView : public QScrollView
{
    Q_OBJECT
    
public:
    DigraphView(QWidget *parent, const char *name);
    ~DigraphView();

    void addEdge(const QString &name1, const QString &name2);
    void process();
    void clear();
    void setSelected(const QString &name);
    void ensureVisible(const QString &name);
    
signals:
    void selected(const QString &name);

protected:
    virtual void drawContents(QPainter* p, int clipx, int clipy, int clipw, int cliph);
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual QSize sizeHint() const;

private:
    int toXPixel(double x);
    int toYPixel(double x);

    void setRenderedExtent(double w, double h);
    void addRenderedNode(const QString &name,
                         double x, double y, double w, double h);
    void addRenderedEdge(const QString &name1, const QString &name2,
                         QMemArray<double> coords);
    static QStringList splitLine(QString str);
    void parseDotResults(const QStringList &list);
    
    double xscale, yscale;
    int width, height;
    QStringList inputs;
    QPtrList<DigraphNode> nodes;
    QPtrList<DigraphEdge> edges;
    DigraphNode *selNode;
};

#endif
