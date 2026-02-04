export async function fetchRobotSnapshot() {
  // TODO: Replace with real backend call.
  // Example: return fetch("/api/robot").then((res) => res.json());
  return Promise.resolve({
    status: "IDLE",
    statusCode: "I2LF",
    uptime: "12:14:32",
    lastItem: "Plastic Bottle",
    counts: {
      trash: 42,
      recycle: 30,
    },
    analytics: [
      { label: "Accuracy", value: 92 },
      { label: "Speed", value: 78 },
      { label: "Quality", value: 85 },
    ],
  });
}
