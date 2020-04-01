# demauze
a really primitive pseudo3d first-person renderer

As I was developing my own pseudo-3d engine I was wondering just how much easier would it be to make a similar renderer, but with no texture mapping and fixed, tile-based movement. The inspiration behind this got booted by Maze Wars, which, upon reading some comments from the assembler code available online, I got suspicious when it said the renderer does not support 2x2 squares or largerr.

And indeed, I emulated it and the renderer acted really strange on some very specific conditions hinting at the said limitation. I've never seen this method explained or mentioned anywhere before, so I tried to recreate it by myself. Turns out it's really simple to implement and inexpensive to compute.


This little piece of code requires SDL2, so go get it!
