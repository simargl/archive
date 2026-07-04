Agatha
=======

PDF Viewer in GTK3 and Poppler

How to install?
````
cd vala-apps/agatha
mkdir build; cd build
cmake ..
make; make install
gtk-update-icon-cache /usr/share/icons/hicolor
glib-compile-schemas /usr/share/glib-2.0/schemas
````
Dependencies:
````
gtk+-3.0
poppler-glib
````
