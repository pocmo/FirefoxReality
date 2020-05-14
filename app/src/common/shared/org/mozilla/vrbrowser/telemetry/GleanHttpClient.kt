package org.mozilla.vrbrowser.telemetry

import android.content.Context
import mozilla.components.service.glean.net.ConceptFetchHttpUploader
import mozilla.telemetry.glean.net.PingUploader
import org.mozilla.vrbrowser.browser.engine.GeckoViewFetchClient

object GleanHttpClient {
    @JvmStatic
    fun create(context: Context): PingUploader {
        return ConceptFetchHttpUploader(lazy { GeckoViewFetchClient(context) })
    }
}