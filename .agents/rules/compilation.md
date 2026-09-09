---
trigger: always_on
description: Compilation and build rules for qgvdial
---

# Compilation and Build Rules

* **Default Build Command**: The compilation and build step MUST always use the default build command specified in the VS Code tasks file (`.vscode/tasks.json`).
* Locate the task configured with `"group": { "kind": "build", "isDefault": true }` (currently `make qgvdial_ubuntu_x86_64_ctr`).
* Run that default build task for verification and building rather than ad-hoc make targets or raw compiler commands.
