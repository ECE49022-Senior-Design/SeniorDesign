<script>
  import { onMount } from "svelte";
  import { fetchRobotSnapshot } from "./lib/backend.js";

  let snapshot = {
    status: "CONNECTING",
    statusCode: "----",
    uptime: "--:--:--",
    lastItem: "Awaiting data",
    counts: { trash: 0, recycle: 0 },
    analytics: [
      { label: "Accuracy", value: 0 },
      { label: "Speed", value: 0 },
      { label: "Quality", value: 0 },
    ],
  };

  onMount(async () => {
    snapshot = await fetchRobotSnapshot();
  });
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
      <span class="pill">Uptime</span>
      <strong>{snapshot.uptime}</strong>
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

<style>
  @import url("https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;600;700&display=swap");

  :global(body) {
    margin: 0;
    background: radial-gradient(circle at top left, #172a3a, #0b0f14 60%);
    color: #f2f5f7;
    font-family: "Space Grotesk", sans-serif;
  }

  :global(*) {
    box-sizing: border-box;
  }

  .screen {
    min-height: 100vh;
    padding: 32px;
    display: flex;
    flex-direction: column;
    gap: 28px;
  }

  .topbar {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 18px 24px;
    background: rgba(10, 18, 25, 0.8);
    border-radius: 16px;
    border: 1px solid rgba(255, 255, 255, 0.08);
    box-shadow: 0 20px 40px rgba(0, 0, 0, 0.35);
  }

  .brand {
    display: flex;
    align-items: center;
    gap: 16px;
  }

  .brand-dot {
    width: 14px;
    height: 14px;
    border-radius: 50%;
    background: #6ef7c4;
    box-shadow: 0 0 12px rgba(110, 247, 196, 0.8);
  }

  .title {
    font-size: 20px;
    font-weight: 600;
  }

  .subtitle {
    font-size: 12px;
    text-transform: uppercase;
    letter-spacing: 0.12em;
    color: rgba(242, 245, 247, 0.6);
  }

  .clock {
    display: flex;
    align-items: center;
    gap: 12px;
    font-size: 14px;
  }

  .pill {
    padding: 4px 10px;
    border-radius: 999px;
    background: rgba(110, 247, 196, 0.2);
    color: #6ef7c4;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.08em;
    font-size: 11px;
  }

  .panel-grid {
    display: grid;
    grid-template-columns: minmax(0, 1.4fr) minmax(0, 0.8fr);
    gap: 24px;
    flex: 1;
  }

  .status-panel,
  .counter-panel {
    background: rgba(15, 24, 33, 0.85);
    border-radius: 22px;
    padding: 26px;
    border: 1px solid rgba(255, 255, 255, 0.08);
    position: relative;
    overflow: hidden;
  }

  .status-panel::before {
    content: "";
    position: absolute;
    inset: -40% 40% auto auto;
    width: 260px;
    height: 260px;
    background: radial-gradient(circle, rgba(110, 247, 196, 0.3), transparent 70%);
    opacity: 0.9;
    pointer-events: none;
  }

  .panel-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 18px;
  }

  h2 {
    margin: 0;
    font-size: 22px;
  }

  .chip {
    padding: 6px 12px;
    border-radius: 12px;
    background: rgba(255, 255, 255, 0.08);
    font-weight: 600;
    letter-spacing: 0.1em;
  }

  .status-body {
    display: flex;
    flex-direction: column;
    gap: 24px;
  }

  .status-indicator {
    display: flex;
    align-items: center;
    gap: 14px;
    font-size: 18px;
  }

  .pulse {
    width: 16px;
    height: 16px;
    border-radius: 50%;
    background: #6ef7c4;
    box-shadow: 0 0 14px rgba(110, 247, 196, 0.9);
    animation: pulse 2s infinite;
  }

  .status-metadata {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 16px;
  }

  .meta {
    background: rgba(6, 12, 18, 0.55);
    padding: 16px;
    border-radius: 14px;
  }

  .meta span {
    display: block;
    font-size: 12px;
    color: rgba(242, 245, 247, 0.6);
    text-transform: uppercase;
    letter-spacing: 0.08em;
  }

  .meta strong {
    display: block;
    margin-top: 8px;
    font-size: 18px;
  }

  .flow {
    display: flex;
    flex-direction: column;
    gap: 16px;
  }

  .flow-step {
    display: flex;
    gap: 14px;
    padding: 12px 14px;
    border-radius: 14px;
    background: rgba(255, 255, 255, 0.03);
  }

  .flow-step.active {
    border: 1px solid rgba(110, 247, 196, 0.4);
    background: rgba(110, 247, 196, 0.1);
  }

  .flow-step .dot {
    width: 10px;
    height: 10px;
    margin-top: 6px;
    border-radius: 50%;
    background: #6ef7c4;
  }

  .flow-step p {
    margin: 4px 0 0;
    font-size: 12px;
    color: rgba(242, 245, 247, 0.6);
  }

  .counter-panel {
    display: flex;
    flex-direction: column;
    gap: 18px;
  }

  .counter-card {
    background: rgba(6, 12, 18, 0.55);
    border-radius: 18px;
    padding: 20px;
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .counter-card h3 {
    margin: 0 0 8px;
    font-size: 18px;
  }

  .counter {
    margin: 0;
    font-size: 42px;
    font-weight: 700;
  }

  .counter-sub {
    font-size: 12px;
    text-transform: uppercase;
    letter-spacing: 0.12em;
    color: rgba(242, 245, 247, 0.55);
  }

  .analytics-bar {
    margin-top: auto;
    background: rgba(6, 12, 18, 0.55);
    border-radius: 18px;
    padding: 18px;
  }

  .analytics-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 14px;
  }

  .analytics-header span {
    font-size: 12px;
    color: rgba(242, 245, 247, 0.6);
  }

  .analytics-track {
    display: flex;
    flex-direction: column;
    gap: 12px;
  }

  .analytics-item {
    display: grid;
    grid-template-columns: 1fr 3fr auto;
    gap: 12px;
    align-items: center;
  }

  .analytics-label {
    font-size: 12px;
    text-transform: uppercase;
    letter-spacing: 0.08em;
  }

  .analytics-meter {
    height: 8px;
    border-radius: 999px;
    background: rgba(255, 255, 255, 0.08);
    overflow: hidden;
  }

  .analytics-fill {
    height: 100%;
    border-radius: 999px;
    background: linear-gradient(90deg, #6ef7c4, #3fe0ff);
  }

  .analytics-value {
    font-size: 12px;
    color: rgba(242, 245, 247, 0.6);
  }

  @keyframes pulse {
    0% {
      transform: scale(0.9);
      opacity: 0.7;
    }
    50% {
      transform: scale(1.1);
      opacity: 1;
    }
    100% {
      transform: scale(0.9);
      opacity: 0.7;
    }
  }

  @media (max-width: 900px) {
    .screen {
      padding: 20px;
    }

    .panel-grid {
      grid-template-columns: 1fr;
    }
  }
</style>
