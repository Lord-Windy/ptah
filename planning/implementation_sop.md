# Implementation SOP (`implementation_sop.md`)

**Purpose:** To execute a single "Ticket" or atomic task safely, adhering to project conventions, and verifying the output.

**Trigger:** When ready to write code for a specific task defined in `vision_sop.md`.

---

## 1. Discovery (Context Audit)
Before writing code, you must understand the environment.
*   **Search:** Use `grep`/`glob` to find similar existing implementations (Style/Pattern matching).
*   **Dependencies:** Verify imports/libraries exist (Check `CMakeLists.txt`, headers).
*   **Location:** Determine exactly where new files go (e.g., `libs/`, `apps/`).

## 2. Technical Design (Mental or Scratchpad)
*   **Signature:** Define function prototypes or struct layouts.
*   **Safety:** Identify potential memory risks or error states.
*   ** conventions:** Ensure strict adherence to C11, `ptah_` naming, and header guards.

## 3. Step-by-Step Execution Plan
Create a specific, ordered checklist of actions for this task. This is your roadmap.
1.  **Draft the Steps:** List every file modification, function creation, and configuration change required.
2.  **Order for Stability:** Arrange steps so the project remains buildable as much as possible (e.g., types first, then declarations, then implementation).
3.  **Granularity:** Each step should be atomic (e.g., "Add `struct Point` to header", not "Implement Physics").

*Example Plan:*
> 1. Define `ptah_my_struct` in `include/my_lib.h`.
> 2. Add `ptah_my_func` prototype to `include/my_lib.h`.
> 3. Create `src/my_lib.c` and add function stubs.
> 4. Add `src/my_lib.c` to `CMakeLists.txt`.
> 5. Implement core logic in `src/my_lib.c`.

## 4. Implementation
Execute the plan created in Step 3.
*   Check off items as they are completed.
*   Maintain the "Scaffold -> Logic -> Build" flow within the specific steps where applicable.

## 5. Verification (The "Must-Haves")
No task is done without:
1.  **Usage Test:** Create or update a test file (e.g., `test/test_<feature>.c`) to prove it works.
2.  **Build:** Run existing build commands to ensure no compilation errors.
3.  **Sanity Check:** Does it match the "Design" from Step 2?
4.  **Cleanup:** Remove any temp files or debug prints.

## 5. Completion
Update the tracking document (from `vision_sop.md`) to mark the ticket as **DONE**.
