# clang-tidy

Below are the reasoning why certain checks are (currently) disabled for out code base

## Externals

We do not perform static analysis of the source of the external libraries. `simplecpp` has its own CI with a clang-tidy workflow.

## Disabled Checks

`abseil-*`<br/>
`altera-*`<br/>
`android-*`<br/>
`boost-*`<br/>
`darwin-*`<br/>
`fuchsia-*`<br/>
`linuxkernel-*`<br/>
`llvm-*`<br/>
`llvmlibc-*`<br/>
`mpi-*`<br/>
`objc-*`<br/>
`openmp-*`<br/>
`zircon-*`<br/>

These are disabled since the platforms/libraries in question are not targeted by us.

`cert-*`<br/>
`cppcoreguidelines-*`<br/>
`google-*`<br/>
`hicpp-*`<br/>

These are coding guidelines we do not follow. Some of the checks might be explicitly enabled though.

`readability-braces-around-statements`<br/>
`readability-isolate-declaration`<br/>
`modernize-use-trailing-return-type`<br/>
`modernize-use-auto`<br/>
`readability-uppercase-literal-suffix`<br/>
`readability-else-after-return`<br/>
`readability-identifier-length`<br/>

These do not reflect the style we are (currently) enforcing.

`readability-function-size`<br/>
`readability-function-cognitive-complexity`<br/>

We are not interested in the size/complexity of a function.

`readability-magic-numbers`<br/>

These do not (always) increase readability.

`bugprone-macro-parentheses`<br/>

To be documented.

`readability-implicit-bool-conversion`<br/>

This does not appear to be useful as it is reported on very common code.

`bugprone-narrowing-conversions`<br/>
`performance-no-automatic-move`<br/>

It was decided not to apply these.

`modernize-loop-convert`<br/>

These might change the behavior of code which might not be intended (need to file an upstream issue)

`modernize-raw-string-literal`<br/>

This leads to a mismatch of raw string literals and regular ones and does reduce the readability.

`readability-convert-member-functions-to-static`<br/>

Disabled because of false positives with Qt `slot` methods (see https://github.com/llvm/llvm-project/issues/57520).

`-clang-analyzer-*`<br/>

Disabled because of false positives (needs to file an upstream bug report).

`misc-non-private-member-variables-in-classes`<br/>

We intentionally use this.

`misc-no-recursion`<br/>

Leads to lots of "false positives". This seem to enforce a coding guidelines of certain codebases.


`bugprone-easily-swappable-parameters`<br/>

This produces a lot of noise and they are not fixable that easily.

`readability-container-data-pointer`<br/>

Disable because of false positives and inconsistent warnings (need to file an upstream bug report).

`misc-const-correctness`<br/>

Work in progress.

`bugprone-assignment-in-if-condition`<br/>

Is reported for valid patterns we are using.

`readability-suspicious-call-argument`<br/>

Produces a lot of false positives since it is too vague in its analysis.

`performance-inefficient-string-concatenation`<br/>

Produces warnings which might be considered false positives starting with C++11 - see https://github.com/llvm/llvm-project/issues/54526.

`readability-redundant-access-specifiers`<br/>

Reports warning with the Qt `<access-specifier> slots:` syntax in class declarations - see https://github.com/llvm/llvm-project/issues/60055.

`modernize-avoid-c-arrays`<br/>

Produces warnings when `const char[]` is being used which is quite common in our code. Does not make sense to enable before C++17 when `std::string_view` becomes available.
Also reports a false positive about templates which deduce the array length: https://github.com/llvm/llvm-project/issues/60053.

`misc-include-cleaner`<br/>

We run this separately via `clang-include-cleaner` in the `iwyu.yml` workflow as the findings of the include checkers still need to be reviewed manually before applying them. 

`bugprone-branch-clone`<br/>
`readability-const-return-type`<br/>
`modernize-return-braced-init-list`<br/>
`misc-throw-by-value-catch-by-reference`<br/>
`readability-avoid-const-params-in-decls`<br/>
`bugprone-signed-char-misuse`<br/>
`readability-redundant-access-specifiers`<br/>
`concurrency-mt-unsafe`<br/>
`misc-use-anonymous-namespace`<br/>
`performance-avoid-endl`<br/>
`performance-noexcept-swap`<br/>
`bugprone-switch-missing-default-case`<br/>
`bugprone-empty-catch`<br/>

To be evaluated (need to remove exclusion).

`cppcoreguidelines-missing-std-forward`<br/>
`cppcoreguidelines-avoid-const-or-ref-data-members`<br/>
`cppcoreguidelines-macro-usage`<br/>
`cppcoreguidelines-pro-type-member-init`<br/>
`cppcoreguidelines-pro-type-static-cast-downcast`<br/>
`cppcoreguidelines-prefer-member-initializer`<br/>
`cppcoreguidelines-misleading-capture-default-by-value`<br/>
`bugprone-argument-comment.CommentBoolLiterals`<br/>
`cert-err33-c`<br/>
`google-readability-namespace-comments`<br/>
`cppcoreguidelines-special-member-functions`<br/>

To be evaluated (need to enable explicitly).

`portability-std-allocator-const`<br/>

Only necessary for code which is exclusively compiled with `libc++`. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`modernize-deprecated-ios-base-aliases`<br/>

Warns about aliases which are removed in C++20. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`bugprone-unchecked-optional-access`<br/>

We are not using any `optional` implementation. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`modernize-replace-auto-ptr`<br/>

Still available until C++17. It is unlikely such code will ever be introduced. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`readability-identifier-naming`<br/>

We are currently using our own `naming.json` to enforce naming schemes. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`portability-simd-intrinsics`<br/>

We are not using SIMD instructions and it suggests to use `std::experiemental::` features which might not be commonly available. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.
