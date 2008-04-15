/*****************************************************************************
 * Copyright (c) 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/
#ifndef QMAKEEDITORINTEGRATOR_H
#define QMAKEEDITORINTEGRATOR_H

#include <editor/editorintegrator.h>
#include "qmakeduchainexport.h"

namespace QMake
{

class AST;
}

class KDEVQMAKEDUCHAIN_EXPORT QMakeEditorIntegrator : public KDevelop::EditorIntegrator
{

public:
    QMakeEditorIntegrator();

    KTextEditor::Cursor findPosition( QMake::AST* node, Edge edge = BackEdge ) const;

    using KDevelop::EditorIntegrator::createRange;
    KTextEditor::Range findRange( QMake::AST* node, RangeEdge = OuterEdge );
    KTextEditor::Range findRange( QMake::AST* from, QMake::AST* to );

private:
};

#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
