# The Compiler
This compiler is written in **C++**. 

## Branch for Grading
For grading purposes, the main branch will constantly keep being updated for the current project, so to get the copy of a completed project with no additions from future projects, check the **branch for that specific project** for grading. Do NOT grade the main branch please. :) 

## Running the Compiler
### In order to run the C++ code, you need a couple things set up
You need to be able to access the make command to run the code using the Makefile provided.
You also need access to the g++ command since the Makefile uses g++ to compile the C++ code in this repository.

#### LINUX:
  - To get g++, simply run "sudo apt install g++" in the terminal
  - To get make, simply run "sudo apt install make" in the terminal
  - If these don't work, try running "sudo apt update" in the terminal

#### WINDOWS:
  - You need to install tools to get these working
  - Something I recommend would be installing MSYS2 (MinGW64) to get access to these commands

#### MAC:
  - This repository has not been explicitly tested on MacOS, but you should probably be able to install g++ and make using Homebrew
  - Run "brew install gcc make" in the terminal if Homebrew is installed on your machine

## After setting up...
  
Go into the directory of the assignment you want to run, and simply run the command:
  - make FILE='*filename*'

where *filename* is the file you want to be compiled. 

**NOTE THAT THE ARGUMENT DOESN'T ASSUME THE FILE IS IN THE testFiles FOLDER.** For example, if you decide to use a file in the testFiles folder, you need to specify the folder, using the syntax like: 
  - make FILE='testFiles/code.txt'
    
The output should appear in the terminal. 
If you want to remove the extra files added when running "make", simply run:
  - make clean

If you are on Linux and want to grind **Valgrind** to check for memory leakages, you can simply run:
  - make valgrind FILE='*filename*'

## Final Notes
The programs should work if you're running it on either Windows or Linux, but I'm not sure if it would be functional running on Mac.
If any problem arises, just let me know so I could try to fix it.

Quick Note: When running "make clean", if you see a warning message saying something like "'rm' is not recognized as an internal or external command..." when running on Windows, don't worry about it. It will still remove the unneccesary files.
