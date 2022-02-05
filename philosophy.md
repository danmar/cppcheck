
# Cppcheck Philosophy

It is important that everybody in the Cppcheck team has a consistent idea about how this tool should work.

This is a static analyzer tool.


## Normal analysis - No false positives

A fundamental goal is "no false positives".

It is not possible to achieve "no false positives" completely. One case where false positives are OK is when the code is garbage.

If the code is written as it is by design, then our goal is to not warn.

If it is not known if there is a problem, then in general we need to bailout. We can only warn when we see that there is a problem.

Stylistic checks are much more prone to false positives and therefore we should avoid writing stylistic checks mostly.

Reporting issues in Trac:
 - If you see a false negative; report that as an enhancement.
 - If you see a false positive; report that as a defect.

### Inconclusive messages

Inconclusive messages will be created if cppcheck cannot be sure there is an issue to warn but 50-50 probability. User shall enable inconclusive messages if they are willing to spend substantially more time on message verification in order to find more issues within a high false positive rate.
Inconclusive messages shall not be used for new checks which are just being developed. There `settings.experimental` can be used.

## Bug hunting - Soundy analysis

The goal is to detect nearly all bugs. It will not be possible to detect ALL bugs. For instance if the code is garbage or if the bug happens in a inline assembler code block.

It will not be possible to avoid false alarms completely but we can not be sloppy about false alarms. There are tools that are too noisy. A handful of false alarms for a project is totally fine in this analysis. But 1000's of false alarms for the average project would not be ok.

We want to detect UB. But we will not add checkers that are too noisy.

Reporting issues in Trac:
 - If you see a false negative; report that as a defect.
 - If you see a false positive; report it as an enhancement.

We should try hard to fix false positives however we can't have heuristics that we know will cause important false negatives.


## No configuration

We want that a user can run Cppcheck without explicit -D and -I configuration.

When this happens the false positives should be avoided. The user can reduce false negatives with configuration.


## Allow compiler extensions

This is not just a tool for mainstream gcc/msvc c/c++ developers. If you can compile the code with a C/C++ compiler then our goal is that Cppcheck can check it.


## C++ language

Our goal is to be highly portable. Users must be able to compile Cppcheck with GCC 4.8 or Visual Studio 2013.

No C++14 is allowed. A subset of C++11 is allowed.


## Avoid dependencies

We are very careful about dependencies.



