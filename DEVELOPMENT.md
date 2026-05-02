# DEVELOPMENT LOG

## Description
This log contains my day-to-day decisions regarding the development of this application.

---

## 2026-04-26 — Architecture Design Session

Conducted a full architectural design session using Claude Code, starting from an initial `ARCHITECTURE.md` draft. Claude acted as a senior HPC engineer reviewer, identifying gaps and proposing improvements across multiple rounds of dialogue.

Key AI contributions: flagged missing `maxIterations` parameter, recommended `std::complex<double>` over anonymous tuples, identified `StableIterationsContainer` memory layout as a critical HPC concern, surfaced the need for Performance Requirements and Parallelism Strategy sections, and researched FakeIt vs. Trompeloeil compatibility.

Key user decisions: accepted `std::complex<double>` after agreeing it honored the problem domain's language; pushed back on AI's initial `IColorScheme` stub by questioning whether `maxIterations` should be in the interface contract, leading to a refined two-parameter `getColor()` signature; questioned the necessity of an explicit constructor on `IColorScheme`, prompting clarification that the virtual destructor is the correct enforcement mechanism; pivoted from FakeIt to Trompeloeil based on AI research confirming fragile Catch2 v3 integration in FakeIt.

Output: fully revised `ARCHITECTURE.md` covering tech stack, performance requirements, parallelism strategy, memory model, coordinate conventions, testing strategy, and data flow.

---

## 2026-05-02 — StableIterationsContainer TDD Implementation

Completed the `StableIterationsContainer` class via strict TDD, including a discrete planning phase before implementation. Also resolved a pending design question on `StableIterationsContainer`'s access interface that had been deferred from the previous session.

**Design decision — `.at(row, col)` vs `operator[][]`:** User asked Claude to stress-test the `operator[][]` approach that had been informally agreed upon. Claude identified that C++20's deprecated comma-operator form of `operator[]` requires either a proxy object (adding a domain-meaningless type) or a raw-pointer return from the outer `operator[]` (losing column-level bounds checking). `.at(row, col)` avoids both problems, maps naturally to `std::vector::at()` for bounds semantics, and is equally idiomatic. User accepted the recommendation; `ARCHITECTURE.md` and `CLAUDE.md` were updated accordingly.

**Key AI contributions:** identified `operator[][]` as requiring a domain-meaningless proxy type or unchecked column access in C++20; proposed `.at(row, col)` as the idiomatic alternative; designed the five-cycle TDD plan including a const-ref aliasing trick to guarantee a genuine assertion failure for the `at()` round-trip red phase (writing through the mutable overload and reading back through a `const` reference, whose stub returns `0u` unconditionally); implemented row-major indexing (`row * _width + col`), private helpers `_isOutOfRange()` and `_outOfRangeMessage()` to satisfy the single-line control body rule, and zero-initialization via `std::vector` default construction.

**Key user directions / corrections:** pushed back on an AI-proposed change to the const overload's return type — Claude had changed `unsigned int` (by value) to `const unsigned int&` on the grounds of STL convention, but the user challenged the reasoning; Claude then explained that `std::vector::at()` returns `const T&` because it is a generic template that cannot know the size of `T`, whereas `StableIterationsContainer::at()` returns a known 4-byte type for which returning by value is cheaper (one register vs. an 8-byte pointer and a dereference) and semantically cleaner; the change was reverted. User also requested a discrete planning phase before each implementation task, establishing a preferred workflow for subsequent sessions.

Output: `include/StableIterationsContainer.hpp`, `src/StableIterationsContainer.cpp`, `tests/StableIterationsContainerTests.cpp`; updates to root `CMakeLists.txt` and `tests/CMakeLists.txt`. Five commits, one per TDD cycle. All tests green.