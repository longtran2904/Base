# Base
*Single-file* public domain libraries for C.\
(Well, technically *double-file* — each library comes as a `.h` and `.c` pair.)

| library | files | description |
|-|-|-|
| **Base**           | `Base.h`, `Base.c` | This is my standard library _replacement_ and the _essential_ piece that ties the rest of the codebase together, originally inspired by [Mr. 4th](https://www.youtube.com/playlist?list=PLT6InxK-XQvNKTyLXk6H6KKy12UYS_KDL).<br>It depends on nothing*, and everything depends on it. |
| **LongGFX**        | `LongGFX.h`, `LongGFX_Win32.c` | OS-abstracted library for graphics and multimedia applications, offering a simple API for window creation, message boxes, and input handling. |
| **LongGFX_D3D11**  | `LongGFX_D3D11.h`, `Win32_D3D11.c` | `LongGFX` backend for creating a `D3D11` graphics context on `Win32`. |
| **LongGFX_OpenGL** | `LongGFX_OpenGL.h`, `Win32_OpenGL.c` | `LongGFX` backend for creating an `OpenGL` graphics context on `Win32`. |
| **LongTest**       | `LongTest.h` | A simple, single-file framework for running tests. |
| **LongScanner**    | `LongScanner.h`, `LongScanner.c` | A scanner and tokenizer for *UTF-8-encoded* text, suitable for building more advanced lexers and parsers.<br>Also includes **CSV** and **JSON** parsers built using it. |
| **CLex**           | `CLex.h`, `CLex.c` | An experimental non-recursive C parser that parses types (without a type table) and expressions (with proper precedence). |
| **Metagen**        | `Metagen.c`, `Metagen.h`, `Metamain.c`| A metaprogramming system that consumes [Metadesk](https://vimeo.com/650161731) files to generate code and data tables.<br>Inspired by [Ryan Fleury](https://www.rfleury.com/p/table-driven-code-generation). |

## Examples
Here are a few example programs built using the libraries above—some turned out more useful than I thought:

### demo
Showcases most of the features in `Base`. For example, logging:

<img width="428" height="314" alt="image" src="https://github.com/user-attachments/assets/936a66ed-34bc-474c-b08e-02f805e6c261" />

### bench_mem
A simple program that benchmarks your system's memory bandwidth, inspired by [@mmozeiko](https://github.com/mmozeiko/).

<img width="358" height="693" alt="image" src="https://github.com/user-attachments/assets/29777576-41b3-4460-8e21-80d833f6bd80" />

### glob
A CLI tool for [globbing](https://en.wikipedia.org/wiki/Glob_(programming)) file names or contents. It’s multi-threaded and uses Win32 **IOCP** with **Async IO** for file loading.\
This is my _favorite_ tool by far. Once you get used to the _extended_ syntax, it feels much faster and more natural than `findstr`.

<img width="960" height="420" alt="image" src="https://github.com/user-attachments/assets/b38c2373-e36b-46d1-af02-2878920749a5" />
