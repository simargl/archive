#!/bin/bash
# 
# Author: simargl <https://github.com/simargl>
# License: GPL v3

for i in $(find . -maxdepth 1 -mindepth 1 -type d -not -path '*/\.*' | sort); do 
    cd $i; mkdir build; cd build; cmake ..; make install DESTDIR=/tmp/1; cd ..; rm -r build; cd ..;
done
mksquashfs /tmp/1 vala-apps.sb
