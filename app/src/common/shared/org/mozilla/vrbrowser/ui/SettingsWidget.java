/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.Switch;
import android.widget.TextView;

import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.WidgetPlacement;

public class SettingsWidget extends UIWidget {
    private static final String LOGTAG = "VRB";

    private SettingsWidget.Delegate mDelegate;

    public SettingsWidget(Context aContext) {
        super(aContext);
        initialize(aContext);
    }

    public SettingsWidget(Context aContext, AttributeSet aAttrs) {
        super(aContext, aAttrs);
        initialize(aContext);
    }

    public SettingsWidget(Context aContext, AttributeSet aAttrs, int aDefStyle) {
        super(aContext, aAttrs, aDefStyle);
        initialize(aContext);
    }

    public void setDelegate(SettingsWidget.Delegate aDelegate) {
        mDelegate = aDelegate;
    }

    private void initialize(Context aContext) {
        inflate(aContext, R.layout.settings, this);

        ImageButton cancelButton = findViewById(R.id.settingsCancelButton);

        cancelButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mDelegate != null)
                    mDelegate.onSettingsCloseClick();
            }
        });

        SettingsButton privacyButton = findViewById(R.id.privacyButton);
        privacyButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mDelegate != null)
                    mDelegate.onSettingsPrivacyClick();
            }
        });

        Switch crashReportingSwitch  = findViewById(R.id.crash_reporting_switch);
        crashReportingSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (mDelegate != null)
                    mDelegate.onSettingsCrashReportingChange(b);
            }
        });

        Switch telemetrySwitch  = findViewById(R.id.telemetry_switch);
        telemetrySwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (mDelegate != null)
                    mDelegate.onSettingsTelemetryChange(b);
            }
        });

        TextView versionText = findViewById(R.id.versionText);
        try {
            PackageInfo pInfo = getContext().getPackageManager().getPackageInfo(getContext().getPackageName(), 0);
            versionText.setText(String.format(getResources().getString(R.string.settings_version), pInfo.versionName));

        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
    }

    @Override
    void initializeWidgetPlacement(WidgetPlacement aPlacement) {
        aPlacement.width = 435;
        aPlacement.height = 350;
        aPlacement.parentAnchorX = 0.5f;
        aPlacement.parentAnchorY = 0.5f;
        aPlacement.translationX = 0.0f;
        aPlacement.translationY = WidgetPlacement.unitFromMeters(-6.0f);
        aPlacement.translationZ = WidgetPlacement.unitFromMeters(10.0f);
        aPlacement.anchorX = 0.5f;
        aPlacement.anchorY = 0.5f;
        aPlacement.worldWidth = 6.0f;
    }

    public interface Delegate {
        void onSettingsCrashReportingChange(boolean isEnabled);
        void onSettingsTelemetryChange(boolean isEnabled);
        void onSettingsPrivacyClick();
        void onSettingsCloseClick();
    }

}
