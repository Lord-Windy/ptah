# Initial MVP

# Refresh on simulation

This is designed to be an economic, automation and procedural generation
in a space system type simulation. It is supposed to have fairly realistic
physics. Ships, space stations, factories and diplomacy is meant to occur

## Overarching Goal

Tick system representing 1 tick = 1 kilosecond

Sub-Ticks if needed

A simple space system that includes N Body physics (assuming that is
gravity). I want several planets,  a sun, a moon and some asteroids

Some basic bodies that can move around

Start of procedural generation.

### What I need to learn

Physics in any shape or form

Gravity in a big way

How to use AVX512 to help with the math

## Expected Order of Completion

### Phase 1: Core Physics Foundation
1. **Basic Vector Math Implementation**
   - Research: Linear algebra basics, 3D vector operations
   - Implement position, velocity, acceleration vectors
   - Basic coordinate systems (Cartesian for now)

2. **Gravity and N-Body Physics**
   - Research: Newton's law of universal gravitation, numerical
     integration methods (Euler, Verlet, RK4)
   - Implement two-body gravity simulation
   - Extend to N-body simulation
   - Handle numerical stability issues

### Phase 2: Simulation Framework
3. **Tick System Architecture**
   - Research: Game loops, fixed timestep vs variable timestep
   - Implement core tick system (1 tick = 1 kilosecond)
   - Add sub-tick support for physics accuracy
   - Time dilation/acceleration controls

4. **Basic Celestial Bodies**
   - Create data structures for planets, stars, moons
   - Implement orbital mechanics
   - Add asteroids with simplified physics
   - Basic collision detection

### Phase 3: Optimization
5. **AVX512 Integration**
   - Research: SIMD programming, AVX512 intrinsics, vectorization strategies
   - Identify computational bottlenecks (likely N-body calculations)
   - Implement vectorized gravity calculations
   - Benchmark and profile improvements

### Phase 4: Procedural Generation
6. **Solar System Generation**
   - Research: Procedural generation algorithms, Perlin noise,
     astronomical constraints
   - Generate realistic star systems (habitable zones, orbital resonances)
   - Asteroid belt generation
   - Planet characteristic generation (size, composition, atmosphere)

### Phase 5: Gameplay Elements
7. **Moving Bodies (Ships/Stations)**
   - Implement spacecraft physics (thrust, orbital transfers)
   - Station placement and orbital mechanics
   - Basic pathfinding in orbital space

8. **Economic Systems**
   - Resource types and distribution
   - Basic supply/demand mechanics
   - Trading routes based on orbital mechanics

9. **Automation & Factories**
   - Production chains
   - Resource extraction from celestial bodies
   - Automated cargo ships

10. **Diplomacy Framework**
    - Faction system
    - Territory/influence mechanics
    - Basic AI decision making

### Key Research Areas:
- **Orbital Mechanics**: Kepler's laws, Hohmann transfers, Lambert's problem
- **Numerical Methods**: Integration techniques for physics simulation
- **Performance**: Cache optimization, parallelization strategies
- **Procedural Generation**: Coherent noise functions, constraint satisfaction
- **Game Design**: Economic balance, progression systems
