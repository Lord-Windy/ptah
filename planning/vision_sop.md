# Vision SOP (`vision_sop.md`)

**Purpose:** To establish a high-level strategic plan for a feature or system, decomposing it into atomic, implementable units of work ("Tickets") that reference the `implementation_sop.md`.

**Trigger:** When starting a new feature, refactor, or complex task.

---

## 1. Define the "North Star"
Start by stating the ultimate goal in 1-2 sentences.
*   What is being built?
*   Why is it needed?

## 2. Context & Constraints
Briefly list:
*   **Pre-requisites:** What must exist before we start?
*   **Dependencies:** Which libraries (existing or external) will we use?
*   **Boundaries:** What is explicitly *out of scope*?

## 3. Architecture Strategy
Describe the approach.
*   **Data Structures:** What key structs/objects are needed?
*   **Interfaces:** What does the public API look like? (Pseudocode is fine).
*   **Integration:** How does this fit into the existing Hexagonal/Modular architecture of Ptah?

## 4. The Breakdown (Ticket Generation)
Break the work into linear, atomic steps. Each step must be "implementable" in a single session.

**Format for each Ticket:**
1.  **ID/Title:** `[SYS-001] <Action> <Component>`
2.  **Description:** One sentence on *what* to change.
3.  **Definition of Done:** Specific file changes or test outcomes.
4.  **Next Step:** Link to `planning/implementation_sop.md` for execution.

## 5. Deliverable
Create a new planning file (e.g., `planning/active/<feature_name>.md`) containing the above. Do *not* start writing code yet.
