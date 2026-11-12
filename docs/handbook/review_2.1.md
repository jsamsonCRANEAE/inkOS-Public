# Goldfish review — Handbook 2.1 (Software Load), 12 Nov 2026

Reviewer: fresh-context agent, given only the draft page and the repo/build log.
Verdict on numbers: **all checked values matched source** (406 KB = 415,616 B; 30.1 % of
1,310,720 B; 62 KB of 320 KB RAM; thresholds 740/2060/3060/3800 and 1100/3150; 5/20 ms poll).

| # | Sev | Finding | Resolution |
|---|---|---|---|
| 1 | HIGH | No USB-lock warning; no backup guidance | Page 2: prerequisites + branch A (stop, suspect lock) |
| 2 | HIGH | SSD1677 stated as fact; no blank-screen branch | Marked "assumed"; branch B (roll back, display driver) |
| 3 | HIGH | SD loader stated as certain | Marked community-reported/unverified (both pages) |
| 4 | MED | Rollback source/rename missing; CrossPoint vs stock unclear | Full URL + rename step; rollback card on page 2 |
| 5 | MED | POWER + UP step vague; no "load ended" signal | Decision flowchart: "screen changed?" check; 2.1.5.1 unverified note |
| 6 | MED | Safari renames repeat downloads; card format unstated | Size/name check on card; fig 2.1-4; FAT32 in prerequisites |
| 7 | MED | Diagnostics dead end if buttons misdecode | Branch C: note count, edit board.h on github.com, rebuild |
| 8 | LOW | "1.25 MB app slot" is the build default, not measured | Reworded "1.25 MB build"; removed from CAUTION |
| 9 | LOW | Failed build leaves LATEST stale silently | Malfunction table row |
| 10 | LOW | POWER not testable in Diagnostics; name casing | NOTE on page 2; lowercase update.bin in prose |

Open items (need hardware): confirm SD loader behaviour, load-complete signal, display controller.
