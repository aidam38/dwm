# dwm by Adam Křivka (aidam38)

## Basic movement
In general, combinations with Alt move through the windows and tags, Alt+Shift resizes windows and Alt+Ctrl moves them around.
* Alt+j, Alt+ m, Alt+Shift+m and Alt+k, Alt+comma, Alt+Shift+comma: selects first, second or third window in the master or stack area in the current tag respectively. This assumes you are most often working with up to five windows. The position of the keys on the keyboard should resemble how the windows are arranged on the screen. You can then build muscle memory for Alt+j and Alt+k for dual window arrangement, in which you frankly spend most of your time probably.
* Shift+Alt+j,k,h,l: resizes windows relatively (mfact and cfact in dwm source code)
* Ctrl+Alt+j,k,h,l: pushed windows around -- j,k up and down the list, h,l increases the number of clients in the master (left) area
* Shift+Ctrl+Alt+j,k: cycles the windows -- the default behaviour of dwm. This is rarely used, but it's there so you could in theory reach 5th or further window
* Alt+space: sends current window to the master area
* Alt+i,o,p: changes the gap between windows
* Other key combinations are used to launch scripts and applications

The colors of dwm are outsourced to a file "dwmcolors.h" to be then used by dmenu and other programs to match dwm (usually system-wide) colorscheme.

