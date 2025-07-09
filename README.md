## Description
Added support for sending GameID information to MemCard PRO GC. Based on the [implementation in Swiss](https://github.com/FIX94/Nintendont/commit/a186ea2715cdb877b3cc5b3d4269a18e2ace8d48).

There are three settings:
 * `Off`: No GameID information is sent to the memory slots.
 * `Full ID`: The full GameID is sent, including Company and Revision data (e.g. RSBE0100). This matches the Swiss naming convention.
 * `Short ID`: A shortened ID is sent, omitting the Company and Revision data (e.g. RSBE). This matches the Nintendont Emulated Memory Card naming convention.
