package org.mozilla.vrbrowser.adapter

import mozilla.components.browser.state.action.EngineAction
import mozilla.components.browser.state.action.TabListAction
import mozilla.components.browser.state.state.TabSessionState
import mozilla.components.browser.state.state.createTab
import mozilla.components.browser.state.store.BrowserStore
import mozilla.components.concept.engine.EngineSession
import mozilla.components.concept.engine.EngineSessionState
import mozilla.components.concept.engine.Settings
import org.json.JSONObject
import org.mozilla.geckoview.GeckoSession
import org.mozilla.vrbrowser.browser.engine.Session

class ComponentsAdapter private constructor(
    private val store: BrowserStore = BrowserStore()
) {
    companion object {
        private val instance: ComponentsAdapter = ComponentsAdapter()

        fun get(): ComponentsAdapter = instance
    }

    fun addSession(session: Session) {
        store.dispatch(TabListAction.AddTabAction(
            tab = session.toTabSessionState()
        ))
    }

    fun removeSession(session: Session) {
        store.dispatch(TabListAction.RemoveTabAction(
            tabId = session.id
        ))
    }

    fun link(tabId: String, geckoSession: GeckoSession) {
        store.dispatch(EngineAction.LinkEngineSessionAction(
            tabId,
            GeckoEngineSession(geckoSession)
        ))
    }

    fun unlink(tabId: String) {
        store.dispatch(EngineAction.UnlinkEngineSessionAction(
            tabId
        ))
    }
}

private fun Session.toTabSessionState(): TabSessionState {
    return createTab(
        id = id,
        url = currentUri
    )
}

private class GeckoEngineSession(
    private val geckoSession: GeckoSession
): EngineSession() {
    override fun loadUrl(url: String, parent: EngineSession?, flags: LoadUrlFlags, additionalHeaders: Map<String, String>?) {
        geckoSession.loadUri(url)
    }

    override val settings: Settings = object : Settings() {}
    override fun clearFindMatches() = Unit
    override fun disableTrackingProtection() = Unit
    override fun enableTrackingProtection(policy: TrackingProtectionPolicy) = Unit
    override fun exitFullScreenMode() = Unit
    override fun findAll(text: String) = Unit
    override fun findNext(forward: Boolean) = Unit
    override fun goBack() = Unit
    override fun goForward() = Unit
    override fun loadData(data: String, mimeType: String, encoding: String) = Unit
    override fun recoverFromCrash(): Boolean = true
    override fun reload() = Unit
    override fun restoreState(state: EngineSessionState) = true
    override fun saveState(): EngineSessionState = DummyEngineSessionState()
    override fun stopLoading() = Unit
    override fun toggleDesktopMode(enable: Boolean, reload: Boolean) = Unit
}

private class DummyEngineSessionState : EngineSessionState {
    override fun toJSON(): JSONObject = JSONObject()
}
