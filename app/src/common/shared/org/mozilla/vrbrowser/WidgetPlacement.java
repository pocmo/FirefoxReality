/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser;

import android.content.Context;

public class WidgetPlacement {
    static final float WORLD_DPI_RATIO = 18.0f/720.0f;

    public WidgetPlacement(Context aContext) {
        density = aContext.getResources().getDisplayMetrics().density;
    }

    public float density;
    public int width;
    public int height;
    public float worldWidth = -1.0f;
    public float anchorX = 0.5f;
    public float anchorY = 0.5f;
    public float translationX;
    public float translationY;
    public float translationZ;
    public float rotationAxisX;
    public float rotationAxisY;
    public float rotationAxisZ;
    public float rotation;
    public int parentHandle = -1;
    public float parentAnchorX = 0.5f;
    public float parentAnchorY = 0.5f;
    public boolean visible = true;
    public boolean opaque = false;
    public boolean showPointer = true;


    public static float unitFromMeters(float aMeters) {
        return aMeters / WORLD_DPI_RATIO;
    }
}
