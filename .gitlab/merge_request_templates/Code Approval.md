Issues: Closes #.... 

The code approval procedure is described in the wiki: [Code approval procedure wiki](https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/-/wikis/Code-Approval-Procedure)

- [ ] The MR is without `WIP/Draft` status
- [ ] Make sure the most recent CI jobs (config, quality, build_test_example) all run fine with no failures
  - if "check clang-format" failed: the code contributor has to run `./do-clang-format.py --apply` eventually with the `--all` option
  - if "check copyright" failed the code contributor has to run`./do-copyright.py --add=20xy` 
- [ ] Make sure also the jobs with MR-label `ready for code review` succeed. This includes the optional jobs, in particular 'coverage', 'release-full-clang-14", "release-full-u-22_04" and make sure no problems occur. You may have to trigger a pipeline manually to check this. 
- [ ] Check in the "coverage" job output that the coverage did not decrease. It should always stay, or increase. If it decreased --> ask contributor to add further needed unit tests, and check coverage report. 
- On the MR page, open the "Open in Web IDE" tool
  - [ ] Check if the provided solution solves the Issue, discuss on gitlab
  - [ ] Check that all changes are actually related to the issue
  - [ ] There are no debug statements left, not even commented out
  - [ ] Check all changes for coding rules and guidelines
- When all above is done: 
  - **Add MR label "Code Review Finished"**
  
