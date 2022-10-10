# clang-tidy

Below are the reasoning why certain checks are (currently) disabled for out code base

## Externals

We do not perform static analysis of the source of the external libraries. `simplecpp` has its own CI with a clang-tidy workflow.

## Disabled Checks

`abseil-*`<br>
`altera-*`<br>
`android-*`<br>
`boost-*`<br>
`darwin-*`<br>
`fuchsia-*`<br>
`linuxkernel-*`<br>
`llvm-*`<br>
`llvmlibc-*`<br>
`mpi-*`<br>
`objc-*`<br>
`openmp-*`<br>
`zircon-*`<br>

These are disabled since the platforms/libraries in question are not targeted by us.

`cert-*`<br>
`cppcoreguidelines-*`<br>
`google-*`<br>
`hicpp-*`<br>

These are coding guidelines we do not follow. Some of the checks might be explicitly enabled though.

`readability-braces-around-statements`<br>
`readability-isolate-declaration`<br>
`modernize-use-trailing-return-type`<br>
`modernize-use-auto`<br>
`readability-uppercase-literal-suffix`<br>
`readability-else-after-return`<br>
`modernize-use-default-member-init`<br>
`readability-identifier-length`<br>

These do not relect the style we are (currently) enforcing.

`readability-function-size`<br>
`readability-function-cognitive-complexity`<br>

We are not interesting in the size/complexity of a function.

`readability-magic-numbers`<br>
`readability-redundant-member-init`<br>
`readability-simplify-boolean-expr`<br>

These do not (always) increase readability.

`bugprone-macro-parentheses`<br>
`readability-implicit-bool-conversion`<br>

To be documented.

`performance-faster-string-find`<br>
`bugprone-narrowing-conversions`<br>
`performance-no-automatic-move`<br>

It was decided not to apply these.

`modernize-use-equals-default`<br>
`modernize-loop-convert`<br>

These might change the behavior of code which might not be intended (need to file an upstream issue)

`modernize-raw-string-literal`<br>

This leads to a mismatch of raw string literals and regular ones and does reduce the readability.

`readability-convert-member-functions-to-static`<br>

Disabled because of false positives with Qt `slot` methods (see https://github.com/llvm/llvm-project/issues/57520).

`-clang-analyzer-*`<br>

Disable because of false positives (needs to file an upstream bug report).

`misc-non-private-member-variables-in-classes`<br>

We actively use this.

`misc-no-recursion`<br>

Leads to lots of "false positives". This seem to enforce a coding guidelines of certain codebases.

`readability-use-anyofallof`<br>

We currently don't even apply our own `useStlAlgorithm` findings.

`bugprone-easily-swappable-parameters`<br>

This produces a lot of noise and they are not fixable that easily.

`readability-container-data-pointer`<br>

Disable because of false positives and inconsistent warnings (need to file an upstream bug report).

`misc-const-correctness`<br>

Work in progress.

`bugprone-assignment-in-if-condition`<br>

Is reported for valid patterns we are using.

`readability-suspicious-call-argument`<br>

Produces a lot of false positives since it is too vague in its analysis.

`performance-inefficient-string-concatenation`<br>

Produces many warnings which very much look like false positives (needs to be reported upstream).

`modernize-avoid-c-arrays`<br>
`readability-container-size-empty`<br>
`bugprone-branch-clone`<br>
`readability-const-return-type`<br>
`modernize-return-braced-init-list`<br>
`misc-throw-by-value-catch-by-reference`<br>
`readability-avoid-const-params-in-decls`<br>
`bugprone-signed-char-misuse`<br>
`readability-redundant-access-specifiers`<br>
`performance-noexcept-move-constructor`<br>
`concurrency-mt-unsafe`<br>

To be evaluated.

`portability-std-allocator-const`<br>

Only necessary for code which is exclusively compiled with `libc++`. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`modernize-deprecated-ios-base-aliases`<br>

Warns about aliases which are removed in C++20. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`bugprone-unchecked-optional-access`<br>

We are not using any `optional` implementation. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`modernize-replace-auto-ptr`<br>

Still available until C++17. It is unlikely such code will ever be introduced. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`readability-identifier-naming`<br>

We are currently using our own `naming.json` to enforce naming schemes. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.

`portability-simd-intrinsics`<br>

We are not using SIMD instructions and it suggests to use `std::experiemental::` features which might not be commonly available. Also disabled for performance reasons - see https://github.com/llvm/llvm-project/issues/57527#issuecomment-1237935132.
