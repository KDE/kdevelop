/*
    SPDX-FileCopyrightText: 2016 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "macdockprogressview.h"
#include <QtGlobal>
#include <utility>

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
#import <AppKit/NSDockTile.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSCIImageRep.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <Foundation/NSTimer.h>

using namespace KDevelop;

@interface MacDockProgressViewPrivate : NSView {
    int min;
    int max;
    int value;
    NSColor *bgColour, *fgColour;
    NSTimer *indeterminateTimer;
}

+ (MacDockProgressViewPrivate *)sharedProgressView;

- (id)init;
- (void)dealloc;
- (void)removeTimer;
- (void)rangeStartsAt:(int)v1 endsAt:(int)v2;
- (void)setValue:(int)v;
- (void)updateBadge;

@end

static MacDockProgressViewPrivate *sharedProgressView = nil;

@implementation MacDockProgressViewPrivate

+ (MacDockProgressViewPrivate *)sharedProgressView
{
    if (sharedProgressView == nil) {
        sharedProgressView = [[MacDockProgressViewPrivate alloc] init];
        [sharedProgressView rangeStartsAt:0 endsAt:100];
    }
    return sharedProgressView;
}

- (id)init
{
    self = [super init];
    bgColour = fgColour = nil;
    indeterminateTimer = nil;
    min = max = value = 0;
    return self;
}

- (void)dealloc
{
    [self removeTimer];
    [bgColour release];
    [fgColour release];
    [super dealloc];
}

- (void)removeTimer
{
    if (indeterminateTimer != nil) {
        [indeterminateTimer invalidate];
        [indeterminateTimer release];
        indeterminateTimer = nil;
    }
}

- (void)rangeStartsAt:(int)v1 endsAt:(int)v2
{
    min = v1;
    max = v2;
    // (re)set the colours to the standard progressbar colour scheme
    [bgColour release];
    [fgColour release];
    bgColour = [[NSColor blackColor] retain];
    fgColour = [[NSColor lightGrayColor] retain];
    if (v1 == v2 ) {
        if (indeterminateTimer == nil) {
            indeterminateTimer = [[NSTimer timerWithTimeInterval:1
                                            target:self
                                            selector:@selector(updateBadge)
                                            userInfo:nil
                                            repeats:YES] retain];
            if (indeterminateTimer) {
                [[NSRunLoop currentRunLoop] addTimer:indeterminateTimer forMode:NSDefaultRunLoopMode];
            }
        }
    } else {
        [self removeTimer];
    }
    [self updateBadge];
}

- (void)setValue:(int)v
{
    value = v;
    [self updateBadge];
}

- (void)updateBadge
{
    [[NSApp dockTile] display];
}

- (void)drawRect:(NSRect)rect
{
    Q_UNUSED(rect)
    NSRect boundary = [self bounds];
    [[NSApp applicationIconImage] drawInRect:boundary
                                fromRect:NSZeroRect
                                operation:NSCompositeCopy
                                fraction:1.0];
    NSRect progressBoundary = boundary;
    progressBoundary.size.height *= 0.1;
    progressBoundary.size.width *= 0.8;
    progressBoundary.origin.x = (NSWidth(boundary) - NSWidth(progressBoundary)) / 2.0;
    progressBoundary.origin.y = NSHeight(boundary) * 0.1;

    double percent = 0.50;
    if (min != max) {
        double range = max - min;
        percent = (value - min) / range;
        if (percent > 1) {
            percent = 1;
        } else if (percent < 0) {
            percent = 0;
        }
    } else {
        // poor man's indefinite busy progressbar obtained by swapping
        // fg and bg colours with the bar at the 50% point.
        std::swap(fgColour, bgColour);
    }

    NSRect currentProgress = progressBoundary;
    currentProgress.size.width *= percent;
    [bgColour setFill];
    [NSBezierPath fillRect:progressBoundary];
    [fgColour setFill];
    [NSBezierPath fillRect:currentProgress];
    [bgColour setStroke];
    [NSBezierPath strokeRect:progressBoundary];
}

@end

void MacDockProgressView::setRange(int min, int max)
{
    [[MacDockProgressViewPrivate sharedProgressView] rangeStartsAt:min endsAt:max];
}

void MacDockProgressView::setProgress(int value)
{
    [[MacDockProgressViewPrivate sharedProgressView] setValue:value];
}

void MacDockProgressView::setProgressVisible(bool visible)
{
    if (visible) {
        [[NSApp dockTile] setContentView:[MacDockProgressViewPrivate sharedProgressView]];
    } else {
        [[NSApp dockTile] setContentView:nil];
    }
    [[NSApp dockTile] display];
}

#else

void MacDockProgressView::setRange(int min, int max)
{
    Q_UNUSED(min)
    Q_UNUSED(max)
}

void MacDockProgressView::setProgress(int value)
{
    Q_UNUSED(value)
}

void MacDockProgressView::setProgressVisible(bool visible)
{
    Q_UNUSED(visible)
}

#endif
