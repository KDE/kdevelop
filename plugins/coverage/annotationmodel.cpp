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

#include "annotationmodel.h"
#include "coveredfile.h"
#include <KLocale>

using Veritas::AnnotationModel;
using Veritas::CoveredFile;


AnnotationModel::AnnotationModel(CoveredFile* file, QObject* parent)
    : m_notInstrumented(QBrush(Qt::BDiagPattern)),
      m_noCoverage(QBrush(Qt::red)),
      m_covered(QBrush(Qt::green))
{
    setParent(parent);
    m_file = new CoveredFile;
    m_file->setUrl(file->url());
    QMap<int, int> cc = file->callCountMap();
    QMapIterator<int, int> it(cc);
    while(it.hasNext()) {
        it.next();
        m_file->setCallCount(it.key(), it.value());
    }
}

AnnotationModel::~AnnotationModel()
{
    delete m_file;
}

/**
  * data() is used to retrieve the information needed to present the
  * annotation information from the annotation model. The provider
  * should return useful information for the line and the data role.
  *
  * \param line the line for which the data is to be retrieved
  * \param role the role to identify which kind of annotation is to be retrieved
  *
  * \returns a \ref QVariant that contains the data for the given role. The
  * following roles are supported:
  *
  * \ref Qt::DisplayRole - a short display text to be placed in the border
  * \ref Qt::TooltipRole - a tooltip information, longer text possible
  * \ref Qt::BackgroundRole - a brush to be used to paint the background on the border
  * \ref Qt::ForegroundRole - a brush to be used to paint the text on the border
  */
QVariant AnnotationModel::data(int line, Qt::ItemDataRole role) const
{
    Q_ASSERT(m_file);
    line += 1; // annotationmodel counts from zero
    int count;
    switch(role) {
    case Qt::DisplayRole: {
        if (m_file->isReachable(line)) {
            return QString::number(m_file->callCountMap()[line]);
        }
        break;
    } case Qt::BackgroundRole: {
        if (!m_file->isReachable(line)) {
            return m_notInstrumented;
        }
        count = m_file->callCountMap()[line];
        if (count == 0) return m_noCoverage;
        else return m_covered;
        break;
    } case Qt::ToolTipRole : {
        if (m_file->isReachable(line)) {
            return QVariant(i18n("Number of times visited."));
        } else {
            return QVariant(i18n("Line not reachable."));
        }
        break;
    } default: break;
    }
    return QVariant();
}

void AnnotationModel::update()
{
    emit reset();
}

#include "annotationmodel.moc"
