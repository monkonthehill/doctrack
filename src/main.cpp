#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

using namespace std;
namespace fs = std::filesystem;

void read_write_file(const fs::path &filePath, fstream &wrt) {
  ifstream read(filePath);
  if (!read)
    return;

  regex reg(R"((//|/\*)\s*(TODO|NOTE|BUG|FIXME|CODENOTE)\s*:?\s*(.*))",
            regex::icase);

  smatch match;
  string line;
  int line_num = 0;

  bool fileHeaderPrinted = false;
  bool inBlock = false;
  int braceBalance = 0;
  bool blockStarted = false;

  while (getline(read, line)) {
    line_num++;

    if (regex_search(line, match, reg)) {
      if (!fileHeaderPrinted) {
        wrt << "\n# FILE: " << filePath.filename().string() << "\n---\n";
        fileHeaderPrinted = true;
      }

      string tag = match[2];
      string message = match[3];

      for (auto &c : tag)
        c = toupper(c);

      wrt << "\n### Line " << line_num << " [" << tag << "] → " << message
          << "\n";

      if (tag == "BUG" || tag == "FIXME" || tag == "CODENOTE") {
        wrt << "```cpp\n";
        inBlock = true;
        blockStarted = false;
        braceBalance = 0;

        for (char c : line) {
          if (c == '{')
            braceBalance++;
          if (c == '}')
            braceBalance--;
        }

        wrt << line << "\n";

        if (line.find('{') != string::npos) {
          blockStarted = true;
        }
      }
      continue;
    }

    if (inBlock) {
      wrt << line << "\n";

      for (char c : line) {
        if (c == '{') {
          braceBalance++;
          blockStarted = true;
        }
        if (c == '}')
          braceBalance--;
      }

      if (blockStarted && braceBalance == 0) {
        if (line.find('}') != string::npos || braceBalance == 0) {
          wrt << "```\n---\n";
          inBlock = false;
          blockStarted = false;
        }
      }
    }
  }
}
void process_directories(const fs::path &search_dir, fstream &wrt) {
  try {
    for (auto it = fs::recursive_directory_iterator(search_dir);
         it != fs::recursive_directory_iterator(); ++it) {

      const auto &entry = *it;
      string name = entry.path().filename().string();

      if (entry.is_directory()) {
        if (name == ".git" || name == "node_modules" || name == "build" ||
            name == "bin" || name == "obj" || name == ".vscode" ||
            name == ".idea" || name == "dist" || name == "out") {
          it.disable_recursion_pending();
        }
        continue;
      }

      if (entry.is_regular_file()) {
        string ext = entry.path().extension().string();

        if (ext == ".cpp" || ext == ".h" || ext == ".js" || ext == ".py") {
          if (entry.path().filename() != "doc.md") {
            read_write_file(entry.path(), wrt);
          }
        }
      }
    }
  } catch (const fs::filesystem_error &e) {
    cerr << "Error: " << e.what() << "\n";
  }
}

int main(int argc, char *argv[]) {
  fs::path target_dir = ".";

  if (argc > 1) {
    target_dir = argv[1];
  }

  if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
    cerr << "❌ Invalid directory: " << target_dir << "\n";
    return 1;
  }

  fs::path docs_dir = "docs";
  if (!fs::exists(docs_dir)) {
    fs::create_directory(docs_dir);
  }

  fs::path md_path = docs_dir / "doc.md";
  fs::path html_path = docs_dir / "report.html";

  fstream wrt(md_path, ios::out);
  if (!wrt) {
    cerr << "Error: Could not create output file\n";
    return 1;
  }

  cout << "🔍 Scanning: " << target_dir << "\n";

  process_directories(target_dir, wrt);

  wrt.close();

  string command =
      "pandoc " + md_path.string() + " -s -o " + html_path.string();
  system(command.c_str());

  cout << "✅ Scan complete.\n";
  cout << "📄 Markdown: " << md_path << "\n";
  cout << "🌐 HTML: " << html_path << "\n";

  return 0;
}
