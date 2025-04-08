# Minishell Project in C

**Author:** Jake Paccione

## Overview

Minishell is a basic shell program written in C that implements a command-line interpreter with a few built-in commands, signal handling, and the ability to execute external commands using fork/exec. The shell provides a colorized prompt that displays the current working directory and supports built-in commands such as `cd`, `exit`, `pwd`, `lf`, and `lp`.

## Features

- **Colored Prompt:**  
  The prompt displays the current working directory in blue, enclosed in square brackets (e.g., `[~/minishell]> `). This is achieved by printing ANSI escape codes:
  ```c
  #define BLUE "\x1b[34;1m"
  #define DEFAULT "\x1b[0m"
  ```
- **Built-in Commands:**

  **cd:** Changes the working directory. When invoked with no argument or with `~`, it changes to the user's home directory using `getuid()`, `getpwuid()`, and `chdir()`.
  
  **exit:** Exits the shell by calling `exit(EXIT_SUCCESS)`.
  
  **pwd:** Prints the current working directory.
  
  **lf:** Lists all files (except `.` and `..`) in the current directory including hidden files.
  
  **lp:** Lists all processes in the system in the format `<PID> <USER> <COMMAND>`. This command reads from `/proc/` by identifying directories with numeric names, retrieves   the process owner's username using `getpwuid()`, and reads the command invoked from `/proc/<PID>/cmdline`.

- **External Command Execution:**
  Any commands not implemented as a built-in are executed using `fork()` followed by `execvp()`. The parent process waits for the child to terminate before printing a new      prompt.

- **Signal Handling:**
  The shell installs a `SIGINT` (Ctrl+C) handler using `sigaction()`.

  When SIGINT is received, the handler sets a `global volatile sig_atomic_t` flag (interrupted) and prints a newline so that the shell returns cleanly to a prompt.

  If `getline()` is interrupted by SIGINT, the shell clears the error on `stdin` and continues without processing the partial input.

- **Error Handling:**
  The shell checks for errors on system calls and library functions. If an error occurs (ex: failure to get the current working directory, `malloc()` failures, or `execvp()`   failure), an error message is printed to `stderr` using `fprintf()`.

## Compilation & Execution

To compile the minishell, use:

```bash
gcc minishell.c
```
Then run the shell using:

```bash
./a.out
```

## Final Notes

The minishlel project was created as an assignment for CS 392 Systems Programming, using concepts discussed in class. 
