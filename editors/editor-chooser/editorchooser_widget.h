#ifndef __EDITORCHOOSER_WIDGET_H__
#define __EDITORCHOOSER_WIDGET_H__


#include <qwidget.h>


#include "editchooser.h"


class EditorChooserWidget : public EditChooser
{
    Q_OBJECT

public:

    EditorChooserWidget(QWidget *parent=0, const char *name=0);

public slots:
    virtual void slotEditPartChanged(const QString &);

private slots:

    void load();
    void save();

    void accept();

};


#endif




