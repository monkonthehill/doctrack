// DocTrack - Real-time code documentation extractor
// Author: @monkonthehill

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

// NOTE: surbhi gautam
using namespace std;
namespace fs = std::filesystem;

// Constants
const unordered_set<string> SKIP_DIRS = {".git",
                                         "node_modules",
                                         "build",
                                         "bin",
                                         "obj",
                                         ".vscode",
                                         ".idea",
                                         "dist",
                                         "out",
                                         "docs",
                                         "__pycache__",
                                         ".pytest_cache",
                                         "target",
                                         "vendor",
                                         "coverage"};

const unordered_set<string> SUPPORTED_EXTENSIONS = {".c",
                                                    ".cpp",
                                                    ".h",
                                                    ".hpp",
                                                    ".js",
                                                    ".ts",
                                                    ".py",
                                                    ".java",
                                                    ".go",
                                                    ".rs",
                                                    ".rb",
                                                    ".sh",
                                                    ".cs",
                                                    ".php",
                                                    ".swift",
                                                    ".kt",
                                                    ".scala"};

const unordered_set<string> OUTPUT_FILES = {"doc.md", "report.html"};

// Get language name from file extension
string getLanguage(const string& ext)
{
    static const unordered_map<string, string> LANGUAGE_MAP = {{".cpp", "cpp"},
                                                               {".c", "c"},
                                                               {".h", "c"},
                                                               {".hpp", "cpp"},
                                                               {".py", "python"},
                                                               {".js", "javascript"},
                                                               {".ts", "typescript"},
                                                               {".java", "java"},
                                                               {".go", "go"},
                                                               {".rs", "rust"},
                                                               {".rb", "ruby"},
                                                               {".sh", "bash"},
                                                               {".cs", "csharp"},
                                                               {".php", "php"},
                                                               {".swift", "swift"},
                                                               {".kt", "kotlin"},
                                                               {".scala", "scala"}};

    auto it = LANGUAGE_MAP.find(ext);
    return it != LANGUAGE_MAP.end() ? it->second : "text";
}

// Print usage help
void printHelp(const char* progname)
{
    cout << "DocTrack - Extract documentation from code comments\n\n";
    cout << "Usage: " << progname << " [OPTIONS] [DIRECTORY]\n\n";
    cout << "Options:\n";
    cout << "  -h, --help     Show this help\n";
    cout << "  -o, --output   Output directory (default: docs/)\n";
    cout << "  --no-html      Skip HTML generation\n\n";
    cout << "Examples:\n";
    cout << "  " << progname << " .           # Scan current directory\n";
    cout << "  " << progname << " src/        # Scan src/ folder\n";
    cout << "  " << progname << " -o out .    # Custom output directory\n";
}

struct CommentInfo
{
    int    line_number;
    string tag;
    string message;
    string context;
    bool   is_multiline;
    int    start_line;
    int    end_line;
};

vector<CommentInfo> extractComments(const fs::path& filePath)
{
    ifstream file(filePath);
    if (!file.is_open())
        return {};

    vector<CommentInfo> comments;
    string              ext  = filePath.extension().string();
    string              lang = getLanguage(ext);

    // Regex patterns
    regex singleLineReg(R"((//|#)\s*(TODO|NOTE|BUG|FIXME|CODENOTE|HACK|OPTIMIZE)\s*:?\s*(.*))",
                        regex::icase);
    regex multiLineStartReg(R"((/\*)\s*(TODO|NOTE|BUG|FIXME|CODENOTE|HACK|OPTIMIZE)\s*:?\s*(.*))",
                            regex::icase);

    string      line;
    int         line_num           = 0;
    bool        inMultiLineComment = false;
    CommentInfo currentMultiLine;
    string      multiLineContent;

    while (getline(file, line))
    {
        line_num++;

        if (inMultiLineComment)
        {
            size_t endPos = line.find("*/");
            if (endPos != string::npos)
            {
                currentMultiLine.end_line = line_num;
                currentMultiLine.context  = multiLineContent;
                comments.push_back(currentMultiLine);

                inMultiLineComment = false;
                multiLineContent.clear();
            }
            else
            {
                multiLineContent += line + "\n";
            }
            continue;
        }

        smatch match;
        if (regex_search(line, match, singleLineReg))
        {
            CommentInfo info;
            info.line_number  = line_num;
            info.tag          = match[2];
            info.message      = match[3];
            info.context      = line;
            info.is_multiline = false;
            info.start_line   = line_num;
            info.end_line     = line_num;

            transform(info.tag.begin(), info.tag.end(), info.tag.begin(), ::toupper);
            comments.push_back(info);
        }
        else if (regex_search(line, match, multiLineStartReg))
        {
            inMultiLineComment            = true;
            currentMultiLine.line_number  = line_num;
            currentMultiLine.tag          = match[2];
            currentMultiLine.message      = match[3];
            currentMultiLine.is_multiline = true;
            currentMultiLine.start_line   = line_num;

            transform(currentMultiLine.tag.begin(),
                      currentMultiLine.tag.end(),
                      currentMultiLine.tag.begin(),
                      ::toupper);

            size_t endPos = line.find("*/", match[0].length());
            if (endPos != string::npos)
            {
                currentMultiLine.end_line = line_num;
                currentMultiLine.context  = line;
                comments.push_back(currentMultiLine);
                inMultiLineComment = false;
            }
            else
            {
                multiLineContent = line.substr(match[0].length()) + "\n";
            }
        }
    }

    return comments;
}

void writeMarkdownReport(const vector<pair<fs::path, vector<CommentInfo>>>& fileComments,
                         ofstream&                                          outputFile,
                         const fs::path&                                    target_dir)
{
    auto   now      = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);

    outputFile << "# DocTrack Report\n\n";
    outputFile << "**Source:** " << fs::absolute(target_dir).string() << "\n";
    outputFile << "**Generated:** " << ctime(&now_time);
    outputFile << "---\n";

    int totalComments = 0;
    for (const auto& [filePath, comments] : fileComments)
    {
        if (comments.empty())
            continue;

        totalComments += comments.size();
        outputFile << "\n# FILE: " << filePath.filename().string() << "\n";
        outputFile << "**Path:** " << filePath.string() << "\n";
        outputFile << "---\n";

        for (const auto& comment : comments)
        {
            outputFile << "\n### "
                       << (comment.is_multiline ? "Lines " + to_string(comment.start_line) + "-" +
                                                      to_string(comment.end_line)
                                                : "Line " + to_string(comment.line_number));

            outputFile << " [" << comment.tag << "] → " << comment.message << "\n";

            if (!comment.context.empty())
            {
                outputFile << "```" << getLanguage(filePath.extension().string()) << "\n";
                outputFile << comment.context;
                if (comment.context.back() != '\n')
                    outputFile << "\n";
                outputFile << "```\n";
            }
            outputFile << "---\n";
        }
    }

    outputFile << "\n## Summary\n";
    outputFile << "**Total files processed:** " << fileComments.size() << "\n";
    outputFile << "**Total comments found:** " << totalComments << "\n";
}

void processFiles(const fs::path&                              search_dir,
                  vector<pair<fs::path, vector<CommentInfo>>>& fileComments)
{
    try
    {
        for (const auto& entry : fs::recursive_directory_iterator(search_dir))
        {
            if (entry.is_directory())
            {
                if (SKIP_DIRS.find(entry.path().filename().string()) != SKIP_DIRS.end())
                {
                    continue;
                }
            }
            else if (entry.is_regular_file())
            {
                string ext      = entry.path().extension().string();
                string filename = entry.path().filename().string();

                if (SUPPORTED_EXTENSIONS.find(ext) != SUPPORTED_EXTENSIONS.end() &&
                    OUTPUT_FILES.find(filename) == OUTPUT_FILES.end())
                {

                    auto comments = extractComments(entry.path());
                    if (!comments.empty())
                    {
                        fileComments.emplace_back(entry.path(), move(comments));
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        cerr << "Error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[])
{
    fs::path target_dir    = ".";
    fs::path output_dir    = "docs";
    bool     generate_html = true;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            printHelp(argv[0]);
            return 0;
        }
        else if (arg == "-o" || arg == "--output")
        {
            if (i + 1 < argc)
            {
                output_dir = argv[++i];
            }
            else
            {
                cerr << "Error: --output requires a directory argument\n";
                return 1;
            }
        }
        else if (arg == "--no-html")
        {
            generate_html = false;
        }
        else if (arg[0] != '-')
        {
            target_dir = arg;
        }
        else
        {
            cerr << "❌ Unknown option: " << arg << "\n";
            cerr << "Use --help for usage information\n";
            return 1;
        }
    }

    // Validate target directory
    if (!fs::exists(target_dir) || !fs::is_directory(target_dir))
    {
        cerr << "❌ Invalid directory: " << target_dir << "\n";
        return 1;
    }

    // Create output directory
    fs::create_directories(output_dir);

    fs::path md_path = output_dir / "doc.md";

    cout << "🚀 Running DocTrack on " << target_dir << "\n";
    cout << "📁 Output directory: " << output_dir << "\n";

    vector<pair<fs::path, vector<CommentInfo>>> fileComments;
    processFiles(target_dir, fileComments);

    ofstream outputFile(md_path);
    if (!outputFile)
    {
        cerr << "Error: Could not create output file: " << md_path << "\n";
        return 1;
    }

    writeMarkdownReport(fileComments, outputFile, target_dir);
    outputFile.close();

    cout << "✅ Scan complete. Found " << fileComments.size() << " files with comments.\n";
    cout << "📄 Markdown: " << md_path << "\n";

    // Generate HTML if requested
    if (generate_html)
    {
        fs::path html_path = output_dir / "report.html";
        cout << "🌐 Generating HTML report...\n";

        string command = "pandoc \"" + md_path.string() + "\" -s -o \"" + html_path.string() + "\"";
        int    ret     = system(command.c_str());

        if (ret != 0)
        {
            cerr << "⚠️ HTML generation failed. Install pandoc or use --no-html\n";
            cerr << " Markdown report available at: " << md_path << "\n";
        }
        else
        {
            cout << "📄 HTML: " << html_path << "\n";
        }
    }

    return 0;
}
