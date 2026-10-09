# Desktop UI refresh — phase 1

The main window now uses a sidebar, a persistent connection panel, and a node-focused home page. Logs and connection records use their existing views on separate pages. Subscription, routing and settings buttons open the existing dialogs; their redesign belongs to phase 2.

The node table retains the existing profile model, selection, sorting, context menus and drag ordering. Name is displayed first without changing logical column indexes or persisted widths. Search stays visible and remains applied when the table refreshes. The connection button calls the existing start/stop handlers and asks the user to select a node when none is selected.

Built-in theme IDs are preserved. System (0) uses the initial system palette to choose the modern light or dark colors. Modern Light (4) and Modern Dark (5) are explicit options. Legacy themes (1–3) and QStyle choices remain available. System appearance is sampled at startup; live OS appearance changes are not yet tracked.

## Validation

- Qt 5.15 debug GUI build: `cmake -S . -B build-ui -DNKR_NO_EXTERNAL=ON -DCMAKE_BUILD_TYPE=Debug`, then `cmake --build build-ui -j 4`.
- Native offscreen smoke run with a temporary instrumented entry point and example profiles: node population, name-first columns, search, search after refresh, navigation between home/logs/connections, add-menu actions, theme idempotence, dark palette, and connection control visibility at 800 × 600.
- Actual Qt screenshots inspected at 1060 × 720 and 800 × 600, including Chinese text and both modern themes.

This validation build excludes gRPC, YAML, QR scanning and hotkeys. It verifies the GUI, not live proxy connectivity, subscription downloads, system proxy changes, TUN permissions or Windows packaging. Phase 3 must run those checks with the full dependencies and sing-box core before producing a distributable release.
