/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPARTCONTROLLER_H
#define KDEVPLATFORM_IPARTCONTROLLER_H

#include "interfacesexport.h"

#include <KParts/PartManager>

class KPluginFactory;

namespace KTextEditor
{
class Editor;
}

namespace KDevelop {

class ICore;

class KDEVPLATFORMINTERFACES_EXPORT IPartController : public KParts::PartManager 
{
    Q_OBJECT
public:
    explicit IPartController( QWidget* parent );
    static KPluginFactory* findPartFactory( const QString& mimetype,
                                             const QString& parttype,
                                             const QString& preferredName = QString() );
    KParts::Part* createPart( const QString& mimetype, const QString& prefName = QString() );

    /**
     * Returns the global editor instance.
     */
    virtual KTextEditor::Editor* editorPart() const = 0;
};

}

#endif

