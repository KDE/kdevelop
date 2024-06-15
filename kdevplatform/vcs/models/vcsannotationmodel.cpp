/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsannotationmodel.h"

#include "../vcsannotation.h"
#include "../vcsrevision.h"
#include "../vcsjob.h"

#include <QDateTime>
#include <QBrush>
#include <QPen>
#include <QHash>
#include <QLocale>
#include <QUrl>
#include <QApplication>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

namespace KDevelop
{

class VcsAnnotationModelPrivate
{
public:
    explicit VcsAnnotationModelPrivate( VcsAnnotationModel* q_ ) : q(q_) {}
    KDevelop::VcsAnnotation m_annotation;
    mutable QHash<KDevelop::VcsRevision, QBrush> m_brushes;
    VcsAnnotationModel* q;
    VcsJob* job;
    QColor foreground;
    QColor background;

    const QBrush& brush(const VcsRevision& revision) const
    {
        auto brushIt = m_brushes.find(revision);
        if (brushIt == m_brushes.end()) {
            const int background_y = background.red()*0.299 + 0.587*background.green()
                                                            + 0.114*background.blue();
            // get random, but reproducible 8-bit values from last two bytes of the revision hash
            const auto revisionHash = qHash(revision);
            const int u = static_cast<int>((0xFF & revisionHash));
            const int v = static_cast<int>((0xFF00 & revisionHash) >> 8);
            const int r = qRound(qMin(255.0, qMax(0.0, background_y + 1.402*(v-128))));
            const int g = qRound(qMin(255.0, qMax(0.0, background_y - 0.344*(u-128) - 0.714*(v-128))));
            const int b = qRound(qMin(255.0, qMax(0.0, background_y + 1.772*(u-128))));
            brushIt = m_brushes.insert(revision, QBrush(QColor(r, g, b)));
        }
        return brushIt.value();
    }

    void addLines( KDevelop::VcsJob* job )
    {
        if( job == this->job )
        {
            const auto results = job->fetchResults().toList();
            for (const QVariant& v : results) {
                if( v.canConvert<KDevelop::VcsAnnotationLine>() )
                {
                    VcsAnnotationLine l = v.value<KDevelop::VcsAnnotationLine>();
                    m_annotation.insertLine( l.lineNumber(), l );
                    emit q->lineChanged( l.lineNumber() );
                }
            }
        }
    }
};

VcsAnnotationModel::VcsAnnotationModel(VcsJob *job, const QUrl& url, QObject* parent,
                                       const QColor &foreground, const QColor &background)
    : d_ptr(new VcsAnnotationModelPrivate(this))
{
    Q_D(VcsAnnotationModel);

    setParent( parent );
    d->m_annotation.setLocation( url );
    d->job = job;
    d->foreground = foreground;
    d->background = background;
    connect( d->job, &VcsJob::resultsReady,this, [this] (VcsJob* job) { Q_D(VcsAnnotationModel); d->addLines(job); } );
    ICore::self()->runController()->registerJob( d->job );
}

VcsAnnotationModel::~VcsAnnotationModel() = default;

static QString abbreviateLastName(const QString& author) {
    auto parts = author.split(QLatin1Char(' '));
    bool onlyOneFragment = parts.size() == 1 || ( parts.size() == 2 && parts.at(1).isEmpty() );
    return onlyOneFragment ? parts.first() : parts.first() + QStringLiteral(" %1.").arg(parts.last()[0]);
}

static QString annotationToolTip(const VcsAnnotationLine& aline)
{
    const bool textIsLeftToRight = (QApplication::layoutDirection() == Qt::LeftToRight);

    const QString boldStyle = QStringLiteral(";font-weight:bold");
    const QString one = QStringLiteral("1");
    const QString two = QStringLiteral("2");
    const QString line = QStringLiteral(
        "<tr>"
          "<td align=\"right\" style=\"white-space:nowrap%1\">%%2</td>"
          "<td align=\"left\" style=\"white-space:nowrap%3\">%%4</td>"
        "</tr>").arg(
            (textIsLeftToRight ? boldStyle : QString()),
            (textIsLeftToRight ? one : two),
            (textIsLeftToRight ? QString() : boldStyle),
            (textIsLeftToRight ? two : one)
        );

    const QString authorLabel = i18n("Author:").toHtmlEscaped();
    const QString dateLabel = i18n("Date:").toHtmlEscaped();
    const QString messageLabel = i18n("Commit message:").toHtmlEscaped();

    const QString author = aline.author().toHtmlEscaped();
    const QString date = QLocale().toString(aline.date()).toHtmlEscaped();
    const QString message = aline.commitMessage().toHtmlEscaped().replace(QLatin1Char('\n'), QLatin1String("<br/>"));

    return
        QLatin1String("<table>") +
        line.arg(authorLabel, author) +
        line.arg(dateLabel, date) +
        line.arg(messageLabel, message) +
        QLatin1String("</table>");
}

QVariant VcsAnnotationModel::data( int line, Qt::ItemDataRole role ) const
{
    Q_D(const VcsAnnotationModel);

    if( line < 0 || !d->m_annotation.containsLine( line ) )
    {
        return QVariant();
    }

    KDevelop::VcsAnnotationLine aline = d->m_annotation.line( line );
    if( role == Qt::ForegroundRole )
    {
        return QVariant(QPen(d->foreground));
    }
    if( role == Qt::BackgroundRole )
    {
        return QVariant(d->brush(aline.revision()));
    } else if( role == Qt::DisplayRole )
    {
        return QVariant( QStringLiteral("%1 ").arg(aline.date().date().year()) + abbreviateLastName(aline.author()) );
    } else if( role == (int) KTextEditor::AnnotationModel::GroupIdentifierRole )
    {
        return aline.revision().revisionValue();
    } else if( role == Qt::ToolTipRole )
    {
        return QVariant(annotationToolTip(aline));
    }
    return QVariant();
}

VcsRevision VcsAnnotationModel::revisionForLine( int line ) const
{
    Q_D(const VcsAnnotationModel);

    ///FIXME: update the annotation bar on edit/reload somehow
    ///BUG: https://bugs.kde.org/show_bug.cgi?id=269757
    if (!d->m_annotation.containsLine(line)) {
        return VcsRevision();
    }

    Q_ASSERT(line >= 0 && d->m_annotation.containsLine(line));
    return d->m_annotation.line( line ).revision();
}

VcsAnnotationLine VcsAnnotationModel::annotationLine(int line) const
{
    Q_D(const VcsAnnotationModel);

    if (line < 0 || !d->m_annotation.containsLine(line)) {
        return VcsAnnotationLine();
    }

    return d->m_annotation.line(line);
}

}

#include "moc_vcsannotationmodel.cpp"
