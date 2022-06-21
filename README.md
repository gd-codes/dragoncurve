# dragoncurve

The [Dragon Curve fractal](https://en.wikipedia.org/wiki/Dragon_curve) is formed by beginning with a line segment, and recursively rotating or "unfolding" it to create a copy of the shape angled 90° to the previous one, as illustrated in the wikipedia link.

### Files
This repo has a collection of individual scripts written in C++11, using the [SFML library](https://www.sfml-dev.org/index.php) for GUI, to recursively generate and draw this fractal. 

The shape is kept track of using an `std::vector<bool>` which represents the sequence of turn directions (Left/Right) for each rotation / corner of the linear path. Iterators over this vector keep looping over it and appending bits at the end to continually keep generating the next order (recursive stage) of the curve.

`dc2` draws the Regular Dragon Curve, `dc3` the Terdragon (variant with 2 unfolds / rotations of angle 120° in each recursive step, instead of just 1 of angle 90°). `dc2_mt` and `dc3_mt` generate the curve in a separate thread from the GUI, which causes it to finish much faster (the "drawing" of the curve is barely visible & finishes in a few frames)

### Compiling and Running
Steps to install SFML, build it (with makefile / from source), and link this code to the binaries are available on the SFML website (Links : [Download page](https://www.sfml-dev.org/download/sfml/2.5.1/) | [Installation & Usage tutorials on various platforms](https://www.sfml-dev.org/tutorials/2.5/#getting-started) | [Using g++ on \*nix](https://stackoverflow.com/questions/35697297/compiling-sfml-on-linux-ubuntu))
