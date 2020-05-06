The code approval procedure is described in the [code approval procedure wiki](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/wikis/Code-Approval-Procedure)

- [ ] The MR is without WIP (work in progress) status
- [ ] Make sure the most recent CI jobs (config, quality, build, tests) all run fine with no failures
  - if "check clang-format" failed: the code contributor has to run `./do-clang-format.py --apply` eventually with the `--all` option
  - if "check copyright" failed the code contributor has to run`./do-copyright.py --add=20xy` 
- [ ] Also run all the extra optional jobs "coverage", "release-clang-8", "release-u-18.04" and make sure no problems occur
- [ ] Check in the "coverage" job output that the coverage did not decreases (!). It should always stay, or increase. If it decreased --> ask contributor to add further unit tests, and check coverage report. 
- On the MR page, open the "Open in Web IDE" tool
  - [ ] Check if the provided solution solves the Issue, discuss on gitlab
  - [ ] Check that all changes are actually related to the issue
  - [ ] There are no debug statements left, not even commented out
  - [ ] Check all changes for coding rules and guidelines
- When all above is done
  - [ ] **Add label "Code Review Finished"**
