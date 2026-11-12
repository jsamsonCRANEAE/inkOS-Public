# InkOS Operations Handbook

Apollo-style "Systems Data" pages for InkOS. Print the PDF on US Letter.

| Page | Subject | Preview |
|---|---|---|
| 2.1-1 | Software load, SD card route | [page_2.1-1.png](page_2.1-1.png) |
| 2.1-2 | Malfunction and rollback | [page_2.1-2.png](page_2.1-2.png) |

**Print:** [InkOS_Software_Load_Handbook.pdf](InkOS_Software_Load_Handbook.pdf)

## How these pages were made
Written with the [elephant-goldfish method](https://github.com/google/elephant-goldfish-model):
the "elephant" (full project context) drafted the page; a "goldfish" reviewer with no
prior context checked it against the code and build log; findings were resolved and the
owner approved. See [review_2.1.md](review_2.1.md).

## Sources of the numbers
- Image size, partition use, RAM: GitHub Actions build log (`pio run -e x4`).
- Button bands: `include/board.h`. Nominal readings: community measurements (unverified on this unit).
- SD-card load route (update.bin in card root, POWER + UP, USB power): pocketink.io community guide (unverified on this unit).

## Editing
`source/` holds the HTML pages and the Python generators that drew them.
Regenerate with the Apollo handbook kit (`render.py`, `check.py`), then rebuild the PDF.
Update the **Change Date** in the footer when a page changes.
The embedded Drafting Caps font is SIL OFL 1.1 — keep `source/OFL.txt` with the sources.
