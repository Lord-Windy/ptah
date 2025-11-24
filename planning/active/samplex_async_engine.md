# Samplex Async Engine - Vision SOP

## 1. Define the "North Star"

Build a composable async engine (Samplex) that enables predefined execution
flows through function pointer chains, solving the complexity of starting async
projects (MUDs, MMOs) by providing a solid foundation for handling connections,
I/O operations, and event-driven behaviors through well-defined Plex items with
clear error handling, cleanup, and continuation paths.

## 2. Context & Constraints

### Pre-requisites
- ✅ PlexRegistry system with thread-safe hash map and read-write locking
- ✅ Plex and PlexItem data structures with timing and lifecycle management
- ✅ SamArena memory management integration
- ✅ Basic libpq and io_uring integration verified
- ✅ SamHashMap and SamVector from samdata library

### Dependencies
- **liburing**: For io_uring-based async I/O operations
- **libpq**: For PostgreSQL async database operations
- **samrena**: Arena-based memory management (existing)
- **samdata**: Hash maps and data structures (existing)
- **pthread**: For read-write locks (existing)

### Boundaries (Out of Scope)
- Full-featured MUD/MMO game logic (this is foundation only)
- High-level protocol implementations (HTTP, WebSocket, etc.)
- Distributed systems or multi-process coordination
- Performance optimization beyond basic io_uring usage
- TLS/SSL support
- Authentication/authorization systems

## 3. Architecture Strategy

### Data Structures

**Core Components (Existing):**
```c
PlexRegistry - Thread-safe registry of Plex instances
  ├─ Hash map: id → Plex*
  ├─ Read-write lock for concurrent access
  └─ Atomic ID counter

Plex - Execution context for a logical workflow
  ├─ Vector of PlexItem* (execution chain)
  ├─ Timing information (start/stop)
  └─ Arena-backed memory

PlexItem - Atomic unit of async work
  ├─ Handler function pointers (success, error, cleanup)
  ├─ User data (void*)
  └─ Timing information
```

**New Components Needed:**
```c
PlexEventLoop - Main event loop using io_uring
  ├─ io_uring instance
  ├─ PlexRegistry reference
  ├─ Timer heap/queue for scheduled items
  └─ Running state

PlexTimer - Timer-based scheduling
  ├─ Target time (nanoseconds)
  ├─ Associated Plex + PlexItem
  └─ Recurring flag

PlexConnection - Network connection management
  ├─ File descriptor
  ├─ Associated Plex
  ├─ Read/write buffers (arena-backed)
  └─ State (connecting, connected, closing)

PlexPostgres - Async Postgres operations
  ├─ PGconn* connection
  ├─ Associated Plex
  └─ Query state
```

### Interfaces (Public API)

**Event Loop Management:**
```c
PlexEventLoop* plex_event_loop_create(PlexRegistry* registry);
void plex_event_loop_destroy(PlexEventLoop* loop);
int plex_event_loop_run(PlexEventLoop* loop);
void plex_event_loop_stop(PlexEventLoop* loop);
```

**Timer Operations:**
```c
PlexTimer* plex_timer_create(Plex* plex, PlexItem* item, int64_t delay_ns, bool recurring);
void plex_timer_cancel(PlexTimer* timer);
int plex_timer_add(PlexEventLoop* loop, PlexTimer* timer);
```

**Connection Management:**
```c
PlexConnection* plex_connection_create(Plex* plex, const char* host, uint16_t port);
void plex_connection_close(PlexConnection* conn);
int plex_connection_read(PlexConnection* conn, PlexItem* item);
int plex_connection_write(PlexConnection* conn, PlexItem* item, void* data, size_t len);
```

**Postgres Operations:**
```c
PlexPostgres* plex_postgres_create(Plex* plex, const char* conninfo);
void plex_postgres_close(PlexPostgres* pg);
int plex_postgres_query(PlexPostgres* pg, PlexItem* item, const char* query);
```

### Integration with Ptah Architecture

Samplex follows the **hexagonal/ports-and-adapters** pattern:

**Core (plex.h, plex.c):**
- PlexRegistry, Plex, PlexItem (pure business logic)
- No I/O dependencies

**Adapters (separate files):**
- `plex_uring.c` - io_uring adapter for event loop
- `plex_timer.c` - Timer scheduling adapter
- `plex_network.c` - Network I/O adapter
- `plex_postgres.c` - PostgreSQL adapter

Each adapter implements the **handler pattern** where PlexItems define
what happens on success/error, and adapters just drive the I/O.

## 4. The Breakdown (Ticket Generation)

### Phase 1: Event Loop Foundation

#### [SPX-001] Implement PlexEventLoop Core Structure

**Description:** Create the basic PlexEventLoop structure with io_uring
initialization and destruction.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_uring.c`
- File: `apps/spikes/samplex/src/plex_uring.h`
- Functions: `plex_event_loop_create()`, `plex_event_loop_destroy()`
- Includes proper Apache 2.0 license headers
- io_uring ring properly initialized with configurable queue depth
- Proper cleanup in destroy function

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-002] Implement Event Loop Run/Stop Logic

**Description:** Add the main event loop execution with io_uring
submission/completion processing.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_uring.c`
- Functions: `plex_event_loop_run()`, `plex_event_loop_stop()`
- Event loop processes io_uring CQEs and dispatches to handlers
- Graceful shutdown on stop signal
- Basic error handling for io_uring operations

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

### Phase 2: Timer Support

#### [SPX-003] Implement PlexTimer Data Structure

**Description:** Create timer structure with scheduling support (one-shot
and recurring).

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_timer.c`
- File: `apps/spikes/samplex/src/plex_timer.h`
- Struct: `PlexTimer` with target time, plex, item, and recurring flag
- Functions: `plex_timer_create()`, `plex_timer_destroy()`
- Includes proper Apache 2.0 license headers

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-004] Integrate Timers with io_uring

**Description:** Add timer scheduling to event loop using io_uring timeout
operations.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_timer.c`
- Functions: `plex_timer_add()`, `plex_timer_cancel()`
- Use io_uring `IORING_OP_TIMEOUT` for timer events
- Timer heap/priority queue for managing multiple timers
- Proper dispatch to PlexItem handlers on timer expiration

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-005] Create Timer Test Demonstration

**Description:** Build a test that creates multiple timers and verifies
execution order.

**Definition of Done:**
- File: `apps/spikes/samplex/src/main.c` (update)
- Test creates 3+ timers with different delays
- Verifies timers fire in correct order
- Tests recurring timer functionality
- Validates PlexItem handlers are called correctly

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

### Phase 3: Network Connection Support

#### [SPX-006] Implement PlexConnection Structure

**Description:** Create connection management structure with socket and
buffer handling.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_network.c`
- File: `apps/spikes/samplex/src/plex_network.h`
- Struct: `PlexConnection` with fd, plex, buffers, state
- Functions: `plex_connection_create()`, `plex_connection_destroy()`
- Buffers backed by SamArena
- Includes proper Apache 2.0 license headers

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-007] Implement Async Connect Operation
**Description:** Add non-blocking socket connection using io_uring.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_network.c`
- Function: `plex_connection_connect()`
- Uses io_uring `IORING_OP_CONNECT`
- Handles connection success/failure via PlexItem handlers
- Proper error propagation to error_handler

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-008] Implement Async Read Operation
**Description:** Add non-blocking socket read using io_uring.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_network.c`
- Function: `plex_connection_read()`
- Uses io_uring `IORING_OP_RECV` or `IORING_OP_READ`
- Reads into arena-backed buffer
- Calls PlexItem handler with received data

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-009] Implement Async Write Operation
**Description:** Add non-blocking socket write using io_uring.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_network.c`
- Function: `plex_connection_write()`
- Uses io_uring `IORING_OP_SEND` or `IORING_OP_WRITE`
- Handles partial writes correctly
- Calls PlexItem handler on completion

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-010] Create Network Echo Server Test
**Description:** Build a simple echo server demonstrating connection handling.

**Definition of Done:**
- File: `apps/spikes/samplex/src/main.c` (update)
- Creates listening socket with io_uring `IORING_OP_ACCEPT`
- Accepts connections via PlexConnection
- Echoes received data back to client
- Demonstrates PlexItem chaining (accept → read → write → read...)

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

### Phase 4: PostgreSQL Integration

#### [SPX-011] Implement PlexPostgres Structure
**Description:** Create async Postgres connection management.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_postgres.c`
- File: `apps/spikes/samplex/src/plex_postgres.h`
- Struct: `PlexPostgres` with PGconn, plex, state
- Functions: `plex_postgres_create()`, `plex_postgres_destroy()`
- Uses PQconnectStart for async connection
- Includes proper Apache 2.0 license headers

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-012] Implement Async Postgres Connect
**Description:** Add non-blocking Postgres connection using libpq async API.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_postgres.c`
- Function: `plex_postgres_connect()`
- Uses PQconnectStart/PQconnectPoll
- Monitors socket fd via io_uring for readability
- Calls PlexItem handler on connection success/failure

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-013] Implement Async Postgres Query
**Description:** Add non-blocking query execution using libpq async API.

**Definition of Done:**
- File: `apps/spikes/samplex/src/plex_postgres.c`
- Function: `plex_postgres_query()`
- Uses PQsendQuery/PQgetResult
- Monitors socket fd via io_uring
- Calls PlexItem handler with PGresult on completion
- Handles multi-statement queries correctly

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-014] Create Postgres Query Test
**Description:** Build test demonstrating async database operations.

**Definition of Done:**
- File: `apps/spikes/samplex/src/main.c` (update)
- Connects to local Postgres (with graceful skip if unavailable)
- Executes test query: `SELECT 1 + 1`
- Demonstrates PlexItem chaining (connect → query → process results)
- Validates result correctness

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

### Phase 5: Integration and Documentation

#### [SPX-015] Build Comprehensive Demo Application

**Description:** Create end-to-end demonstration combining timers,
network, and Postgres.

**Definition of Done:**
- File: `apps/spikes/samplex/src/main.c` (major update)
- Demonstrates all features:
  - Multiple timers firing at different intervals
  - Network server accepting connections
  - Postgres queries triggered by timer or connection
  - Proper Plex lifecycle management
- Clean shutdown with all resources freed
- Readable output showing execution flow

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-016] Update Build Configuration
**Description:** Ensure CMakeLists.txt properly links all new dependencies.

**Definition of Done:**

- File: `apps/spikes/samplex/CMakeLists.txt`
- All source files added (plex_uring.c, plex_timer.c, plex_network.c,
  plex_postgres.c)
- Proper dependency on liburing (uring target)
- Proper dependency on libpq (PostgreSQL::PostgreSQL)
- Builds cleanly without warnings

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

#### [SPX-017] Add Architecture Documentation
**Description:** Document the Samplex architecture and usage patterns.

**Definition of Done:**
- File: `apps/spikes/samplex/README.md` (new)
- Explains the Plex/PlexItem concept
- Shows example of defining execution flows
- Documents handler patterns (success, error, cleanup)
- Includes simple code examples
- Explains when to use Samplex vs other approaches
- Includes proper Apache 2.0 license header

**Next Step:** Reference `planning/implementation_sop.md` for execution.

---

## 5. Deliverable

This planning document has been created at
`planning/active/samplex_async_engine.md`.

**Total Tickets:** 17 tickets across 5 phases

**Execution Order:**
1. Start with Phase 1 (Event Loop Foundation) - tickets SPX-001 to SPX-002
2. Proceed to Phase 2 (Timer Support) - tickets SPX-003 to SPX-005
3. Continue to Phase 3 (Network Support) - tickets SPX-006 to SPX-010
4. Move to Phase 4 (Postgres Integration) - tickets SPX-011 to SPX-014
5. Complete with Phase 5 (Integration) - tickets SPX-015 to SPX-017

**Next Action:** Do NOT start implementation yet. Review this plan, adjust
if needed, then begin execution following `planning/implementation_sop.md`
for each ticket.
