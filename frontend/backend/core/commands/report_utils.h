#ifndef REPORT_UTILS_H
#define REPORT_UTILS_H

#include <string>
#include <cstdlib>
#include <filesystem>
#include <iostream>

// Determina formato según extensión del path
inline std::string getFormat(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    if (ext == ".jpg" || ext == ".jpeg") return "jpg";
    if (ext == ".png")  return "png";
    if (ext == ".pdf")  return "pdf";
    if (ext == ".svg")  return "svg";
    return "jpg"; // default
}

// Renderiza un .dot a imagen con graphviz
inline void renderDot(const std::string& dotContent,
                      const std::string& outPath) {
    std::string fmt    = getFormat(outPath);
    std::string tmpDot = "/tmp/extreamfs_report.dot";

    // Escribir .dot temporal
    FILE* f = fopen(tmpDot.c_str(), "w");
    if (!f) { std::cout << "ERROR: No se pudo crear archivo .dot\n"; return; }
    fprintf(f, "%s", dotContent.c_str());
    fclose(f);

    // Ejecutar graphviz
    std::string cmd = "dot -T" + fmt + " " + tmpDot + " -o " + outPath;
    int result = system(cmd.c_str());
    if (result == 0)
        std::cout << "OK: Reporte generado en " << outPath << "\n";
    else
        std::cout << "ERROR: Falló graphviz al generar reporte\n";
}

// Para reportes .txt (bm_inode, bm_block, file)
inline void writeTxt(const std::string& content,
                     const std::string& outPath) {
    FILE* f = fopen(outPath.c_str(), "w");
    if (!f) { std::cout << "ERROR: No se pudo crear " << outPath << "\n"; return; }
    fprintf(f, "%s", content.c_str());
    fclose(f);
    std::cout << "OK: Reporte generado en " << outPath << "\n";
}

#endif