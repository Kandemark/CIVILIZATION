# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- Native Windows GUI (`win32_app.c`) with GDI rendering.
- Advanced Economic System (Currency, Trade, Market Dynamics).
- Environmental Disaster System (Earthquakes, Floods, etc.).
- Event Trigger System linking economy and environment to game events.
- Interactive CLI for legacy/debug mode.

### Changed
- Migrated from console-based entry point to Win32 API.
- Refactored `game.c` to support modular system initialization.
