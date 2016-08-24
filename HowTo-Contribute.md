## Contributing to the Max-MXJ Package

The Max-MXJ (MXJ) package is accepting contributions that improve or refine any of the types, functions, content, or documentation. 

All changes should be submitted as Github Pull Requests.


## Housekeeping
Your pull request should: 

* Include a description of what your change intends to do
* The final commit of the pull request must pass all tests on all platforms or it will not be considered. It is desirable, but not necessary, for all the tests to pass at each commit. Please see [ReadMe.md](./ReadMe.md) for instructions to build and run the test suite.
* All commits must have clear and unambiguous commit messages, linked to Github issue tracker number (for example, the commit message includes the text "see #1974" to link it to issue number 1974.) 
* Include appropriate tests 
    * Tests should include reasonable permutations of the target fix/change
    * Include baseline changes with your change
    * All changed code should strive to have 100% code coverage


## Java Compiler Settings

MXJ must be able to execute successfully using Java 6 (the version installed by Apple for many years and still present on many user systems). Thus it is important to use `-target 6` when compiling.

