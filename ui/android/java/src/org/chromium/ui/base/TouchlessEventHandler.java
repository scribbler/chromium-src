// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.ui.base;

/**
 * org.chromium.ui.base.TouchlessEventHandler
 */
public class TouchlessEventHandler {
    private static final String EVENT_HANDLER_INTERNAL =
            "org.chromium.ui.base.TouchlessEventHandlerInternal";
    private static TouchlessEventHandler sInstance;

    static {
        try {
            sInstance = (TouchlessEventHandler) Class.forName(EVENT_HANDLER_INTERNAL).newInstance();
        } catch (ClassNotFoundException | InstantiationException | IllegalAccessException
                | IllegalArgumentException e) {
            sInstance = null;
        }
    }

    public static boolean hasTouchlessEventHandler() {
        return sInstance != null;
    }

    public static boolean onUnconsumedKeyboardEventAck(int nativeCode) {
        // No null check is needed here because it called after hasTouchlessEventHandler in native.
        assert sInstance != null;
        return sInstance.onUnconsumedKeyboardEventAckInternal(nativeCode);
    }

    public static void addCursorVisibilityObserver(CursorVisibilityObserver observer) {
        if (sInstance != null) {
            sInstance.addCursorVisibilityObserverInternal(observer);
        }
    }

    public static void removeCursorVisibilityObserver(CursorVisibilityObserver observer) {
        if (sInstance != null) {
            sInstance.removeCursorVisibilityObserverInternal(observer);
        }
    }

    public static void onDidFinishNavigation() {
        if (sInstance != null) {
            sInstance.onDidFinishNavigationInternal();
        }
    }

    public static void onActivityHidden() {
        if (sInstance != null) {
            sInstance.onActivityHiddenInternal();
        }
    }

    protected boolean onUnconsumedKeyboardEventAckInternal(int nativeCode) {
        return false;
    }

    protected void addCursorVisibilityObserverInternal(CursorVisibilityObserver observer) {}

    protected void removeCursorVisibilityObserverInternal(CursorVisibilityObserver observer) {}

    protected void onDidFinishNavigationInternal() {}

    protected void onActivityHiddenInternal() {}
}