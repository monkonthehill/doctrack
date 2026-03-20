#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

using namespace std;
namespace fs = std::filesystem;

// ---------------- FILE PROCESSING ----------------
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
  int braceCount = 0;

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

      // Start block immediately
      if (tag == "BUG" || tag == "FIXME" || tag == "CODENOTE") {
        wrt << "```cpp\n";
        wrt << line << "\n";

        inBlock = true;
        braceCount = 0;

        for (char c : line) {
          if (c == '{')
            braceCount++;
          if (c == '}')
            braceCount--;
        }

        continue;
      }

      continue;
    }

    if (inBlock) {
      wrt << line << "\n";

      for (char c : line) {
        if (c == '{')
          braceCount++;
        if (c == '}')
          braceCount--;
      }

      if (braceCount <= 0 && line.find('}') != string::npos) {
        wrt << "```\n---\n";
        inBlock = false;
      }
    }
  }
}

// ---------------- DIRECTORY PROCESSING ----------------
void process_directories(const fs::path &search_dir, fstream &wrt) {
  try {
    for (auto it = fs::recursive_directory_iterator(search_dir);
         it != fs::recursive_directory_iterator(); ++it) {

      const auto &entry = *it;
      string name = entry.path().filename().string();

      // Skip unwanted directories
      if (entry.is_directory()) {
        if (name == ".git" || name == "node_modules" || name == "build" ||
            name == "bin" || name == "obj" || name == ".vscode" ||
            name == ".idea" || name == "dist" || name == "out") {
          it.disable_recursion_pending();
        }
        continue;
      }

      // Process files
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

// ---------------- MAIN ----------------
int main(int argc, char *argv[]) {
  fs::path target_dir = "."; // default

  // If user provides folder → use it
  if (argc > 1) {
    target_dir = argv[1];
  }

  // Validate directory
  if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
    cerr << "❌ Invalid directory: " << target_dir << "\n";
    return 1;
  }

  fstream wrt("doc.md", ios::out);
  if (!wrt) {
    cerr << "Error: Could not create output file\n";
    return 1;
  }

  cout << "🔍 Scanning: " << target_dir << "\n";

  process_directories(target_dir, wrt);

  wrt.close();

  system("pandoc doc.md -s -o report.html");

  cout << "✅ Scan complete. Output: doc.md + report.html\n";

  return 0;
}
