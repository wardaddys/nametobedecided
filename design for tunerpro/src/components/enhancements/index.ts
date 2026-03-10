/**
 * Enhancement Components for ECU Tuner Pro
 * 
 * These components add professional-grade features including:
 * - Quick start guide/onboarding
 * - Keyboard shortcuts
 * - Progress indicators for long operations
 * - Data export functionality
 * - Enhanced tooltips with glossary integration
 * - Connection status indicators
 * - Input validation with min/max enforcement
 * - Danger confirmation dialogs
 * - What-If simulator
 * - Success animations
 * - Inline suggestions
 * - Bulk interpolation tools
 * - Hotkey overlay
 * 
 * All enhancements follow WCAG 2.1 AA accessibility guidelines
 * and include 200ms micro-interactions as specified in testing requirements.
 */

// Base Enhancements
export { QuickGuideOverlay } from "./QuickGuideOverlay";
export { KeyboardShortcuts } from "./KeyboardShortcuts";
export { ProgressModal } from "./ProgressModal";
export { ExportDialog } from "./ExportDialog";
export { TooltipWrapper, ECUTooltips } from "./TooltipWrapper";
export { ConnectionStatus } from "./ConnectionStatus";
export { ValidatedInput } from "./ValidatedInput";

// Advanced Features (from Ultimate Refinement Prompt)
export { DangerConfirmDialog, DangerDialogs } from "./DangerConfirmDialog";
export { HotkeyOverlay } from "./HotkeyOverlay";
export { WhatIfSimulator } from "./WhatIfSimulator";
export { SuccessAnimation } from "./SuccessAnimation";
export { InlineSuggestionChip, SuggestionScenarios } from "./InlineSuggestionChip";
export { BulkInterpolationTool } from "./BulkInterpolationTool";
