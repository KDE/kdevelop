/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEV_CREATECLASS_H
#define KDEV_CREATECLASS_H

/*
 * These classes were once defined in this file, but have since been moved to their own files. 
 * They could be just forward-declared here, but they are included to keep source compatibility. 
 */
#include "classgenerator.h"
#include "classidentifierpage.h"
#include "licensepage.h"
#include "outputpage.h"

#include <KAssistantDialog>
#include <KUrl>

#include "../languageexport.h"

namespace KDevelop {

class OverridesPage;

/**
 * Provides an assistant for creating a new class using a ClassGenerator.
 */
class KDEVPLATFORMLANGUAGE_EXPORT CreateClassAssistant : public KAssistantDialog
{
    Q_OBJECT

public:
    CreateClassAssistant(QWidget* parent, const KUrl& baseUrl = KUrl());
    virtual ~CreateClassAssistant();
    /**
     * Creates the generic parts of the new class assistant.
     */
    virtual void setup();

    /**
     * \return The generator that this assistant will use
     */
    ClassGenerator* generator() const;

    void setGenerator(ClassGenerator* generator);

    virtual ClassIdentifierPage* newIdentifierPage();

    virtual OverridesPage* newOverridesPage();

    /**
     * @return The base url from which the assistant was called.
     **/
    KUrl baseUrl() const;


public Q_SLOTS:
    /**
     * Called when the next button is clicked.
     */
    virtual void next();
    /**
     * Called when the assistant completes.
     */
    virtual void accept();

private Q_SLOTS:
    void checkClassIdentifierPage(bool valid);
    void checkOutputPage(bool valid);

private:
    bool validateClassIdentifierPage();
    bool validateOverridesPage();
    bool validateLicensePage();
    bool validateOutputPage();

    friend class OutputPage;
    class CreateClassAssistantPrivate* const d;
};

}

#endif // KDEV_CREATECLASS_H
