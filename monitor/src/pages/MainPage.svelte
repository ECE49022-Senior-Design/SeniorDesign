<script>
  export let snapshot;
</script>

<main class="screen">
  <header class="topbar">
    <div class="brand">
      <div class="brand-dot"></div>
      <div>
        <div class="title">Main</div>
        <div class="subtitle">Trash Sorting Monitor</div>
      </div>
    </div>
    <div class="clock">
      <div class="server">
        <span class="pill">Server</span>
        <strong>{snapshot.serverUrl || "Not Set"}</strong>
      </div>
      <span class="pill">Uptime</span>
      <strong>{snapshot.uptime}</strong>
      <a class="nav-button" href="#analytics">Analytics</a>
    </div>
  </header>

  <section class="panel-grid">
    <article class="status-panel">
      <div class="panel-header">
        <h2>Status</h2>
        <span class="chip">{snapshot.statusCode}</span>
      </div>

      <div class="status-body">
        <div class="status-indicator">
          <div class="pulse"></div>
          <span>{snapshot.status}</span>
        </div>
        <div class="status-metadata">
          <div class="meta">
            <span>Last Item</span>
            <strong>{snapshot.lastItem}</strong>
          </div>
          <div class="meta">
            <span>Mode</span>
            <strong>Auto Sort</strong>
          </div>
        </div>

        <div class="flow">
          <div class="flow-step active">
            <span class="dot"></span>
            <div>
              <strong>Checking</strong>
              <p>Scanning incoming item</p>
            </div>
          </div>
          <div class="flow-step">
            <span class="dot"></span>
            <div>
              <strong>Sorting</strong>
              <p>Assigning to bin</p>
            </div>
          </div>
          <div class="flow-step">
            <span class="dot"></span>
            <div>
              <strong>Complete</strong>
              <p>Logged and cleared</p>
            </div>
          </div>
        </div>
      </div>
    </article>

    <aside class="counter-panel">
      <div class="counter-card">
        <div>
          <h3>Trash</h3>
          <p class="counter">{snapshot.counts.trash}</p>
        </div>
        <div class="counter-sub">This shift</div>
      </div>

      <div class="counter-card">
        <div>
          <h3>Recycle</h3>
          <p class="counter">{snapshot.counts.recycle}</p>
        </div>
        <div class="counter-sub">This shift</div>
      </div>

      <div class="analytics-bar">
        <div class="analytics-header">
          <h4>Detailed Analytics</h4>
          <span>Live sample</span>
        </div>
        <div class="analytics-track">
          {#each snapshot.analytics as item}
            <div class="analytics-item">
              <div class="analytics-label">{item.label}</div>
              <div class="analytics-meter">
                <div class="analytics-fill" style={`width: ${item.value}%`}></div>
              </div>
              <div class="analytics-value">{item.value}%</div>
            </div>
          {/each}
        </div>
      </div>
    </aside>
  </section>
</main>
