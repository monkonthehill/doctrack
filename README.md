
# 🚀 DocTrack

**DocTrack** is a real-time code documentation tool that extracts developer notes directly from source code and converts them into structured, readable documentation.

It scans your codebase for comments like:

```cpp
// TODO: improve logic
// BUG: fix crash
// NOTE: important behavior
```

…and generates clean **Markdown + HTML reports** with optional code context.

---

## ✨ Features

* 🔍 Scans entire project directories
* 🧠 Detects:

  * TODO
  * NOTE
  * BUG
  * FIXME
  * CODENOTE
* 📄 Generates:

  * `doc.md` (Markdown)
  * `report.html` (HTML via Pandoc)
* 📦 Extracts code context for critical tags (BUG, FIXME, CODENOTE)
* ⚡ Works across multiple file types:

  * `.cpp`, `.h`, `.js`, `.py`
* 🚫 Skips unnecessary folders:

  * `.git`, `node_modules`, `build`, etc.

---

## 🛠 Installation

### 1. Clone the repository

```bash
git clone https://github.com/monkonthehill/DocTrack.git
cd DocTrack
```

---

### 2. Build the project

```bash
g++ -std=c++17 src/main.cpp -o doctrack_bin
```

---

### 3. Setup dependencies

```bash
chmod +x setup.sh
./setup.sh
```

---

### 4. Install CLI command (optional but recommended)

```bash
chmod +x doctrack
sudo mv doctrack /usr/local/bin/
```

Now you can use it globally:

```bash
doctrack .
```

---

## 🚀 Usage

### Scan current directory

```bash
doctrack .
```

### Scan specific folder

```bash
doctrack src/
```

---

## 📄 Output

After running:

```bash
doctrack .
```

You will get:

* `doc.md` → structured documentation
* `report.html` → formatted HTML report

---

## 📌 Example Output

````text
# FILE: calculator.cpp
---

### Line 12 [BUG] → fix division logic

```cpp
int divide(int a, int b) {
    // BUG: division by zero
    return a / b;
}
````

```

---

## ⚙️ Requirements

- C++17 compatible compiler  
- :contentReference[oaicite:0]{index=0} (for HTML output)  
- Optional: LaTeX (for PDF export)

---

## 🧠 How It Works

1. Recursively scans project directories  
2. Detects tagged comments using regex  
3. Extracts:
   - file name  
   - line number  
   - message  
4. Captures surrounding code context  
5. Generates structured documentation  

---

## 🚧 Limitations

- Context extraction is heuristic-based (not AST-level)
- Works best with properly formatted comments
- Multi-language support is basic

---

## 🔮 Future Improvements

- CLI commands (`init`, `watch`, etc.)
- VS Code extension
- Git integration
- Priority detection system
- Web dashboard

---

## 🤝 Contributing

Pull requests are welcome. For major changes, open an issue first.

---

## 📜 License

MIT License

---

## 👨‍💻 Author

Built by Nitish
```
