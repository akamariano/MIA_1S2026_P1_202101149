"use client";

import { useState, useRef } from "react";
import TerminalInput from "./TerminalInput";
import TerminalOutput from "./TerminalOutput";
import { executeCommand } from "../services/api";

export default function CommandPanel() {
  const [input, setInput] = useState("");
  const [output, setOutput] = useState(">> Sistema listo. Backend en http://localhost:8080\n");
  const [loading, setLoading] = useState(false);
  const fileRef = useRef<HTMLInputElement>(null);

  const handleExecute = async () => {
    if (!input.trim()) return;
    setLoading(true);
    setOutput(prev => prev + `\n> ${input}\n`);
    const result = await executeCommand(input);
    setOutput(prev => prev + result + "\n");
    setInput("");
    setLoading(false);
  };

  const handleFileLoad = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (ev) => {
      setInput(ev.target?.result as string ?? "");
    };
    reader.readAsText(file);
  };

  const handleFileExecute = async () => {
    if (!input.trim()) return;
    setLoading(true);
    setOutput(">> Ejecutando script...\n");

    const lines = input.split("\n");

    for (const line of lines) {
      // Línea en blanco — preservar
      if (line.trim() === "") {
        setOutput(prev => prev + "\n");
        continue;
      }

      // Comentario — mostrar tal como aparece
      if (line.trim().startsWith("#")) {
        setOutput(prev => prev + line + "\n");
        continue;
      }

      // Comando — ejecutar y mostrar salida
      setOutput(prev => prev + `> ${line}\n`);
      const result = await executeCommand(line);
      setOutput(prev => prev + result + "\n");
    }

    setLoading(false);
  };

  const handleClear = () => setOutput("");

  return (
    <div className="bg-slate-900 rounded-2xl shadow-2xl p-6 border border-slate-700">
      <div className="flex gap-4 mb-4 items-center flex-wrap">
        <input
          ref={fileRef}
          type="file"
          accept=".smia"
          onChange={handleFileLoad}
          className="text-sm text-slate-300 file:mr-4 file:py-2 file:px-4 
          file:rounded-lg file:border-0 
          file:bg-indigo-600 file:text-white
          hover:file:bg-indigo-700"
        />

        <button
          onClick={handleFileExecute}
          disabled={loading}
          className="bg-gradient-to-r from-green-500 to-emerald-600 
          px-4 py-2 rounded-lg text-white font-medium
          hover:scale-105 transition disabled:opacity-50"
        >
          {loading ? "Ejecutando..." : "Ejecutar Script"}
        </button>

        <button
          onClick={handleExecute}
          disabled={loading}
          className="bg-gradient-to-r from-indigo-500 to-purple-600 
          px-4 py-2 rounded-lg text-white font-medium
          hover:scale-105 transition disabled:opacity-50"
        >
          {loading ? "..." : "Ejecutar"}
        </button>

        <button
          onClick={handleClear}
          className="bg-slate-700 px-4 py-2 rounded-lg text-white
          hover:bg-slate-600 transition"
        >
          Limpiar
        </button>
      </div>

      <TerminalInput value={input} setValue={setInput} />
      <TerminalOutput output={output} />
    </div>
  );
}