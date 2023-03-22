
### GNU Aris distribution

GNU Aris is a logical proof program that provides support for propositional and predicate logic, Boolean algebra, and arithmetical logic in the form of abstract sequences (seqlog). This software is distributed under the GNU General Public License.

--------------------
Copyright (C) 2012, 2013 Ian Dunn

Copying and distribution of this file, with or without modification, are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.

----
### Table of Contents
- [Installation Guide](#installation-guide)
  - [Build GNU-ARI through source](#build-gnu-ari-through-source)
- [Contributing Guide](#contributing-guide)
  - [Getting Started](#getting-started)
  - [How to Contribute](#how-to-contribute)
  - [Report a Bug](#report-a-bug)
- [Contact Us](#contact-us)
- [References](#references)

### Installation Guide
#### Build GNU-ARI through source
To build GNU Aris through source, follow the instructions given below:

1. Clone the Aris repository from https://github.com/kovzol/aris using the command `git clone https://github.com/kovzol/aris`

2. Create a directory named build.

3. Run the command `cmake ...`

4. Run the command `make`

5. If you find this, that means you're done and ready to launch your aris from the source.
![](https://paper-attachments.dropboxusercontent.com/s_9CA2A95225FBFB6C39417465DAA6CDC3851E677741EEA4E771571459D05051FA_1678791691570_image.png)

Step 6) Launch the Aris using ./aris

#### Build GNUI-ARIS through snap
To install GNU Aris through snap, visit https://snapcraft.io/aris and find the required distribution details according to your system. For example, for Linux machines, use `sudo snap install aris` to install the Aris and launch it with `aris`.

For further instructions on building and installation, refer to the ./INSTALL file.

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
