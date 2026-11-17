[README.md](https://github.com/user-attachments/files/34998408/README.md)
# my-implementation-exam-paper-repository# Exam Paper Repository — repaired & extended **C++ console** project

This is the corrected version of the original `this1.docx` program, plus the two
lab topics that were missing.

## What was broken in the original `.docx`
| Problem | Fix here |
|---|---|
| Bare `Virtual` token before `class User` | removed — real `abstract` class with pure virtual functions |
| Bare `Demo` token before `oopDemoMenu()` | removed |
| `cout << ”11. Type Casting\n”;` (smart quotes) | replaced with normal quotes |
| `pauseScreen()` needed **two** Enters | now needs one |
| Exception handling / multithreading listed as "NOT used" | both fully implemented |

## Build & run
```bash
cd cpp-console-project
make
./exam_repo
```
Demo login: `student` / `faculty` / `admin` (all password `1234`).
Admin dashboard → **9. OOP Lab Demo** → **12. Exception Handling**, **13. Multithreading**.

## Concepts implemented
**Original OOP (preserved):** classes/objects, encapsulation, abstract class,
inheritance, polymorphism, virtual functions, virtual base class / diamond problem,
constructors, `this` pointer, pointers to objects and derived classes, overriding,
function overloading, operator overloading, friend function, access specifiers,
basic↔class and class→class casting.

**New — Exception Handling:**
- `try` / `catch` / `throw`
- `throws` equivalent: `noexcept(false)` + documented callee contract (C++17 removed
  dynamic exception specifications; Java's `throws` has no exact C++ keyword)
- `finally` block: RAII `Finally` scope guard whose destructor always runs
- user-defined exceptions: `RepositoryException` base + 6 derived classes
- catching derived-class before base-class, and catching via base-class reference

**New — Multithreading:**
- `std::thread` with `std::mutex` and `std::atomic`
- `std::async` / `std::future` with exceptions propagating through `future.get()`
- exceptions thrown inside worker threads are caught locally so the app keeps running
- race-condition avoidance demonstrated: both counters reach exactly 80000

## Files
- `main.cpp` — entire program, commented by concept
- `Makefile` — `make`, `make run`, `make clean`
- `README.md` — this file
