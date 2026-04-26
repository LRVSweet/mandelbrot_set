## Project Objective
Develop a clean, efficient Mandelbrot set explorer with support for panning, zooming, and interchangeable coloring schemes.

---

## Tech Stack

| Concern | Choice | Notes |
|---|---|---|
| Language standard | C++20 | Minimum C++17; C++20 preferred for `std::span`, concepts |
| Build system | CMake | Required for Catch2 v3; standard for SFML and OpenMP integration |
| Package manager | vcpkg | CMake integration; manages SFML, Catch2, Trompeloeil |
| Visualization | SFML 3 | Breaking API changes from SFML 2 — pin to v3 |
| Testing | Catch2 v3 | No longer header-only; requires CMake integration |
| Mocking | Trompeloeil | First-class Catch2 v3 support; `mock_interface<T>` designed for pure-virtual ABCs |
| Parallelism | OpenMP | Compiler-level; no separate package required |

### Why Trompeloeil Over FakeIt
FakeIt hooks into Catch2 internal APIs and has broken twice with Catch2 point releases. Trompeloeil uses only stable public Catch2 macros and has a `mock_interface<T>` template purpose-built for mocking pure-virtual abstract base classes — exactly the pattern this architecture uses.

---

## Performance Requirements

| Requirement | Target |
|---|---|
| Maximum resolution | 1920x1080 (with support for lower resolutions via window resizing) |
| Target frame rate | 30–60fps (30fps floor for high iteration counts; 60fps target for shallow depths) |
| Recalculation latency | ≤33ms per frame at 1920x1080 for iteration counts ≤1,000 |
| Iteration count range | 256 (default) to 10,000 (v1 upper limit) |

**Note**: Latency will degrade gracefully above 1,000 iterations. The 33ms target is the benchmark for the parallel CPU engine. GPU acceleration is a post-v1 consideration.

---

## Architecture

### 3-Layer Architecture

| Layer | Responsibilities | Tools Used | Notes |
|---|---|---|---|
| Layer 3: User Interface | Convert iteration counts to a graphically navigable visualization; capture user input | SFML / Terminal | Terminal-based implementation used as POC during development |
| Layer 2: Coordination | Coordinate actions between the logic and UI layers; own the game loop | Standard C++ | Clean Code |
| Layer 1: Logic | Produce a `StableIterationsContainer` across a specified region of the complex plane | Standard C++ | Clean Code architecture with a systems-friendly hot loop |

### Architectural Components

- **`int main()`**: Bootstraps dependencies and calls `Coordinator::run()`.
- **`Coordinator`**: Owns the game loop. Receives an `IUserInterface` and `ICalculationEngine` via dependency injection. Translates user input (pan/zoom) into `VisualizationRange` updates, requests recalculation, and drives the UI.
- **`IUserInterface`**: Receives a `StableIterationsContainer` and renders it. Responsible for caching rendered frames and detecting user input. Also receives an `IColorScheme` via dependency injection.
- **`ICalculationEngine`**: Computes stable Mandelbrot iteration counts for a given `VisualizationRange`. The hot loop lives here. `maxIterations` is engine-level configuration set at construction.

### Concrete Implementations

| Interface | Implementation | Purpose |
|---|---|---|
| `IUserInterface` | `TerminalUserInterface` | POC — ASCII/character-based rendering |
| `IUserInterface` | `SFMLUserInterface` | Primary — graphical rendering via SFML |
| `ICalculationEngine` | `SerialCalculationEngine` | Baseline — single-threaded; used for correctness validation |
| `ICalculationEngine` | `ParallelCalculationEngine` | HPC target — OpenMP-parallelized hot loop |
| `IColorScheme` | *(TBD)* | Interchangeable color mapping strategies |

### Open Design Questions
- *(none)* — The Coordinator does **not** need separate implementations per `IUserInterface` type. The `IUserInterface` abstraction handles all concrete UI variants.

---

## Parallelism Strategy

The Mandelbrot set is an **embarrassingly parallel** problem: each pixel's iteration count is computed independently of every other pixel. There are no data dependencies across pixels, no shared mutable state, and no synchronization required during computation.

Parallelism lives exclusively inside `ICalculationEngine` implementations. The interfaces above the engine remain synchronous and single-threaded.

### Planned Engine Roadmap

1. **`SerialCalculationEngine`**: Single-threaded baseline. Used to establish correctness via TDD before any optimization.
2. **`ParallelCalculationEngine`**: OpenMP-parallelized. A `#pragma omp parallel for` over the pixel loop is sufficient to exploit the embarrassingly parallel structure. This is the v1 HPC target.
3. **GPU Engine** *(post-v1)*: CUDA or OpenCL implementation. Separate `ICalculationEngine` concrete class.

### Why OpenMP
OpenMP requires minimal code change over the serial implementation (one pragma, one compiler flag), makes the parallelism strategy legible, and is the industry standard for this class of problem. It also makes serial-vs-parallel benchmarking straightforward since the only variable is the pragma.

### Async / Progressive Rendering *(post-v1)*
The current architecture uses a synchronous game loop. At high iteration counts, `calculateStableIterations()` will block for longer than one frame period. The designed upgrade path is to change `calculateStableIterations()` to return `std::future<StableIterationsContainer>`, allowing the Coordinator to display the previous frame while computation runs on a background thread. This is a localized interface change and is deferred to post-v1.

---

## Memory Model

### `StableIterationsContainer` Layout
The backing store is a **flat `std::vector<unsigned int>`** with **row-major layout**. The public `[row][col]` operator is an abstraction over `data[row * width + col]`. This ensures contiguous memory access during sequential iteration, which is critical for cache performance in the hot loop and the rendering layer.

A `vector<vector<unsigned int>>` layout is explicitly prohibited — it produces per-row heap allocations and pointer indirection that thrashes the cache.

### Ownership and Lifetime
- `calculateStableIterations()` returns `StableIterationsContainer` **by value**. Move semantics ensure no deep copy occurs.
- The caller (Coordinator) owns the returned container.
- `ICalculationEngine` implementations should **pre-allocate** their internal buffer at construction time (sized to `maxResolution`) and reuse it across frames to avoid per-frame heap allocation.

### Caching Strategy
- **UI layer** (`IUserInterface`): Caches the rendered frame (e.g., SFML sprites/textures). Used by `repeatLastVisualization()` when no pan or zoom has occurred.
- **Engine layer** (`ICalculationEngine`): May cache iteration data internally. Simple exact-match caching (same `VisualizationRange` → return cached result) can be implemented as a decorator. Region-aware caching (reusing overlapping pixels across pan/zoom) requires internal state and must be a dedicated `ICalculationEngine` implementation, not a decorator.

---

## Coordinate System and Default View

### Pixel-to-Complex Mapping
- Row 0 = top of screen = **maximum imaginary value**
- Column 0 = left of screen = **minimum real value**
- Imaginary value decreases as row index increases.
- Real value increases as column index increases.

### Default View
The default view is centered at **(-0.75, 0)** in the complex plane, which frames the entire Mandelbrot set aesthetically. At 1920x1080:
- Real axis range: **[-2.5, 1.0]** (width: 3.5)
- Imaginary axis range: **[-0.984, 0.984]** (height scaled to maintain square pixels on a 16:9 display)

Implementations must maintain a **1:1 pixel aspect ratio** (square pixels) across all zoom levels and resolutions. The `VisualizationRange` is always derived from a center point and zoom level to enforce this invariant.

---

## Testing Strategy

All components follow a strict **Test-Driven Development (TDD)** flow. Tests are written before implementation.

### Correctness Reference Points
The Mandelbrot set has analytically known values that serve as ground truth for testing the calculation engine:

| Point | Expected result |
|---|---|
| `c = 0 + 0i` | Never escapes — returns `maxIterations` |
| `c = 2 + 0i` | Escapes on iteration 1 |
| `c = -2 + 0i` | Escapes on iteration 1 |
| `c = -0.5 + 0i` | Stable — returns `maxIterations` (on the real axis inside the set) |

### Testing Seams
The architecture creates natural unit test boundaries:
- `StableIterationsContainer`: Test `[row][col]` access, bounds behavior, and memory layout invariants.
- `SerialCalculationEngine`: Test against reference correctness points above. This is the correctness baseline for all other engine implementations.
- `ParallelCalculationEngine`: Must produce **identical output** to `SerialCalculationEngine` for all inputs. Tested by direct comparison.
- `Coordinator`: Test game loop logic (pan/zoom → `VisualizationRange` calculation) with mock `IUserInterface` and `ICalculationEngine` implementations.
- `IUserInterface` implementations: Integration-tested; unit tests focus on input detection logic.

---

## Data Flow — Single Frame Cycle

The following describes a **pan event** as a representative example of the full frame cycle:

1. `Coordinator::run()` calls `ui.visualizationTerminationRequested()` — returns false, loop continues.
2. `Coordinator` calls `ui.getPanVector()` — returns e.g. `{1.0, 0.0}` (pan right).
3. `Coordinator` calls `ui.getZoomFactor()` — returns `1.0` (no zoom change).
4. `Coordinator` calculates a new `VisualizationRange` by applying the pan vector to the current range, maintaining the 1:1 pixel aspect ratio invariant.
5. `Coordinator` calls `engine.calculateStableIterations(newRange)` — **blocks** until complete.
6. Engine returns a `StableIterationsContainer` (moved, not copied).
7. `Coordinator` calls `ui.displayVisualization(container)` — UI renders the frame and caches it.
8. Loop returns to step 1.

**No pan/zoom case**: Steps 2–3 return neutral values (`{0,0}` and `1.0`). `Coordinator` skips recalculation and calls `ui.repeatLastVisualization()` instead.

---

## Special Types

### `class StableIterationsContainer`
Container for the number of stable Mandelbrot iterations at each pixel in the visualization.

- **Backing store**: Flat `std::vector<unsigned int>`, row-major layout
- **Access**: `operator[row][col]` implemented as `data[row * width + col]`
- **Ownership**: Returned by value from `calculateStableIterations()`; move semantics apply

### `struct VisualizationRange`
Defines the region of the complex plane to visualize.

| Field | Type | Description |
|---|---|---|
| `horizontalPoints` | `unsigned int` | Number of pixels along the horizontal axis |
| `verticalPoints` | `unsigned int` | Number of pixels along the vertical axis |
| `topLeft` | `std::complex<double>` | Top-left coordinate of the visualization region |
| `bottomRight` | `std::complex<double>` | Bottom-right coordinate of the visualization region |

---

## Interfaces

### `IColorScheme`
#### Responsibility
Map a stable iteration count to a display color.

#### Public Methods
- **`virtual ~IColorScheme() = default`**: Virtual destructor — required for safe polymorphic destruction.
- **`virtual sf::Color getColor(unsigned int stableIterations, const unsigned int maxIterations) const = 0`**: Returns the display color for a given iteration count. Both parameters are passed by value — `unsigned int` is cheaper to pass by value than by reference. `maxIterations` is included in the interface contract because no meaningful color mapping exists without knowledge of the iteration range. `const` enforces that color schemes are stateless (same inputs always produce the same output).

#### HPC Note — Virtual Dispatch
`getColor()` is called once per pixel per frame (~2M calls at 1920x1080). For a monomorphic call site (single concrete `IColorScheme` at runtime), branch prediction mitigates most overhead. The more significant constraint is that virtual dispatch blocks inlining and therefore auto-vectorization of the rendering loop.

**Known upgrade path**: When rendering performance requires it, replace the per-pixel signature with a per-buffer signature:
```cpp
virtual void getColors(std::span<const unsigned int> iterations,
                       const unsigned int maxIterations,
                       std::span<sf::Color> output) const = 0;
```
This collapses ~2M virtual calls per frame to one and allows the inner loop to be vectorized. This change is deferred until the rendering layer becomes a measured bottleneck.

---

### `IUserInterface`
#### Responsibility
Render a `StableIterationsContainer` as a visual frame and report user input (pan, zoom, termination).

#### Public Methods
- **constructor**: Receives an `IColorScheme` via dependency injection.
- **`std::vector<double> getPanVector()`**: Returns a 2-element vector indicating requested pan direction and magnitude.
  - `{0.0, 0.0}` — no pan
  - `{-1.0, 1.0}` — pan left 1 unit, up 1 unit
- **`double getZoomFactor()`**: Returns zoom multiplier relative to the previous frame.
  - `1.0` — no change
  - `2.0` — zoom in (linear dimensions halved)
  - `0.5` — zoom out (linear dimensions doubled)
- **`tuple<unsigned int, unsigned int> getScreenDimensions()`**: Returns current screen dimensions in pixels (horizontal, vertical).
- **`void repeatLastVisualization()`**: Displays the previously cached frame.
- **`void displayVisualization(StableIterationsContainer)`**: Renders a new frame and caches it.
- **`bool visualizationTerminationRequested()`**: Returns true if the user has requested to exit.

---

### `Coordinator`
#### Responsibility
Own the game loop. Translate user input into `VisualizationRange` updates, drive recalculation, and deliver results to the UI.

#### Public Methods
- **constructor**: Receives `IUserInterface` and `ICalculationEngine` via dependency injection.
- **`void run()`**: Executes the game loop:
  - On pan/zoom input: calculates new `VisualizationRange`, calls `calculateStableIterations()`, passes result to `displayVisualization()`.
  - On no input: calls `repeatLastVisualization()`.
  - On termination request: exits the loop cleanly.

---

### `ICalculationEngine`
#### Responsibility
Compute stable Mandelbrot iteration counts for a specified region of the complex plane.

#### Configuration
- **`maxIterations`**: Set at construction. Defines the upper bound on iterations before a point is considered stable. Not a per-call parameter — it is a property of the engine's rendering policy.

#### Public Methods
- **constructor**: Receives `maxIterations` and any parallelism configuration via dependency injection.
- **`StableIterationsContainer calculateStableIterations(VisualizationRange)`**: Returns a fully populated `StableIterationsContainer` for the given range. Blocks until computation is complete (synchronous in v1).
