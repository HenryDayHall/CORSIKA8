# Guidelines for code development, structure, formating etc.

The CORSIKA Project very much welcomes contributions. Here we outlined
how you can find the right place to contribute, and how to do that.
Connect to http://gitlab.ikp.kit.edu and corsika-devel@lists.kit.edu
or corsika-project@lists.kit.edu to get in touch with the project.
The CORSIKA Project decides on the [GUIDELINES](GUIDELINES.md), and can
change them.

## Code formatting

We rely on `clang-format` for code formatting. This has the tremendous
advantage that definitely all code follows the same formatting rules,
and nobody at any point needs to invest time and effort into code
formatting. We provide a script `do-clang-format.sh`, which can be
very useful. But we urge everybody to integrate `clang-format` already
on the level of your source code editor. See [the official
page](https://clang.llvm.org/docs/ClangFormat.html) for information
about `clang-format` and its editor integration. 

The definition of source code format is written down in the file
[.clang-format](.clang-format) and can be changed, if the CORSIKA
Project agrees on it. To see what is possible, check
e.g. [link1](https://clangformat.com/) or
[link2](https://zed0.co.uk/clang-format-configurator/).

## Naming conventions

While `clang-format` does the structural formatting, we still need to agree on naming conventions:

  - Classes and structs start with capital letters
  - Class member variables start with "f"
  - Any static variable has a "g" prefix. A static member variable starts with "fg"
  - Class member functions start with capital letters
  - Any class getter begins with "Get", and setter with "Set"
  - enums should be "enum class", and start with a capital "E", enum entries start with "e"

  - We use namespaces to avoid clashes and to structure code
    - *Everything* is part of the corsika namespace
    - All classes and objects are encapsulated into suited sub-namespaces,
      thus corsika::geometry, corsika::processes, corsika::units, etc.
    - Namespace names do not use capital letters.  
    - Every header file is copied during build and install into
      "include/corsika/[namespace]" which also means, each header file
      can only provide definitions for a _single_ namespace. It is one
      main purpose of namespaces to structure the location of header
      files.
    - Each header file uses an include protection that includes at
      least the namespace name, and header file name, thus, `#ifndef
      __include_geometry_Point_h__` or `#ifnedf __geometry_Point_h__`,
      or similar are acceptable.
    - Header files should always be included with `<..>`, thus,
      `#include <corsika/geometry/Point.h>` since the build system
      will always provide the correct include directives (and files
      anyway cannot be found in file-system paths that typically do
      not follow the namespace naming conventions outlined
      here).

  - Header files are named after the main class (or object) they
    define. This also means each header file name starts with a
    capital letter.
  


## Coding rules

  - Code may not introduce any compiler errors, or warnings
  - All unit tests must succeed at all times
  - We use C++17 concepts wherever useful and helpful
  - On any major error or malfunction we throw an exception. This is needed and required for complex physics and shower debugging.
  - We never catch exceptions for error handling, there might be very few special exceptions from this. We need to discuss such cases.
  - Everything that should not change should be `const`
  - Everything that does not need to be visible to the outside of a class/struct should be `private` or `protected`
  - We prefer the use of references, wherever useful
  - There cannot be any pointer in the interface of any class or object
    exposed to outside users, there might be pointers for very special cases
    inside of classes.
  - When you contribute new code, or extend existing code, at the same time provide unit-tests for all functionality.
  - Code must be documented with `doxygen` commands
  

# How to contribute

  - We organize all development via `Issues` that may be feature requests,
    ideas, or bugs fix requests. 
  - New issues can be created, or existing issues
    picked up. 
  - Issues are discussed in meetings or via  corsika-devel@lists.kit.edu  within the CORSIKA Project.
  - Issues are assigned to milestones. 
  - The work on issues is performed in `branches` that can be best
    created directly via the gitlab web interface. 
  - Proposed code to close one issue (located in a specific git
    branch) is reviewed and eventually discussed, and finally merged
    into the master branch to close the issue.



# How to become scientific author of the CORSIKA Project

The CORSIKA Project decides on who becomes scientific author. The
following conditions are clearly sufficient, but not all of them are
required all the time:
  - responsibility for a particular functionality or part
  - follows these [GUIDELINES](GUIDELINES.md)
  - agrees to the [COLLABORATION_AGREEMENT](COLLABORATION_AGREEMENT.md)
  - active in the CORSIKA Project, that means responsive to
    discussions and problems in corsika-devel@list.kit.edu or on https//gitlab.ikp.kit.edu, of relevant *issues*,
    or in (phone) meetings
  - the members of the CORSIKA Project panel agree
