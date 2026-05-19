# OS Tuner Unified Settings Experience — Implementation Plan

This document outlines the first steps to implement the OS Tuner Unified Settings Experience, as per Phase 0 of the development prompt.

## Phase 0: Data model unification

The goal of this phase is to update the underlying `ECUDefinition` structures to support workspace-based UI grouping and to introduce the new `WorkspaceRegistry` class for querying these settings.

### Step 1: Update `ECUDefinition` (Phase 0.1)
Modify `ECUDefinition::Constant` and `ECUDefinition::Table` in `src/core/ECUDefinition.h` to include metadata fields:
- `primaryWorkspace`: The workspace where this setting is primarily edited (e.g., "fueling").
- `secondaryWorkspaces`: List of workspaces where this setting appears as read-only.
- `subsection`: The specific sub-section within the workspace (e.g., "injector_hardware").
- `displayOrder`: Integer value for sorting settings within a sub-section.
- `humanLabel`: User-facing descriptive name for the setting.
- `helpText`: Extended context or help description.

### Step 2: Create Mapping YAML and Registry (Phase 0.2 & 0.3)
1. **Create `WorkspaceRegistry` Class:**
   - Define `WorkspaceRegistry.h` and `WorkspaceRegistry.cpp` in `src/core/`.
   - Implement YAML parsing to load mapping configurations.
   - Provide lookup methods for finding a workspace's ID, label, related settings, and secondary workspaces.
   
2. **Build `resources/workspaces/mapping.yaml`:**
   - Define mapping for top priority settings initially.
   - The registry will read this file and attach the parsed metadata to the initialized `ECUDefinition` parameters during or post-load.

3. **Workspace Configuration YAML:**
   - Create `resources/workspaces/workspace_metadata.yaml` if needed to store the definitions of the 12 locked workspaces.

### Step 3: Integrate and Test (Phase 0.4)
- Test that the YAML loads cleanly.
- Verify `WorkspaceRegistry` can correctly answer reverse lookups mapping workspace IDs to settings.
- Flag settings without primary workspaces to enforce compliance.

Following the successful execution of Phase 0, I will proceed to Phase 1: Workspace shell creation.
