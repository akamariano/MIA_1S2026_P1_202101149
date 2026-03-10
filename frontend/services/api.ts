const API_URL = "http://localhost:8080";

export async function executeCommand(command: string): Promise<string> {
  try {
    const res = await fetch(`${API_URL}/command`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ command }),
    });
    const data = await res.json();
    return data.output ?? data.error ?? "Sin respuesta";
  } catch (err) {
    return "ERROR: No se pudo conectar al backend";
  }
}

export async function getReport(path: string): Promise<string> {
  return `${API_URL}/report?path=${encodeURIComponent(path)}`;
}

export async function checkStatus(): Promise<boolean> {
  try {
    const res = await fetch(`${API_URL}/status`);
    const data = await res.json();
    return data.status === "ok";
  } catch {
    return false;
  }
}