/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KDEVPARTCONTROLLER_H__
#define __KDEVPARTCONTROLLER_H__

#include <interfaces/ipartcontroller.h>

#include <QUrl>

#include <KSharedConfig>

#include "core.h"

class QWidget;

namespace KParts
{
class Part;
class ReadOnlyPart;
class ReadWritePart;
}

namespace KTextEditor
{
class Document;
class Editor;
class View;
}

class QMimeType;

Q_DECLARE_METATYPE(KSharedConfigPtr)

namespace KDevelop
{
class PartControllerPrivate;

class KDEVPLATFORMSHELL_EXPORT PartController : public IPartController
{
    friend class Core;
    friend class CorePrivate;
    Q_OBJECT

public:
    PartController(Core *core, QWidget *toplevel);
    ~PartController() override;

    bool showTextEditorStatusBar() const;

    KTextEditor::Document* createTextPart();
    KTextEditor::Editor* editorPart() const override;

    bool canCreatePart( const QUrl &url );

    using IPartController::createPart;

    KParts::Part* createPart( const QUrl &url, const QString& prefName = QString() );

    bool isTextType(const QMimeType& mimeType);

public Q_SLOTS:
    void setShowTextEditorStatusBar(bool show);

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );
    virtual void initialize();
    virtual void cleanup();

private:
    void setupActions();

private:
    const QScopedPointer<class PartControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PartController)
};

}
#endif

