//gcc -shared -fPIC -o libstubwayland.so stub_wayland.c
//LD_PRELOAD=./libstubwayland.so ./firefox

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

// Example stub functions for Wayland and GDK symbols

// GDK Wayland device
void *gdk_wayland_device_get_wl_seat() {
    return NULL;
}

// Wayland functions
void *wl_display_connect() {
    return NULL;
}

void wl_display_disconnect(void *display) {
    // do nothing
}

int wl_display_dispatch(void *display) {
    return 0;
}

int wl_display_roundtrip(void *display) {
    return 0;
}

void gdk_wayland_window_set_use_custom_surface() {
    // do nothing
}

// Add more functions as needed based on your findings
