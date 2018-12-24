# dwm by Adam Křivka (aidam38)

## Basic movement
Modkey is Alt, often combined with Shift and Control.
* Alt+j,k,l,; selects first, second, third, fourth window respectively in the current tag. This assumes you are most often working with up to four windows
* Shift+Alt+j,k,h,l resizes windows relatively (mfact and cfact in dwm source code)
* Ctrl+Alt+j,k,h,l pushed windows around -- j,k up and down the list, h,l increases the number of clients in the master (left) area
* Shift+Ctrl+Alt+j,k cycles the windows -- the default behaviour of dwm. This is rarely used, but it's there so you could in theory reach 5th or further window
* Alt+space sends current window to the master area
* Alt+i,o,p changes the gap between windows
* Other key combinations are used to launch scripts and applications

The colors of dwm are outsourced to a file "dwmcolors.h" to be then used by dmenu and other programs to match dwm (usually system-wide) colorscheme.

