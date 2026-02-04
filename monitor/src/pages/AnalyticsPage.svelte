<script>
  export let snapshot;
</script>

<main class="screen">
  <header class="topbar">
    <div class="brand">
      <div class="brand-dot"></div>
      <div>
        <div class="title">Analytics</div>
        <div class="subtitle">Detailed Performance</div>
      </div>
    </div>
    <div class="clock">
      <div class="server">
        <span class="pill">Server</span>
        <strong>{snapshot.serverUrl || "Not Set"}</strong>
      </div>
      <span class="pill">Uptime</span>
      <strong>{snapshot.uptime}</strong>
      <a class="nav-button" href="#main">Back</a>
    </div>
  </header>

  <section class="panel-grid single">
    <article class="status-panel">
      <div class="panel-header">
        <h2>Live Analytics</h2>
        <span class="chip">{snapshot.statusCode}</span>
      </div>

      <div class="analytics-page">
        <div class="analytics-summary">
          <div class="summary-card">
            <span>Total Trash</span>
            <strong>{snapshot.counts.trash}</strong>
          </div>
          <div class="summary-card">
            <span>Total Recycle</span>
            <strong>{snapshot.counts.recycle}</strong>
          </div>
          <div class="summary-card">
            <span>Last Item</span>
            <strong>{snapshot.lastItem}</strong>
          </div>
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
    </article>
  </section>
</main>
