
# GNU Aris

GNU Aris is a logical proof program that provides support for propositional and predicate logic, Boolean algebra, and arithmetical logic in the form of abstract sequences (seqlog). This software is distributed under the GNU General Public License.

Copyright (C) 2012, 2013 Ian Dunn

Copying and distribution of this file, with or without modification, are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.

## Table of Contents
- [Installation Guide](#installation-guide)
  - [gtk version](#gtk-version)
  - [qt version](#qt-version)
  - [WebAssembly](#webassembly)
- [Contributing Guide](#contributing-guide)
  - [Getting Started](#getting-started)
  - [How to Contribute](#how-to-contribute)
  - [Report a Bug](#report-a-bug)
- [Contact Us](#contact-us)
- [References](#references)

## Installation Guide
Check under `Releases` for Linux/Windows bundles and `AppImages` or visit our [website](https://matek.hu/zoltan/aris) .

Else, to build GNU Aris through source, follow the instructions given below. This should work on Linux, Mac or Windows (by using, for example, the MSYS2 toolchain and its MINGW64 environment):

Clone/Download the repository. For example:
   ```
   git clone https://github.com/kovzol/aris
   ```

### gtk version

1. Follow the steps:
```
mkdir build
cd build
cmake ..
```

2. You should see something like this:
```
-- The C compiler identification is GNU 11.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Found LibXml2: /usr/lib/x86_64-linux-gnu/libxml2.so (found version "2.9.13") 
-- Found PkgConfig: /usr/bin/pkg-config (found version "0.29.2") 
-- Checking for module 'gtk+-3.0'
--   Found gtk+-3.0, version 3.24.33
-- Configuring done
-- Generating done
-- Build files have been written to: /home/abc/workspace/aris/build/
```

3. Run the command `make`. You should see something like this at the end of the process:
```
[ 88%] Building C object CMakeFiles/aris.dir/src/undo.c.o
[ 92%] Building C object CMakeFiles/aris.dir/src/var.c.o
[ 96%] Building C object CMakeFiles/aris.dir/src/vec.c.o
[100%] Linking C executable aris
[100%] Built target aris
```

4. Launch using `./aris`.

For further instructions on building and installation ( without `cmake`), refer to the [INSTALL](INSTALL) file.

#### Snap Installation

To install GNU Aris through snap, visit https://snapcraft.io/aris and find the required distribution details according to your system.

### qt version
#### Using `cmake`

Run the following inside `aris/` :
```
cmake -S qt/ -B build-qt/ -DCMAKE_GENERATOR:STRING=Ninja -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build build-qt/ --target all
```
This should generate a binary `aris-qt` inside `build-qt/` , it can be run with `./aris-qt` or you can install to `/usr/local` with:
```
cd build-qt/ && cmake -P cmake_install.cmake
```
To uninstall:
```
cd build-qt/ && xargs rm < install_manifest.txt
```

#### Using `qmake`

Run the following inside `aris/` with appropriate `path/to/qmake`:
```
mkdir build-qt
cd build-qt
path/to/qmake -o Makefile ../qt/aris-qt.pro -spec linux-g++ CONFIG+=qtquickcompiler
```
This should generate a binary `aris-qt` inside `build-qt/` , it can be run with `./aris-qt` .

### WebAssembly

To build to WebAssembly, `Qt 6.5` and `emscripten` version `3.1.25` are required.
Also, since `emscripten` doesn't have support for `libxml2` yet, it would need to be compiled from source.

Follow these steps (after cloning libxml2) to compile `libxml2` :
```
mkdir build
cd libxml2 && autoreconf -if -Wall
cd ../build 
path/to/emconfigure ../libxml2/configure --with-http=no --with-ftp=no --with-python=no --with-threads=no --enable-shared=no
path/to/emmake make
path/to/emar rcs libxml2.a *.o
```

Now, inside `aris/` run the following:
```
mkdir libxml2
cp path/to/above/created/libxml2.a libxml2/
cp -r path/to/cloned-libxml2/include/libxml/ libxml2/
```
You can customize these paths by editing the `aris-qt.pro` file.

Finally:

```
mkdir build-wasm
cd build-wasm
/path/to/Qt/6.5.0/wasm_singlethread/bin/qmake /path/to/aris/qt/aris-qt.pro -spec wasm-emscripten && /usr/bin/make qmake_all
make -j $(nproc)
```

Now, run:
```
python -m http.server 8000
```
The project can be found at `http://0.0.0.0:8000/` .


### Contributing Guide
We welcome contributions to our project and thank you for taking the time to improve it. Here are some guidelines to follow when contributing:

#### Getting Started
Before contributing, please make sure to:
- Read the README file to understand the purpose and scope of the project.
- Look through the existing issues and pull requests to see if your contribution has already been addressed or discussed.
- If you're making significant changes or adding a new feature, consider opening an issue first to discuss it with the project maintainers.

#### How to Contribute
There are several ways to contribute to our project:
- Reporting Bugs: If you find a bug, please open an issue and describe the problem. Make sure to include steps to reproduce the issue and any error messages you receive regarding that issue.
- Fixing Bugs: If you'd like to fix a bug, please create a pull request with your changes. Make sure to include a description of the problem and how your changes will address it.
- Improving Documentation: If you notice any errors or omissions in the documentation, please create a pull request with your changes. Make sure to explain why the changes you made are necessary and how they can improve the documentation.

#### Report a Bug:
Please include enough information for the maintainers to reproduce the problem. Generally, that means:

- The contents of any input files necessary to reproduce the bug and command line invocations of the program(s) involved (very important!).
- A description of the problem and any samples of the erroneous output.
- The version number of the program(s) involved (use --version).
- Hardware, operating system, and compiler versions (uname -a).
- Unusual options you gave to configure, if any (see config.status).
- Anything else that you think would be helpful.

### Contact Us
Thank you for considering contributing to our project! We appreciate your time and effort, and we look forward to working with you.
Feel free to connect with us for discussions regarding our findings.
Mailing list: bug-aris@gnu.org
- Please use this list for all discussion: bug reports, enhancements, etc.
- Anyone is welcome to join the list; to do so, visit http://lists.gnu.org/mailman/listinfo/bug-aris

### References
Distribution: ftp://ftp.gnu.org/gnu/aris/ 

Home page: http://www.gnu.org/software/aris/

Documentation: https://www.gnu.org/software/aris/manual/aris.html

See AUTHORS and THANKS for contributors.

-------------
GNU Aris is free software.  See the COPYING file for copying conditions.
