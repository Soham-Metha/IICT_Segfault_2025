# Team Kernull – Segfault Hackathon 2025

This project is our submission for the **IICT Segfault Hackathon 2025**.
We are building a **new programming language and compiler**, designed on top of our custom virtual machine (**VIREX**) - making it a complete toolchain from **source code -> AST -> bytecode -> execution**.

## 🚀 Key Innovations

* **Custom Programming Language**

  * Designed specifically for this hackathon project.
  * Includes its own syntax, semantics, and compiler frontend.

* **Compiler on Top of VIREX**

  * Not just a wrapper - we implemented a full compiler targeting the VIREX VM.
  * Translates high-level code into VIREX bytecode.

* **AST Visualization**

  * Unique **graphical representation of Abstract Syntax Trees**.
  * Color-coded and shape-differentiated nodes for intuitive debugging.

* **NCurses-based UI Layer**

  * Terminal interface for smooth interaction and visualization.
  * Wrappers for I/O and runtime interaction.

## 🏗️ Architecture

![Architecture Diagram](doc/Arch.png)

The project consists of:

1. **Frontend (Compiler)** -> parses source language -> generates AST -> lowers to VIREX bytecode.
2. **Backend (VIREX)** -> executes bytecode efficiently.
3. **Visualization** -> renders AST and debugging info.
4. **UI** -> user-friendly interface via NCurses.

## ✅ Current Progress / Roadmap / TODOs

* [ ] Finalize project name
* [ ] Finalize language name
* [ ] Finalize compiler name
* [x] Design AST visuals (colors, shapes, etc.)
* [x] Implement NCurses-based UI wrapper functions
* [x] Implement I/O function wrappers
* [x] Implement compiler (parsing, analysis, codegen)
* [ ] Implement code optimizations.

## ⚡ Powered by VIREX

VIREX serves as the **execution engine**, but everything above (compiler, language design, AST visualizer, UI) is **newly built** in this repo.
