The tiff files are the RAW RGB data files. They have been provided by USC Viterbi - SIPI Image Database located here : http://sipi.usc.edu/database/database.php
The tiff files were obtained from http://sipi.usc.edu/database/database.php?volume=misc
The C program to seperate the RAW RGB tiff files to individual RGB channels is tiff2raw.c which is located at http://sipi.usc.edu/database/tiff2raw.c
The C program is also present in this folder just in case. It has been named as tiff2raw.c.txt. It has been modified to output widthxheight in the output file name.
The program needs libtiff which can be got at http://www.remotesensing.org/libtiff/
Compile as follows:  gcc tiff2raw.c -ltiff -o tiff2raw

To convert a 24-bit/pixel TIFF image (image.tiff) to three raw images,  the basename of the output files must be specified.

      tiff2raw image.tiff output

The raw image output will be written to three files: "output.wxh.red",  "output.wxh.grn" and "output.wxh.blu".

