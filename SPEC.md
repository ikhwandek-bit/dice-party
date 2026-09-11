# [Project Name] — Specification

> **Status:** Draft / In Review / Approved
> **Owner:** [Name]
> **Last updated:** [YYYY-MM-DD]
> **Version:** [0.1]

---

## 1. Overview

<!-- 1-2 paragraphs: what is this, who is it for, and why does it exist?
     Someone should understand the point of the project after reading just this section. -->

## 2. Goals

<!-- What this project MUST achieve. Bullet points, concrete and testable. -->
- Goal 1
- Goal 2

## 3. Non-Goals

<!-- Explicitly out of scope. Prevents scope creep and re-litigating decisions later. -->
- Not doing X
- Not supporting Y (at least in this version)

## 4. Requirements

### 4.1 Functional Requirements
<!-- What the system must do. User-story format works well:
     "As a [role], I can [action] so that [outcome]." -->
- [ ] FR-1: As a ___, I can ___ so that ___.
- [ ] FR-2: ...

### 4.2 Non-Functional Requirements
<!-- Performance, security, compatibility, scalability, accessibility, etc. -->
- [ ] NFR-1: Must support ___
- [ ] NFR-2: Response time under ___
- [ ] NFR-3: Compatible with ___

## 5. Architecture / Design

<!-- How the pieces fit together. Diagrams (even ASCII) are encouraged.
     Cover: major components/modules, data flow, third-party integrations, tech stack. -->

```
[optional architecture diagram or component list]
```

## 6. User Flows

<!-- Walk through main scenarios step by step. Catches edge cases early. -->

**Flow 1: [Name]**
1. Step
2. Step
3. Step

## 7. Interface / API Contracts

<!-- Endpoints, function signatures, hooks/events, expected inputs/outputs.
     Skip or trim if not applicable. -->

| Interface | Input | Output | Notes |
|---|---|---|---|
| | | | |

## 8. Data Model

<!-- Tables, fields, types, relationships. Or state shape for frontend-heavy projects. -->

| Entity | Field | Type | Notes |
|---|---|---|---|
| | | | |

## 9. Edge Cases & Error Handling

<!-- What happens when things go wrong? Duplicate input, invalid data, concurrency, offline, etc. -->
- Case: ... → Behavior: ...

## 10. Assumptions & Open Questions

<!-- Anything unresolved. Better to flag than silently assume. -->
- [ ] Question 1
- Assumption: ...

## 11. Milestones / Phases

<!-- Optional but useful for tracking progress and natural checkpoints. -->

| Phase | Deliverable | Target Date | Status |
|---|---|---|---|
| 1 | | | |
| 2 | | | |

## 12. References

<!-- Related docs, prior art, design inspiration, external specs. -->
- [Link/doc]

---

### How to use this template
- Delete sections that genuinely don't apply — an empty section is worse than no section.
- Keep it **living**: update it as decisions change. A stale spec is worse than none.
- Be **concrete, not aspirational** ("shows pending applications in a sortable table" > "is user-friendly").
- Write it *before or alongside* development, not after — retroactive specs are documentation, not design tools.