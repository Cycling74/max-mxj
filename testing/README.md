# max-mxj-testing

Testing Tools and Resources for the max-mxj package.




## Prerequisites

1. **Install a copy of Max** that you wish to use.  
It may be installed anywhere as long as you know where it is.
2. Download the latest release of the [Max-Test package](https://github.com/Cycling74/max-test/releases) and place it in your **Packages** folder.  If the name is not `max-test` (it may have some version information in the name) then rename it to simply be `max-test`.
If you are testing both Max 6 and Max 7 you will need to copy it into both Packages folders.
3. Download this package (or clone it from Github) into your **Packages** folder (or both Packages folders as noted above).

If you wish to automate the testing you will to additionally do the following (as documented in the ReadMe of the **Max-Test** package):

1. Find the file named `testpackage-config-example.json` in the `max-test/misc` folder.
2. Make a copy/duplicate of the file
3. Rename the copy `testpackage-config.json`

This will open the network ports for the "oscar" extension in max-test to communicate remotely with the automated test harness.


## To Run Tests Manually...

Simply open a test patcher (a patcher ending with the `.maxtest.maxpat` filename suffix). Look for a `1` or a `0` immediately above any `test.assert` objects.

## To Run Tests with Automation

1. Open a terminal window
2. cd into the `max-test` package folder
3. `cd ruby`
4. `ruby test.rb /Applications` where the "/Applications" may be substituted with a different path if the copy of Max you wish to test is somewhere else.
5. Wait a few minutes for it to go through its paces.

Additional documentation on the process can be found in the `max-test` repository.