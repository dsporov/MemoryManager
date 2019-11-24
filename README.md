# MemoryManager

Implementation of a memory management interface that allocates blocks of a certain fixed size from a pre-allocated continuous region of memory

## Table of Contents
* [Prerequisites](#prerequisites)
* [Cloning](#cloning)
* [IDE Setup](#ide-setup)
* [Build and run](#build-run)

<a name="prerequisites"></a>
## Prerequisites
1. Install [Visual Studio Community 2019][1]
2. Install [git][0]

<a name="cloning"></a>
## Cloning
```
git clone https://github.com/dsporov/MemoryManager.git
```

<a name="ide-setup"></a>
## IDE Setup
Visual Studio does not require any additional setup. There are no 3rd party libraries other than [google test][2] used in the project. However Google Test has been included into the git repository as a precompiled version.

<a name="build-run"></a>
## Build and run
1. Run Visual Studio.
2. Open __MemManager.sln__.
3. Select the required platform (Win32/x64) & configuration (Release/Debug).
4. Build the solution.
5. Run the executable from the IDE (a bunch of unit tests should be executed).

[0]: https://git-scm.com/
[1]: https://visualstudio.microsoft.com/vs/community/
[2]: https://github.com/google/googletest
