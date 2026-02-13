"use client";

import { useState } from "react";
import TerminalInput from "./TerminalInput";
import TerminalOutput from "./TerminalOutput";

export default function CommandPanel() {
  const [input, setInput] = useState("");
  const [output, setOutput] = useState(">> Sistema listo...\n");

  const handleExecute = () => {
    setOutput(prev => prev + `\n> ${input}\nComando ejecutado correctamente.\n`);
    setInput("");
  };

  const handleClear = () => {
    setOutput("");
  };

  return (
    <div className="bg-slate-900 rounded-2xl shadow-2xl p-6 border border-slate-700">
      
      {/* Toolbar */}
      <div className="flex gap-4 mb-4 items-center">
        <input
          type="file"
          className="text-sm text-slate-300 file:mr-4 file:py-2 file:px-4 
          file:rounded-lg file:border-0 
          file:bg-indigo-600 file:text-white
          hover:file:bg-indigo-700"
        />

        <button
          onClick={handleExecute}
          className="bg-gradient-to-r from-indigo-500 to-purple-600 
          px-4 py-2 rounded-lg text-white font-medium
          hover:scale-105 transition"
        >
          Ejecutar
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
