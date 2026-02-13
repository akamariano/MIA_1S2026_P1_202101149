interface Props {
  output: string;
}

export default function TerminalOutput({ output }: Props) {
  return (
    <div>
      <label className="text-slate-400 text-sm">Salida:</label>
      <div
        className="w-full mt-2 bg-black text-emerald-400 font-mono
        p-4 rounded-xl border border-slate-700
        h-64 overflow-y-auto"
      >
        <pre>{output}</pre>
      </div>
    </div>
  );
}
