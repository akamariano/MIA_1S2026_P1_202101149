interface Props {
  value: string;
  setValue: (val: string) => void;
}

export default function TerminalInput({ value, setValue }: Props) {
  return (
    <div className="mb-6">
      <label className="text-slate-400 text-sm">Entrada:</label>
      <textarea
        value={value}
        onChange={(e) => setValue(e.target.value)}
        rows={8}
        className="w-full mt-2 bg-black text-green-400 font-mono 
        p-4 rounded-xl border border-slate-700
        focus:outline-none focus:ring-2 focus:ring-indigo-500"
        placeholder="Escribe tus comandos aquí..."
      />
    </div>
  );
}
