WPSet
=======

A simple tool for changing your desktop wallpaper

How to install?
````
cd vala-apps/wpset
mkdir build; cd build
cmake ..
make; make install
gtk-update-icon-cache /usr/share/icons/hicolor
glib-compile-schemas /usr/share/glib-2.0/schemas
````
Dependencies:
````
gtk+-3.0
gsettings-desktop-schemas
imlib2
````
