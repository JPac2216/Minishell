# Minishell

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
