# C with Pthreads

## Requirements

- Visual Studio Code with the Microsoft C/C++ extension (`ms-vscode.cpptools`)
- MSYS2 UCRT64 with GCC installed (GDB is only needed for debugging)

In the MSYS2 UCRT64 terminal, install the compiler:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

The VS Code configuration uses the default MSYS2 UCRT64 paths directly, so adding them to `PATH` is not necessary. Restart VS Code after installing GCC. If MSYS2 is installed in a different folder, update the compiler path in `.vscode`.

## Build and run

Open this folder in VS Code, select **C: run main** in the Run and Debug configuration dropdown, and click the green Run button or press `F5`. The configured task builds `main.c` with pthread support before launching the program. This run configuration does not require GDB. You can also build with `Ctrl+Shift+B`.

The sample starts and joins a POSIX thread. Put additional C source files in the build task's `args` list as the project grows.
