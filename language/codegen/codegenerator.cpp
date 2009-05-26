/*
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

#include "codegenerator.h"

#include "documentchangeset.h"
#include "duchainchangeset.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <qtimer.h>

namespace KDevelop
{

struct CodeGeneratorPrivate
{
    DUChainChangeSet * duchainChanges;
    DocumentChangeSet documentChanges;
};

CodeGenerator::CodeGenerator()
    : d(new CodeGeneratorPrivate)
{
}

CodeGenerator::~CodeGenerator()
{
    delete d;
}

DocumentChangeSet* CodeGenerator::textEdits() const
{
    return &d->documentChanges;
}

void CodeGenerator::start(void)
{
    kDebug() << "Starting Code Generation Job";
    QTimer::singleShot(0, this, SLOT(executeGenerator()));
}

void CodeGenerator::generateTextEdit(AstChangeSet* astChange)
{
}

void CodeGenerator::generateTextEdit(DUChainChangeSet* astChange)
{
}

void CodeGenerator::setErrorText(const QString & errorText)
{
    KJob::setErrorText(errorText);
}

void CodeGenerator::executeGenerator(void)
{
    kDebug() << "Checking Preconditions for the codegenerator";
    
    //Shouldn't there be a method in iDocument to get a DocumentRange as well?
    DocumentRange range( ICore::self()->documentController()->activeDocument()->url().url(),
                         ICore::self()->documentController()->activeDocument()->textSelection());
    if(!checkPreconditions(0,range))
    {
        setErrorText("Error checking conditions to generate code: " + errorText());
        emitResult();
    }
    kDebug() << "Gathering user information for the codegenerator";
    if(!gatherInformation())
    {
        setErrorText("Error Ggathering user information: " + errorText());
        emitResult();
    }
    kDebug() << "Generating code";
    if(!process())
    {
        setErrorText("Error generating code: " + errorText());
        emitResult();
    }
    kDebug() << "Submitting to the user for review";
    if(!displayChanges())
        emitResult();
    
    d->documentChanges.applyAllChanges();
    emitResult();
}

bool CodeGenerator::displayChanges(void)
{
    //Create a window that shows changes to be made
    return true;
}

}
