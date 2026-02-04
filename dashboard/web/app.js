const el = (id) => document.getElementById(id);

function fmtMs(ms) {
  if (!ms) return "—";
  const d = new Date(ms);
  return d.toLocaleString();
}

function render(state) {
  const v = state.vision?.latest;
  el("label").textContent = v?.label ?? "—";
  el("recyclable").textContent = (v == null) ? "—" : (v.recyclable ? "Yes" : "No");
  el("confidence").textContent = (v?.confidence != null) ? `${Math.round(v.confidence * 100)}%` : "—";
  if (v?.location) {
    el("location").textContent = `x=${v.location.x.toFixed(2)}, y=${v.location.y.toFixed(2)}`;
  } else {
    el("location").textContent = "—";
  }

  el("total").textContent = state.counts?.total ?? 0;
  el("countRec").textContent = state.counts?.recyclable ?? 0;
  el("countTrash").textContent = state.counts?.trash ?? 0;
  el("errors").textContent = state.counts?.errors ?? 0;

  el("vision").textContent = state.vision?.online ? "online" : "offline";
  el("arm").textContent = state.arm?.status ?? "unknown";
  el("lastUpdate").textContent = fmtMs(state.last_update_ms);
}

function connectWS() {
  const proto = (location.protocol === "https:") ? "wss" : "ws";
  const ws = new WebSocket(`${proto}://${location.host}/ws`);

  ws.onopen = () => el("conn").textContent = "Live";
  ws.onclose = () => el("conn").textContent = "Disconnected";
  ws.onerror = () => el("conn").textContent = "Error";

  ws.onmessage = (evt) => {
    try {
      const msg = JSON.parse(evt.data);
      if (msg.state) render(msg.state);
    } catch (e) {
      console.error("Bad WS message", e);
    }
  };

  return ws;
}

connectWS();
