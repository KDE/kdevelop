/*************************************************************************************
 *  Copyright (C) 2013 by Milian Wolff <mail@milianw.de>                             *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "qmljshighlighting.h"

#include <language/duchain/declaration.h>

using namespace KDevelop;

class HighlightingInstance : public KDevelop::CodeHighlightingInstance
{
public:
    HighlightingInstance(const CodeHighlighting* highlighting)
    : CodeHighlightingInstance(highlighting)
    {}

    virtual bool useRainbowColor(Declaration* dec) const
    {
        // JS has a function-based prototype OO system, so for now rainbow-color
        // everything that is not a function declaration.
        // In the future we will have to investigate how to handle properties
        // of a function/object
        return dec->kind() == Declaration::Instance &&
               dec->context()->type() != DUContext::Class &&
               dec->context()->type() != DUContext::Enum;
    }
};

QmlJsHighlighting::QmlJsHighlighting(QObject* parent)
: CodeHighlighting(parent)
{

}

CodeHighlightingInstance* QmlJsHighlighting::createInstance() const
{
    return new HighlightingInstance(this);
}
