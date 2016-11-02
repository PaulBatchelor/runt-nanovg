# runt-nanovg

Runt bindings to nanovg for realtime vector graphics.

## Current list of words

x1,y1,x2,y2 vgcirc: "draws a circle"
x1,y2,x2,y2 vgrect: "draws a rectangle"
r,g,b,a vgfc: sets fill color
r,g,b,a vgf: fills a shape
id vgstrt: starts nanovg loop using id as callback id
r,g,b,a vgclr: sets clear color
(none) vgstop: stops nanovg
id vgswp: swaps out current drawing callback
(none) vgw: gets window width
(none) vgh: gets window height
