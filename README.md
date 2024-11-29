# Nitrate Standard Library

The Nitrate Standard Library is a comprehensive collection of packages that provide a wide range of functionality, primarily **infrastructure code**, for Nitrate programs. The library is designed to be easy to use and to provide a convenient interface for most programs, ensuring a secure and reliable development environment. 

## Library components

Unless otherwise specified, all functions in the standard library are **thread-safe and reentrant**. In many cases, functions are **idempotent**.

The standard library is divided into several packages, each implementing a different stack layer. The packages are listed in the order of lowest level to highest level:

- [`libstd-impl`](libstd-impl/README.md): **Platform-specific functionality**, including file system access, networking, and more. Many components of the other libraries have an 'implementation function setter' that, in the absence of override, will panic when called. The platform library sets some of these to actual implementations, enabling a testable, modular, and platform-independent design of `libstd-bare`, `libstd-core`, and `libstd-main`.
- [`libstd-bare`](libstd-bare/README.md): The **minimal language features**, including slices, reflection, format strings, a general allocator, and more.
- [`libstd-core`](libstd-core/README.md): Performant implementations of common **data structures and algorithms**, as well as some universal utility functions like deterministic random number generation, logging, hash functions, and more.
- [`libstd-main`](libstd-main/README.md): A modular suite of **higher-level functionality**, including a generic I/O abstraction, many parsing and serialization utilities (JSON, XML, TOML, and others), cryptography, regular expressions, and much more.
