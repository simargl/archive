// g++ radio.cpp -o radio $(fltk-config --cxxflags) $(fltk-config --ldflags) -lmpv

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Slider.H>

extern "C" {
#include <mpv/client.h>
}

#include <cstring>
#include <cstdint>
#include <cstdio>


struct Station {
    const char *name;
    const char *url;
};


Station stations[] =
{
    {"Radio Paradise",
     "https://stream.radioparadise.com/mp3-128"},

    {"SomaFM Groove Salad",
     "https://ice2.somafm.com/groovesalad-128-mp3"},

    {"SomaFM Drone Zone",
     "https://ice2.somafm.com/dronezone-128-mp3"},

    {"SomaFM Secret Agent",
     "https://ice2.somafm.com/secretagent-128-mp3"},

    {"KEXP Seattle",
     "https://kexp-mp3-128.streamguys1.com/kexp128.mp3"},

    {"181.FM The Eagle",
     "https://listen.181fm.com/181-eagle_128k.mp3"},

    {"181.FM Classical",
     "https://listen.181fm.com/181-classical_128k.mp3"},

    {"Jazz24",
     "https://live.wostreaming.net/direct/ppm-jazz24mp3-ibc1"},

    {"SomaFM Indie Pop Rocks",
     "https://ice2.somafm.com/indiepop-128-mp3"},

    {"Radio Swiss Jazz",
     "https://stream.srg-ssr.ch/m/rsj/mp3_128"}
};


#define NSTATIONS 10


mpv_handle *mpv;
Fl_Box *title_box;
Fl_Slider *volume_slider;


char last_title[512] = "";


void play_station(int n)
{
    const char *cmd[] =
    {
        "loadfile",
        stations[n].url,
        "replace",
        nullptr
    };

    mpv_command(mpv, cmd);

    strcpy(last_title, stations[n].name);

    title_box->copy_label(stations[n].name);
    title_box->redraw();
}


void stop_player(Fl_Widget *, void *)
{
    const char *cmd[] =
    {
        "stop",
        nullptr
    };

    mpv_command(mpv, cmd);
}


void volume_changed(Fl_Widget *, void *)
{
    int64_t vol =
        (int64_t)volume_slider->value();

    mpv_set_property(
        mpv,
        "volume",
        MPV_FORMAT_INT64,
        &vol
    );
}


void check_metadata(void *)
{
    while (true)
    {
        mpv_event *event =
            mpv_wait_event(mpv, 0);

        if (event->event_id ==
            MPV_EVENT_NONE)
            break;


        if (event->event_id ==
            MPV_EVENT_PROPERTY_CHANGE)
        {
            mpv_event_property *p =
                (mpv_event_property *)event->data;


            if (!p || !p->name)
                continue;


            if (strcmp(p->name,"metadata")==0 &&
                p->format == MPV_FORMAT_NODE)
            {
                mpv_node *node =
                    (mpv_node *)p->data;


                if (!node ||
                    node->format !=
                    MPV_FORMAT_NODE_MAP)
                    continue;


                mpv_node_list *list =
                    node->u.list;


                for (int i=0;i<list->num;i++)
                {
                    const char *key =
                        list->keys[i];


                    if (!strcmp(key,"icy-title") ||
                        !strcmp(key,"title"))
                    {
                        if (list->values[i].format ==
                            MPV_FORMAT_STRING)
                        {
                            const char *song =
                                list->values[i].u.string;


                            if (strcmp(song,last_title))
                            {
                                strncpy(
                                    last_title,
                                    song,
                                    sizeof(last_title)-1
                                );

                                title_box->copy_label(song);
                                title_box->redraw();

                                printf(
                                    "Now Playing: %s\n",
                                    song
                                );
                            }
                        }
                    }
                }
            }
        }
    }


    Fl::repeat_timeout(
        0.5,
        check_metadata
    );
}


void button_callback(Fl_Widget *, void *data)
{
    play_station((int)(intptr_t)data);
}


int main()
{
    mpv = mpv_create();

    if (!mpv)
        return 1;


    if (mpv_initialize(mpv) < 0)
        return 1;


    mpv_observe_property(
        mpv,
        0,
        "metadata",
        MPV_FORMAT_NODE
    );


    int64_t volume = 60;

    mpv_set_property(
        mpv,
        "volume",
        MPV_FORMAT_INT64,
        &volume
    );


    Fl_Window win(
        420,
        470,
        "MPV Internet Radio"
    );


    Fl_Button b1(20,20,180,35,"Radio Paradise");
    Fl_Button b2(220,20,180,35,"Groove Salad");

    Fl_Button b3(20,65,180,35,"Drone Zone");
    Fl_Button b4(220,65,180,35,"Secret Agent");

    Fl_Button b5(20,110,180,35,"KEXP");
    Fl_Button b6(220,110,180,35,"181 Eagle");

    Fl_Button b7(20,155,180,35,"Classical");
    Fl_Button b8(220,155,180,35,"Jazz24");

    Fl_Button b9(20,200,180,35,"Indie Pop");
    Fl_Button b10(220,200,180,35,"Swiss Jazz");


    Fl_Button stop(
        150,245,120,35,
        "STOP"
    );


    title_box =
        new Fl_Box(
            20,
            300,
            380,
            60,
            "Ready"
        );

    title_box->box(FL_DOWN_BOX);
    title_box->align(
        FL_ALIGN_CENTER |
        FL_ALIGN_WRAP
    );


    volume_slider =
        new Fl_Slider(
            20,
            390,
            380,
            30
        );

    volume_slider->type(
        FL_HORIZONTAL
    );

    volume_slider->range(
        0,
        100
    );

    volume_slider->value(
        60
    );

    volume_slider->callback(
        volume_changed
    );


    b1.callback(button_callback,(void*)0);
    b2.callback(button_callback,(void*)1);
    b3.callback(button_callback,(void*)2);
    b4.callback(button_callback,(void*)3);
    b5.callback(button_callback,(void*)4);
    b6.callback(button_callback,(void*)5);
    b7.callback(button_callback,(void*)6);
    b8.callback(button_callback,(void*)7);
    b9.callback(button_callback,(void*)8);
    b10.callback(button_callback,(void*)9);

    stop.callback(stop_player);


    win.end();
    win.show();


    Fl::add_timeout(
        0.5,
        check_metadata
    );


    int ret = Fl::run();


    mpv_destroy(mpv);

    return ret;
}
