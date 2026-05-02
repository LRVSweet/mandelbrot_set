# CLAUDE.md — Mandelbrot Set Visualizer

## Required Reading

Before beginning any task, read `ARCHITECTURE.md` in full. All implementation decisions
must be consistent with the architecture documented there. If a task appears to conflict
with `ARCHITECTURE.md`, stop and raise the conflict rather than proceeding.

---

## Project Context

A portfolio-quality Mandelbrot set explorer targeting demonstrated HPC thought and
AI-enhanced development (AI-DVL) competency. See `ARCHITECTURE.md` for full objectives,
tech stack, and component specifications.

---

## Development Workflow

### TDD — Strictly Observed

Follow red-green-refactor without exception:

- **Red:** Write the test first. Confirm it fails for the right reason — an assertion
  failure, not a build error. A build error means the test is not yet valid.
- **Green:** Write the minimum code to make that one test pass. No more.
- **Refactor:** Clean up to meet style standards. Rerun tests. No regressions permitted.

Do not write implementation code before a failing test exists for it. Do not write
multiple tests and then batch-implement — one test, one implementation cycle.

### Commits

Commit after each discrete task: after CMake configures cleanly, after each new failing
test, after each green phase, after each refactor. Commit messages state what changed
and why, not just what files were touched.

---

## Code Style

### General Principle

Apply Clean Code standards throughout the codebase. The single documented exception is
the escape-time hot loop inside `ICalculationEngine` implementations — see the
**Hot Path Exception** section below. Outside that loop, there are no performance
justifications for violating Clean Code. If you believe a deviation is warranted
somewhere other than the hot path, stop and raise it for discussion rather than
proceeding.

### Functions

- Functions do one thing, only that thing, and do it well.
- All functions operate at exactly one level of abstraction.
- Control structure bodies (`if`, `else`, `for`, `while`, `try`, `catch`) contain
  exactly one line — a call to a named helper, a return, or a single expression.
  Multi-line control bodies are a style violation.
- Error handling is a job. A function that handles errors does nothing else.

**Incorrect — multi-line control body:**
```cpp
void processResult(const StableIterationsContainer& counts) {
    if (counts[0][0] > 100) {
        auto adjusted = counts[0][0] * 1.5;
        logHighValue(adjusted);
        applyToBuffer(adjusted);
    }
}
```

**Correct — single call per control body:**
```cpp
void processResult(const StableIterationsContainer& counts) {
    if (_isHighIterationPoint(counts, 0, 0)) {
        _applyHighValueProcessing(counts, 0, 0);
    }
}
```

### Naming

- Names are descriptive. Long descriptive names are preferred over short ambiguous ones.
- Prioritize solution-domain names (computing and mathematics terminology).
- Use problem-domain names only when no solution-domain equivalent is adequately
  descriptive.
- Private members use a leading underscore: `_visualizationRange`, `_maxIterations`.
- Private helper methods use a leading underscore: `_computePointIterations()`.

### File and Section Organization — Step-Down Rule

Public interface first, private helpers below, each helper placed directly under the
function that calls it. Delineate sections with comment headers:

```cpp
//==============================================================
// SerialCalculationEngine
//==============================================================

StableIterationsContainer SerialCalculationEngine::calculateStableIterations(
    VisualizationRange range) {
    // delegates to helpers
}

//--------------------------------------------------------------
// SerialCalculationEngine - Helpers
//--------------------------------------------------------------

uint32_t SerialCalculationEngine::_computePointIterations(
    std::complex<double> c) {
    // escape-time math — see Hot Path Exception below
}
```

### Constants and Magic Values

No magic numbers in code. All constants are `constexpr` values defined in a dedicated
`Constants.hpp` or as `static constexpr` members of the relevant class.

### Memory Ownership

- Prefer value semantics. Pass and return by value; rely on move semantics for
  efficiency. Do not pessimize with unnecessary copies.
- Polymorphic ownership uses `std::unique_ptr<IInterface>`. Never hold a raw owning
  pointer to an interface type.
- No raw `new` or `delete`. If you are about to write either, find the RAII equivalent.

### Documentation

All public classes, methods, and non-trivial private helpers carry a Doxygen comment:

```cpp
/**
 * @brief Computes stable Mandelbrot iteration counts for the specified region.
 *
 * @param range  The region of the complex plane to visualize, including pixel
 *               dimensions and coordinate bounds.
 * @return       A fully populated StableIterationsContainer. Returned by value;
 *               move semantics apply.
 */
StableIterationsContainer calculateStableIterations(VisualizationRange range);
```

### Error Handling

- Never fail silently.
- Throw descriptive exceptions. Prefer custom exception classes that carry
  structured context. Derive all custom exceptions from `std::runtime_error`
  or `std::logic_error` so they participate in the standard catch hierarchy.
  Use standard library types only for simple, unambiguous cases where a
  custom class would add no additional clarity.
- Use `assert()` for internal invariants in debug builds.

---

## Hot Path Exception

The escape-time iteration loop inside `ICalculationEngine` implementations is the
single documented exception to the Clean Code rules above. This loop executes millions
of times per frame (up to ~2M at 1920×1080). The following deviations are permitted
here, and only here:

- Iteration math may be expressed as a dense block rather than extracted into named
  sub-helpers, if extraction would prevent compiler inlining or auto-vectorization.
- Any deviation from standard style in this block must be preceded by a comment of
  the form: `// HPC: <reason for deviation>`.

**Procedure:** Start with the clean, extracted version. Only collapse it if benchmarking
produces measured evidence that the abstraction is a bottleneck. Document that evidence
in `DEVELOPMENT.md` before making the change.

---

## Test Style (Catch2 v3 with Trompeloeil)

### Structure

Map your JS testing instincts to Catch2 as follows:

| JS (Mocha) | C++ (Catch2) |
|---|---|
| `describe('MyClass')` | `TEST_CASE("MyClass", "[MyClass]")` |
| `describe('methodName()')` | `SECTION("methodName()")` |
| `context('given ...')` | `SECTION("given ...")` (nested) |
| `it('should ...')` | `SECTION("should ...")` (innermost) |
| `expect(x).to.equal(y)` | `CHECK(x == y)` |

**One `CHECK()` per innermost `SECTION`.** No exceptions.

Use `REQUIRE` instead of `CHECK` only when testing that an exception is thrown —
`REQUIRE_THROWS_AS(expr, ExceptionType)` — since the throw itself is what terminates
the test path.

### Example

```cpp
TEST_CASE("StableIterationsContainer", "[StableIterationsContainer]") {

    SECTION("operator[][]") {

        SECTION("given a valid 2D index") {

            SECTION("returns the correct iteration count for the first element") {
                StableIterationsContainer container(/* ... */);
                CHECK(container[0][0] == 10);
            }

            SECTION("returns the correct iteration count for a mid-row element") {
                StableIterationsContainer container(/* ... */);
                CHECK(container[0][1] == 20);
            }
        }

        SECTION("given an out-of-range index") {

            SECTION("throws std::out_of_range") {
                StableIterationsContainer container(/* ... */);
                REQUIRE_THROWS_AS(container[99][99], std::out_of_range);
            }
        }
    }
}
```

### Correctness Reference Points

Use these known Mandelbrot values as ground truth in calculation engine tests:

| Point | Expected result |
|---|---|
| `c = 0 + 0i` | Never escapes — returns `maxIterations` |
| `c = 2 + 0i` | Escapes on iteration 1 |
| `c = -2 + 0i` | Escapes on iteration 1 |
| `c = -0.5 + 0i` | Stable — returns `maxIterations` |

### Test File Organization

Each class gets its own test file: `SerialCalculationEngineTests.cpp`,
`StableIterationsContainerTests.cpp`, etc. Test files live in `tests/`. Each file
includes only the header(s) it is directly testing.

### Mocking

Use Trompeloeil for all mocking. Its `mock_interface<T>` template is designed for
pure-virtual abstract base classes — exactly the pattern this architecture uses.
Do not use FakeIt.

---

## What Claude Code Must Not Do Without Explicit Discussion

- **Do not implement any deferred component** — no `ParallelCalculationEngine`, no
  `SFMLUserInterface`, no `IColorScheme` implementations, no benchmark harness — unless
  explicitly instructed.
- **Do not add dependencies** not listed in `ARCHITECTURE.md` without proposing them
  first and receiving explicit approval.
- **Do not skip the red phase.** Never write implementation code before the failing
  test exists. A test that does not yet compile does not count as red — it must compile
  and fail an assertion.
- **Do not use raw `new` or `delete`.**
- **Do not use `auto` to obscure a semantically important type.** Use `auto` for
  iterator boilerplate and obvious initializations (`auto x = std::make_unique<T>(...)`).
  Be explicit for interface types and non-obvious return types.
- **Do not collapse `.hpp`/`.cpp` into a single header-only file** without explicit
  discussion. Separation is the default.
- **Do not write multi-line control structure bodies.** If a body needs more than one
  line, extract a named helper.
- **Do not silently swallow exceptions** or return sentinel error values (`-1`,
  `nullptr`) where a thrown exception is the correct response.
- **Do not deviate from the Hot Path Exception rule.** Dense, unexplained math outside
  the designated hot loop is a style violation, not an optimization.
- **Do not alter `ARCHITECTURE.md`** during a coding task. Architectural changes are
  design decisions made in consultation, not implementation details.
