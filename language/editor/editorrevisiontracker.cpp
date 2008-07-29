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

#include <KTextEditor/SmartInterface>

#include "interfaces/icore.h"
#include "interfaces/idocumentcontroller.h"

#include "indexedstring.h"
#include "editorintegrator.h"

namespace KDevelop {

class EditorRevisionTrackerPrivate {
public:
  QHash<KUrl, int> documents;
};

EditorRevisionTracker::EditorRevisionTracker()
  : d(new EditorRevisionTrackerPrivate)
{
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

  EditorIntegrator editor;
  editor.setCurrentUrl( IndexedString(url.pathOrUrl()) );
  if (LockedSmartInterface smart = editor.smart())
    d->documents.insert(url, smart->currentRevision());
  else
    d->documents.insert(url, -1);
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

  int revision = d->documents[url];
  if (revision != -1) {
    EditorIntegrator editor;
    editor.setCurrentUrl( IndexedString(url.pathOrUrl()) );
    if (LockedSmartInterface smart = editor.smart())
      smart->releaseRevision(revision);
  }

  d->documents.remove(url);
}

void EditorRevisionTracker::removeUrls(const KUrl::List& list)
{
  foreach (const KUrl& url, list)
    removeUrl(url);
}

void EditorRevisionTracker::openUrl(const KUrl& url, const KTextEditor::Cursor& cursor)
{
  if (IDocument* doc = ICore::self()->documentController()->documentForUrl( url )) {
    EditorIntegrator editor;
    editor.setCurrentUrl( IndexedString(url.pathOrUrl()) );
    if (LockedSmartInterface smart = editor.smart()) {
      KTextEditor::Range translated = editor.translate( smart, SimpleRange(KTextEditor::Range(cursor, 0)) ).textRange();
      ICore::self()->documentController()->activateDocument( doc, translated );
      return;
    }
  }

  ICore::self()->documentController()->openDocument( url, cursor );
}

}
