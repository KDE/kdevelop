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

#include "editorrevisiontracker.h"

#include <KTextEditor/Document>
#include <KTextEditor/SmartInterface>

#include "interfaces/icore.h"
#include "interfaces/idocumentcontroller.h"

#include "../duchain/indexedstring.h"
#include "editorintegrator.h"
#include "editorintegratorstatic.h"

namespace KDevelop {

struct TrackedDocument
{
  TrackedDocument(int rev = -1, KTextEditor::Document* doc = 0) : revision(rev), document(doc) {}
  int revision;
  KTextEditor::Document* document;
};

class EditorRevisionTrackerPrivate {
public:
  QHash<KUrl, TrackedDocument> documents;
};

EditorRevisionTracker::EditorRevisionTracker(QObject* parent)
  : QObject(parent)
  , d(new EditorRevisionTrackerPrivate)
{
  connect(EditorIntegrator::notifier(), SIGNAL(documentLoaded(KTextEditor::Document*)), this, SLOT(documentLoaded(KTextEditor::Document*)));
  connect(EditorIntegrator::notifier(), SIGNAL(documentAboutToBeDeleted(KTextEditor::Document*)), this, SLOT(documentClosed(KTextEditor::Document*)));
  connect(ICore::self()->documentController(), SIGNAL(documentUrlChanged(KDevelop::IDocument*)), this, SLOT(documentUrlChanged(KDevelop::IDocument*)));
}

EditorRevisionTracker::~EditorRevisionTracker()
{
  while (!d->documents.isEmpty())
    removeUrl(d->documents.constBegin().key());

  delete d;
}

void EditorRevisionTracker::addUrl(const KUrl& url)
{
  if (d->documents.contains(url))
    return;

  if (LockedSmartInterface smart = EditorIntegrator::smart(url)) {
    d->documents.insert(url, TrackedDocument(smart->currentRevision(), smart.currentDocument()));
    kDebug(0) << "Document tracked to revision" << d->documents[url].revision;
  } else {
    d->documents.insert(url, TrackedDocument());
    kDebug(0) << "Document tracked to invalid revision (smart lock null)";
  }
}

void EditorRevisionTracker::addUrls(const KUrl::List& list)
{
  foreach (const KUrl& url, list)
    addUrl(url);
}

void EditorRevisionTracker::removeUrl(const KUrl& url)
{
  if (!d->documents.contains(url))
    return;

  const TrackedDocument& track = d->documents[url];
  if (track.revision != -1)
    if (LockedSmartInterface smart = EditorIntegrator::smart(url))
      if (smart.currentDocument() == track.document)
        smart->releaseRevision(track.revision);

  d->documents.remove(url);
}

void EditorRevisionTracker::removeUrls(const KUrl::List& list)
{
  foreach (const KUrl& url, list)
    removeUrl(url);
}

KTextEditor::Cursor EditorRevisionTracker::translateCursor(const KUrl& url, const KTextEditor::Cursor& cursor) const
{
  if (!d->documents.contains(url))
    return cursor;

  const TrackedDocument& track = d->documents[url];
  if (track.revision == -1)
    return cursor;

  if (LockedSmartInterface smart = EditorIntegrator::smart(url)) {
    if (track.document != smart.currentDocument())
      return cursor;

    smart->useRevision( track.revision );
    return smart->translateFromRevision(KTextEditor::Range(cursor, 0)).start();
  }

  return cursor;
}

KTextEditor::Range EditorRevisionTracker::translateRange(const KUrl& url, const KTextEditor::Range& range) const
{
  if (!d->documents.contains(url)) {
    kDebug(0) << "Document not tracked." << url;
    return range;
  }

  const TrackedDocument& track = d->documents[url];
  if (track.revision == -1) {
    kDebug(0) << "Document tracked to invalid revision" << url;
    return range;
  }

  kDebug(0) << "Document tracked to revision" << track.revision;

  if (LockedSmartInterface smart = EditorIntegrator::smart(url)) {
    if (track.document != smart.currentDocument())
      return range;

    smart->useRevision( track.revision );
    return smart->translateFromRevision( range );
  } else {
    kDebug(0) << "Unable to acquire smart lock";
  }

  return range;
}

void EditorRevisionTracker::openUrl(const KUrl& url, const KTextEditor::Cursor& cursor) const
{
  if (d->documents.contains(url)) {
    const TrackedDocument& track = d->documents[url];
    if (track.revision != -1) {
      if (IDocument* doc = ICore::self()->documentController()->documentForUrl( url )) {
        if (LockedSmartInterface smart = EditorIntegrator::smart(url)) {
          if (track.document == smart.currentDocument()) {
            smart->useRevision( track.revision );
            ICore::self()->documentController()->activateDocument( doc, smart->translateFromRevision( KTextEditor::Range(cursor, 0) ) );
            return;
          }
        }
      }
    }
  }

  ICore::self()->documentController()->openDocument( url, cursor );
}

KUrl::List EditorRevisionTracker::urls() const
{
  return d->documents.keys();
}

void EditorRevisionTracker::documentLoaded(KTextEditor::Document* document)
{
  if (d->documents.contains(document->url())) {
    if (LockedSmartInterface smart = EditorIntegrator::smart(document->url())) {
      TrackedDocument& track = d->documents[document->url()];
      track.revision = smart->currentRevision();
      track.document = smart.currentDocument();
    }
  }
}

void EditorRevisionTracker::documentClosed(KTextEditor::Document* document)
{
  if (d->documents.contains(document->url())) {
    TrackedDocument& track = d->documents[document->url()];
    track.revision = -1;
    track.document = 0;
  }
}

void EditorRevisionTracker::documentUrlChanged(KDevelop::IDocument* document)
{
  KTextEditor::Document* textDocument = document->textDocument();
  if (!textDocument)
    return;

  QMutableHashIterator<KUrl, TrackedDocument> it = d->documents;
  while (it.hasNext()) {
    if (it.next().value().document == document->textDocument()) {
      TrackedDocument track = it.value();
      it.remove();
      d->documents.insert(document->url(), track);
      break;
    }
  }
}

}

#include "editorrevisiontracker.moc"
