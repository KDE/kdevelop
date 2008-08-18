/* KDevelop xUnit plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_COVANNOTATIONMODEL_H
#define VERITAS_COVANNOTATIONMODEL_H

#include <ktexteditor/annotationinterface.h>
#include <QBrush>

namespace Veritas {

class CoveredFile;

/*! Places annotations on text-views, which show the covered lines
 * and their callcount */
class AnnotationModel : public KTextEditor::AnnotationModel
{
  Q_OBJECT
  public:
    AnnotationModel(Veritas::CoveredFile* file, QObject* parent=0);
    virtual ~AnnotationModel();
    virtual QVariant data(int line, Qt::ItemDataRole role) const;
    void update();

private:
    Veritas::CoveredFile* m_file;
    QBrush m_notInstrumented;
    QBrush m_noCoverage;
    QBrush m_covered;

};

}

#endif // VERITAS_COVANNOTATIONMODEL_H
