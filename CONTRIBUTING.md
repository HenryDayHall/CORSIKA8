# Guidelines for code development, structure, formating etc.

The CORSIKA Project very much welcomes contributions. Here we outline
how you can find the right place to contribute, and how to do that.
Connect to https://gitlab.iap.kit.edu and corsika-devel@lists.kit.edu (self-register at https://www.lists.kit.edu/sympa/subscribe/corsika-devel) to get in touch with the project.
The CORSIKA Project decides on the [GUIDELINES](CONTRIBUTING.md) and can decide to
change/improve them.

# How to contribute

  - We organize all development via gitlab `Issues` that may be feature requests,
    ideas, discussions, or bugs fix requests. 
  - New issues can be created, or existing issues
    picked up or contributed to. 
  - Issues are discussed in meetings or via  corsika-devel@lists.kit.edu  
    within the CORSIKA Project.
  - Issues are assigned to milestones. 
  - The work on issues is performed in `branches` that can be best
    created directly via the gitlab web interface. 
  - Proposed code to close one issue (located in a specific git
    branch) is reviewed, discussed, and eventually merged
    into the master branch via a merge-request (MR) to close the issue.
  - all merge request will undergo a code review, and must be approved before merge, in order to ensure high code qualtiy: [Code Approval Procedure](https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/-/wikis/Code-Approval-Procedure)


## Code formatting

We rely on `clang-format` for code formatting. This has the tremendous
advantage that definitely all code follows the same formatting rules,
and nobody at any point needs to invest time and effort into code
formatting. We provide a script `do-clang-format.sh`, which can be
very useful. But we urge everybody to integrate `clang-format` already
on the level of your source code editor. See [the official
page](https://clang.llvm.org/docs/ClangFormat.html) for information
about `clang-format` and its editor integrations. At least: run
`do-clang-format.sh` prior to any `git add/commit` command. Code with
improper formatting will not be accepted for merging. It will trigger automatic warnings by the continuous integration (CI) system.

The definition of source code format is written down in the file
[.clang-format](.clang-format) and can be changed, if the CORSIKA
Project agrees on it. To see what is possible, check
e.g. [link1](https://clangformat.com/) or
[link2](https://zed0.co.uk/clang-format-configurator/).


## Coding rules, conventions and guidelines

Please read the [Coding wiki page](https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/-/wikis/Coding-Conventions-and-Guidelines).




## Release versioning scheme

Releases of CORSIKA 8 are thought to be the baseline for larger scale
validation, and full production.  The releases are numbered as CORSIKA 8 vx.y.z,
starting with x=1. The x index is updated for new releases that normally contain improved or
enhanced physics performance, and also interface
changes to accomodate improvements. The y and z indices can be updated more
frequently for bug fixes or new features. Changes in z will not
contain interface changes, thus, production code will remain
fully compatible within changes of z.  Special releases of CORSIKA might
also have a release name.


# How to become scientific author of the CORSIKA Project

The CORSIKA Project decides on who becomes scientific author. The
following conditions are sufficient, but not all of them are
required all the time:
  - responsibility for a particular functionality or software/management part 
  - have read and follow these [GUIDELINES](CONTRIBUTING.md)
  - active in the CORSIKA Project, that means responsive to
    discussions and problems in corsika-devel@list.kit.edu or on https//gitlab.iap.kit.edu, 
    of relevant *Issues*, in (phone) meetings or our Mattermost workspace
  - agreement to the [COLLABORATION_AGREEMENT](COLLABORATION_AGREEMENT.md) 
  - the members of the CORSIKA Project must agree
