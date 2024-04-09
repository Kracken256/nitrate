---
layout: page
title: Composite Types
permalink: /docs/composites/
parent: Documentation
has_children: true
---

# QUIX Composite Types

In the QUIX programming language, composite types play a crucial role in organizing and managing data effectively. These types offer diverse features tailored to different programming scenarios, providing developers with flexibility and efficiency. Let's delve deeper into each of the QUIX composite types:

- [**Struct:**](/docs/composites/struct) Structs in QUIX maintain the order of their members, similar to structures in C-like languages. They provide a straightforward way to group related data elements into a single unit. While member order is preserved, QUIX allows for the insertion of padding between members for memory alignment purposes, optimizing memory usage and access efficiency.

    A struct definition in QUIX follows the syntax:

    ```quix
    struct MyStruct {
        pub member1: u8 [= <const_expr>],
        member2: u16 [= <const_expr>],
        member3: u32 [= <const_expr>],
    }
    ```

    Some things to note:
    1. QUIX structures can not contain more then 18446744073709551615 immediate members. This limit does not include members of members.
    1. All members of a struct are private by default. To make a member public, use the `pub` keyword.
    1. Members can be initialized with a constant expression using the `=` operator. This feature is useful for setting default values for struct members. If a member is not initialized, what happens depends on the compiler settings. It is possible to compile with the `-fdefaults` flag to initialize all members to zero. Otherwise, accessing an uninitialized member will result in undefined behavior.
    1. QUIX structs may contain other structs, groups, classes, regions, and unions and primitive types as members. This feature allows for the creation of complex data structures with nested elements.
    1. QUIX structs can not contain recursive references. This restriction ensures that the size of a struct can be determined at compile time.
    1. Use the `sizeof` operator to determine the size of a struct in **bytes**.
    1. Use the `bitcount()` function to determine the size of a struct in **bits** taking into account bitfields.


- [**Group:**](/docs/composites/group) QUIX groups offer a more optimized approach compared to structs. Unlike structs, member order is not preserved in groups, allowing the compiler to optimize the layout for better memory utilization. Padding may or may not be inserted between members to align data properly. Despite potential variations in memory layout, QUIX guarantees ABI correctness at link time through mangled symbols, ensuring compatibility across different modules.

    ```quix
    group MyGroup {
        pub member1: u8 [= <const_expr>],
        member2: u16 [= <const_expr>],
        member3: u32 [= <const_expr],
    }
    ```

    Some things to note:
    1. QUIX groups can not contain more then 18446744073709551615 immediate members. This limit does not include members of members.
    1. All members of a group are private by default. To make a member public, use the `pub` keyword.
    1. Members can be initialized with a constant expression using the `=` operator. This feature is useful for setting default values for group members. If a member is not initialized, what happens depends on the compiler settings. It is possible to compile with the `-fdefaults` flag to initialize all members to zero. Otherwise, accessing an uninitialized member will result in undefined behavior.
    1. QUIX groups may contain other structs, groups, classes, regions, and unions and primitive types as members. This feature allows for the creation of complex data structures with nested elements.
    1. QUIX groups can not contain recursive references. This restriction ensures that the size of a group can be determined at compile time.
    1. Use the `sizeof` operator to determine the size of a group in **bytes**.
    1. Use the `bitcount()` function to determine the size of a group in **bits** taking into account bitfields.


- [**Class:**](/docs/composites/class) Drawing inspiration from C++ classes, QUIX classes provide a versatile tool for object-oriented programming. In addition to data storage capabilities similar to structs and groups, classes in QUIX can encapsulate methods, constructors, and destructors. This versatility enables developers to create complex data structures and implement sophisticated behavior within their programs.

    **docs coming soon**

- [**Region:**](/docs/composites/region) Regions in QUIX represent contiguous binary packed structures with precise control over memory layout. These structures have no padding inserted between members, and member order is preserved exactly as defined by the programmer. This feature ensures that what you define is precisely what you get in memory, making regions ideal for scenarios where memory layout precision is crucial. When combined with Bitfields, regions excel in implementing binary packet protocols, offering efficient memory usage and streamlined data transmission.

    ```quix
    region MyRegion {
        pub member1: u8 [= <const_expr>],
        member2: u16 [= <const_expr>],
        member3: u32 [= <const_expr],
    }
    ```

    Some things to note:
    1. QUIX regions can not contain more then 18446744073709551615 immediate members. This limit does not include members of members.
    1. All members of a region are private by default. To make a member public, use the `pub` keyword.
    1. Members can be initialized with a constant expression using the `=` operator. This feature is useful for setting default values for region members. If a member is not initialized, what happens depends on the compiler settings. It is possible to compile with the `-fdefaults` flag to initialize all members to zero. Otherwise, accessing an uninitialized member will result in undefined behavior.
    1. QUIX regions may contain other structs, groups, classes, regions, and unions and primitive types as members. This feature allows for the creation of complex data structures with nested elements.
    1. QUIX regions can not contain recursive references. This restriction ensures that the size of a region can be determined at compile time.
    1. Although the `sizeof` operator is defined for regions, it is not recommended to use it. The size of a region is not guaranteed to be a multiple of 8 bits, making it unsuitable for some calculations. Use the `bitcount()` function to determine the size of a region in **bits** taking into account bitfields. Of course, the size of a region in memory will be a multiple of 8 bits, but QUIX enables more precise control giving the abstraction that the minimum unit of memory is the bit.
    1. Use the `bitcount()` function to determine the size of a group in **bits** taking into account bitfields.

- [**Union:**](/docs/composites/union) QUIX unions provide a low-level construct for managing overlapping memory regions, akin to C unions. In unions, different data members share the same memory space, allowing for efficient memory utilization. However, QUIX unions lack type safety and member selection mechanisms, requiring careful handling to prevent unintended memory overlaps and ensure data integrity. Despite their low-level nature, unions offer valuable flexibility for scenarios where memory optimization is paramount.

    ```quix
    union MyUnion {
        pub member1: u8,
        member2: u16,
        member3: u32,
    }
    ```

    Some things to note:
    1. QUIX unions can not contain more then 18446744073709551615 immediate members. This limit does not include members of members.
    1. All members of a union are private by default. To make a member public, use the `pub` keyword.
    1. Members in a union are initialized to 0 by default. QUIX does not support initializing union members with constant expressions. The initial value of a union is behaves like `memset(&_union, 0, sizeof(_union))` in C.
    1. QUIX unions may contain other structs, groups, classes, regions, and unions and primitive types as members. This feature allows for the creation of complex data structures with nested elements.
    1. QUIX unions can not contain recursive references. This restriction ensures that the size of a union can be determined at compile time.
    1. Use the `sizeof` operator to determine the size of a union in **bytes**.
    1. Use the `bitcount()` function to determine the size of a union in **bits** taking into account bitfields.