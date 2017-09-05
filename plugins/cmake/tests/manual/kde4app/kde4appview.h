/*
 * kde4appview.h
 *
 * Copyright (C) 2007 %{AUTHOR} <%{EMAIL}>
 */
#ifndef KDE4APPVIEW_H
#define KDE4APPVIEW_H

#include <QtGui/QWidget>

#include "ui_kde4appview_base.h"

class QPainter;
class KUrl;

/**
 * This is the main view class for kde4app.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */

class kde4appView : public QWidget, public Ui::kde4appview_base
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kde4appView(QWidget *parent);

    /**
     * Destructor
     */
    virtual ~kde4appView();

private:
    Ui::kde4appview_base ui_kde4appview_base;

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

private slots:
    void switchColors();
    void settingsChanged();
};

#endif // kde4appVIEW_H
