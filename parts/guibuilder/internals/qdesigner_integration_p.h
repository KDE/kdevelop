/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_INTEGRATION_H
#define QDESIGNER_INTEGRATION_H

#include "shared_global_p.h"

#include <QtCore/QObject>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerFormWindowManagerInterface;

class QVariant;
class QWidget;

class QT_SHARED_EXPORT QDesignerIntegration: public QObject
{
    Q_OBJECT
public:
    QDesignerIntegration(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~QDesignerIntegration();

    inline QDesignerFormEditorInterface *core() const;

signals:
    void propertyChanged(QDesignerFormWindowInterface *formWindow, const QString &name, const QVariant &value);

public slots:
    virtual void updateProperty(const QString &name, const QVariant &value);
    virtual void updateActiveFormWindow(QDesignerFormWindowInterface *formWindow);
    virtual void setupFormWindow(QDesignerFormWindowInterface *formWindow);
    virtual void updateSelection();
    virtual void updateGeometry();
    virtual void activateWidget(QWidget *widget);

protected:
    virtual QWidget *containerWindow(QWidget *widget);

private:
    void initialize();

private:
    QDesignerFormEditorInterface *m_core;
    QDesignerFormWindowManagerInterface *m_formWindowManager;
};

inline QDesignerFormEditorInterface *QDesignerIntegration::core() const
{ return m_core; }


#endif // QDESIGNER_INTEGRATION_H
