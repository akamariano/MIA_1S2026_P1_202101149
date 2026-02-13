import CommandPanel from "@/components/CommandPanel";

export default function Home() {
  return (
    <main className="min-h-screen bg-gradient-to-br from-slate-900 to-slate-800 p-8">
      <div className="max-w-6xl mx-auto">
        <h1 className="text-3xl font-bold text-white mb-6">
          ExtreamFS Console
        </h1>
        <CommandPanel />
      </div>
    </main>
  );
}
