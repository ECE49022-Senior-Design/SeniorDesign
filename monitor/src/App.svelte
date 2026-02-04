<script>
  import { onMount } from "svelte";
  import { fetchRobotSnapshot } from "./lib/backend.js";
  import MainPage from "./pages/MainPage.svelte";
  import AnalyticsPage from "./pages/AnalyticsPage.svelte";

  let snapshot = {
    status: "CONNECTING",
    statusCode: "----",
    uptime: "--:--:--",
    serverUrl: "",
    lastItem: "Awaiting data",
    counts: { trash: 0, recycle: 0 },
    analytics: [
      { label: "Accuracy", value: 0 },
      { label: "Speed", value: 0 },
      { label: "Quality", value: 0 },
    ],
  };

  let page = "main";

  const syncPage = () => {
    const hash = window.location.hash.replace("#", "");
    page = hash === "analytics" ? "analytics" : "main";
  };

  onMount(async () => {
    snapshot = await fetchRobotSnapshot();
    syncPage();
    window.addEventListener("hashchange", syncPage);
    return () => window.removeEventListener("hashchange", syncPage);
  });
</script>

{#if page === "analytics"}
  <AnalyticsPage {snapshot} />
{:else}
  <MainPage {snapshot} />
{/if}
