# C with Pthreads

## Requirements

- Visual Studio Code with the Microsoft C/C++ extension (`ms-vscode.cpptools`)
- MSYS2 UCRT64 with GCC and GDB installed

In the MSYS2 UCRT64 terminal, install the compiler and debugger:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb
```

The VS Code configuration uses the default MSYS2 UCRT64 paths directly, so adding them to `PATH` is not necessary. Restart VS Code after installing the tools. If MSYS2 is installed in a different folder, update the compiler and debugger paths in `.vscode`.

## Build and run

Open this folder in VS Code, open `main.c`, and use the C/C++ Run button or press `F5`. The configured task builds `main.c` with pthread support before launching the program. You can also build with `Ctrl+Shift+B`.

The sample starts and joins a POSIX thread. Put additional C source files in the build task's `args` list as the project grows.
