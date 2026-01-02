---
hide:
  - toc
---

# Frontend: File Layer

Welcome to the **test-driven journey** for the `Layer_File` module.  
This module is foundational in managing source file input for the compiler pipeline. We will build it step-by-step through **small, verifiable test cases**.

The `Layer_File` module is responsible for safely and correctly reading source files and preparing them for subsequent parsing. It handles:

- Opening and validating access to the source file
- Determining file size for buffer allocation
- Reading the file's entire contents into memory
- Splitting content into lines for easy parsing and error reporting
- Tracking the current processing line number

---

## How to Use This Guide

This guide presents _File Layer functionality as a series of **incremental test scenarios** similar in style to the features described in The Ray Tracer Challenge_.

Each test:

- Describes behavior or a feature to implement
- Is designed to run _before_ the implementation
- Guides stepwise development and validation

**Goal:** Make your tests _fail first_, then write minimal implementation to pass them, keeping code clean and robust.

---

## Getting Started: Core Structures and Interfaces

### File_Context Structure

In any file handling operation, what should the code keep track of?  
The most obvious answer would be:

- Contents of the file

However, remember the error messages you get in case of errors?  
Error reporting usually requires: file name, line number & column number where the error occurred.

Since we need to track so much, let’s create a structure that holds all this info:

```c
typedef struct File_Context {
    const char* file_path;
    String contents;
    Line_Context lines;
    unsigned int line_num;
} File_Context;
```

> **Note:**
>
> - `String` is a helper struct defined in `Utils/strings.h` with fields for a character buffer and length.
> - `Line_Context` will later track each line’s start address, length, and possibly line/column info.

---

### Essential Functions

We will gradually implement these:

```c
file_read(const char* filePath, File_Context* context);
file_fetch_next_line(File_Context* context);
file_fetch_curr_line(File_Context* context);
```

For now, focus only on `file_read` implementation.

---

## Error Handling

Start with _minimal error handling_.  
You can expand this later with richer diagnostics.

```c
if (!condition) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}
```

---

## Test Scenarios

Below scenarios guide your incremental implementation.

---

??? example "Scenario 1: Reading a Valid File"

    **Goal:** Verify successful file reading and context initialization.

    **Given** a valid file with multiline text.

    **When** `file_read` is called.

    **Then** confirm:

      - File is read successfully.
      - `contents` contains exact file data with null termination.
      - `file_path` matches the input path.
      - `line_num` is initialized to zero.

---

??? example "Scenario 2: File Open Failure Handling"

    **Goal:** Ensure proper failure handling for nonexistent files.

    **Given** an invalid or inaccessible file path.

    **When** `file_read` is called.

    **Then:**

      - Print an error and exit cleanly.
      - `contents` remains empty or null.

---

Now then, what's `line context`?

It's what will be passed everywhere for processing,
it should mainly contain an individual line that is being processed
and the file name & line no for error reporting.

```c
typedef struct Line_Context {
    String line;
    unsigned int line_no;
    const char* file_name;
    const char* line_start;
} Line_Context;

```

But wheres the column number for error reporting?

It is obtained via _pointer arithmetic_,

In C, strings are just character arrays.

for example, consider string 'Hello world', how would you obtain the position of the second 'o'?

This can be done by _subtracting_ the position of 'H'(string start) from the position of 'o'.

now then, continue with the testcases,

??? example "Scenario 3: Fetching Next Lines Sequentially"

    **Goal:** Confirm sequential iteration over file lines.

    **Given** a `File_Context` with known multiline content.

    **When** repeatedly calling `file_fetch_next_line`.

    **Then:**

      - return the next _unprocessed line_
      - Lines are properly truncated/stripped.
      - `line_num` increments correctly.

    ??? info "hint"

        if you find that the current line wasn't processed(it's length is > 0), just return it as is.

---

??? example "Scenario 4: Fetching the Current Line"

    **Goal:** Retrieve the active line.

    **Given** a `File_Context` with multiple lines already read.

    **When** calling `file_fetch_curr_line`.

    **Then:**

      - Returns the line at the current `line_num`.
