# Base
*Single-file* public domain libraries for C, originally inspired by [Mr. 4th](https://www.youtube.com/@Mr4thProgramming).\
(Well, technically *double-file* — each library comes as a `.h` and `.c` pair.)

| library | files | description |
|-|-|-|
| **Base**           | `Base.h`, `Base.c` | This is my standard library _replacement_ and the _essential_ piece that ties the rest of the codebase together.<br>It depends on nothing*, and everything depends on it. |
| **LongGFX**        | `LongGFX.h`, `LongGFX_Win32.c` | OS-abstracted library for graphics and multimedia applications, offering a simple API for window creation, message boxes, and input handling. |
| **LongGFX_D3D11**  | `LongGFX_D3D11.h`, `Win32_D3D11.c` | `LongGFX` backend for creating a `D3D11` graphics context on `Win32`. |
| **LongGFX_OpenGL** | `LongGFX_OpenGL.h`, `Win32_OpenGL.c` | `LongGFX` backend for creating an `OpenGL` graphics context on `Win32`. |
| **LongTest**       | `LongTest.h` | A simple, single-file framework for running tests. |
| **LongScanner**    | `LongScanner.h`, `LongScanner.c` | A scanner and tokenizer for *UTF-8-encoded* text, suitable for building more advanced lexers and parsers.<br>Also includes **CSV** and **JSON** parsers built using it. |
| **CLex**           | `CLex.h`, `CLex.c` | An experimental non-recursive C parser that parses types (without a type table) and expressions (with proper precedence). |
| **Metagen**        | `Metagen.c`, `Metagen.h`, `Metamain.c`| A metaprogramming system that consumes [Metadesk](https://vimeo.com/650161731) files to generate code and data tables.<br>Inspired by [Ryan Fleury](https://www.rfleury.com/p/table-driven-code-generation). |
