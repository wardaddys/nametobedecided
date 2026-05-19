# Questions for Saeed

## Q1: TunerProColors.h → OSTunerColors.h file rename
The prompt says rename `TunerProColors.h` to `OSTunerColors.h`. This file is `#include`d by 15+ widget files. Renaming the file itself requires updating every `#include "core/TunerProColors.h"` across the codebase, plus the CMakeLists.txt HEADERS list.

**Proposed approach:** Rename the file, then do a batch find-replace on the include. This is mechanical but touches many files.

**Alternative:** Create `OSTunerColors.h` that just `#include`s `TunerProColors.h` as an alias, do the rename gradually. But this leaves the old name in the tree.

**Going with:** Full rename now. If this causes merge conflicts with your local work, let me know.

## Q2: GitHub repo URL for releases
The `UpdateChecker` now opens a browser to a releases URL. I used `https://github.com/AeroSaeed/OS-Tuner/releases` as a placeholder. What's the actual GitHub repo URL?

## Q3: License file
The prompt recommends AGPL-3.0. Should I add a `LICENSE` file now, or wait for your explicit decision?
