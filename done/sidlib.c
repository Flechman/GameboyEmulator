

#include "sidlib.h"

// ======================================================================
static int update_(gpointer data)
{
    simple_image_displayer_t* const psd = data;

    GdkPixbuf* pb = gtk_image_get_pixbuf(GTK_IMAGE(psd->image));
    // gdk_pixbuf_fill(pb, 0); // clear to black

    guchar* pixels = gdk_pixbuf_get_pixels(pb);
    psd->gen(pixels, psd->height, psd->width);
    gtk_image_set_from_pixbuf(GTK_IMAGE(psd->image),
                              gdk_pixbuf_new_from_data(
                              pixels,
                              GDK_COLORSPACE_RGB,      // colorspace
                              0,                       // has_alpha (no alpha)
                              8,                       // bits-per-sample (must be 8)
                              psd->width, psd->height, // cols, rows
                              3*psd->width,            // rowstride
                              NULL, NULL               // nothing to free yet
                              )
                             );

    return 1; // continue timer
}

// ======================================================================
simple_image_displayer_t* sd_init(const char* title, int width, int height, guint time,
                                  ds_image_generator generator,
                                  ds_key_handler key_p_handler, ds_key_handler key_r_handler)
{
    simple_image_displayer_t* output = calloc(sizeof(simple_image_displayer_t), 1);
    if (output != NULL) {
        output->width = width;
        output->height = height;
        output->gen = generator;
        output->keys_p = (key_p_handler == NULL ? ds_simple_key_handler : key_p_handler);
        output->keys_r = key_r_handler;
        output->time = time;
        output->timeout_id = 0;
        output->title = title;
        output->image = NULL;
    }
    return output;
}

// ======================================================================
static GBytes* bw_to_rgb(const guchar* bw, guint size)
{
    GByteArray* rgb = g_byte_array_sized_new(3 * size);
    if (rgb != NULL) {
        for (size_t i = 0; i < size; i++)
            for (size_t j = 0; j < 3; j++)
                g_byte_array_append(rgb, bw + i, 1);
    }
    return g_byte_array_free_to_bytes(rgb);
}

// ======================================================================
gboolean ds_simple_key_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;
    switch(keyval) {
    case GDK_KEY_space:
        if (psd->timeout_id > 0) {
            // pause update
            g_source_remove(psd->timeout_id);
            psd->timeout_id = 0;
        } else {
            // relaunch update
            if ((psd->time > 0) && (psd->gen != NULL)) {
                psd->timeout_id = g_timeout_add(psd->time, update_, data);
            }
        }
        return TRUE;

    case 'q':
    case 'Q':
        gtk_main_quit();
        return TRUE;
    }
    return FALSE;
}

// ======================================================================
static gboolean keypress_handler_(GtkWidget* widget __attribute__((unused)),
                                  GdkEventKey* event, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if ((psd == NULL) || psd->keys_p == NULL) return FALSE;
    return psd->keys_p(event->keyval, data);
}

// ======================================================================
static gboolean keyrelease_handler_(GtkWidget* widget __attribute__((unused)),
                                    GdkEventKey* event, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if ((psd == NULL) || psd->keys_r == NULL) return FALSE;
    return psd->keys_r(event->keyval, data);
}

// ======================================================================
void sd_launch(int* p_argc, char*** p_argv, simple_image_displayer_t* p_sd)
{
    if (p_sd != NULL) {
        // initial image all black
        guchar* bw = calloc((size_t) p_sd->width * sizeof(guchar), (size_t) p_sd->height);

        gtk_init(p_argc, p_argv);
        GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), p_sd->title);
        gtk_window_set_default_size(GTK_WINDOW(window), p_sd->width + 20, p_sd->height + 20);
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

        p_sd->image = gtk_image_new_from_pixbuf(
                      gdk_pixbuf_new_from_bytes(
                      bw_to_rgb(bw, (guint) (p_sd->height * p_sd->width)),
                      GDK_COLORSPACE_RGB,        // colorspace
                      0,                         // has_alpha (no alpha)
                      8,                         // bits-per-sample (must be 8)
                      p_sd->width, p_sd->height, // cols, rows
                      3*p_sd->width              // rowstride
                      )
                      );
        gtk_container_add(GTK_CONTAINER(window), p_sd->image);

        // quit function
        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        if ((p_sd->time > 0) && (p_sd->gen != NULL)) {
            // set update function
            p_sd->timeout_id = g_timeout_add(p_sd->time, update_, p_sd);
        } else if (p_sd->gen != NULL) {
            // call update function only once
            (void)update_(p_sd);
        }

        // keys handle
        if (p_sd->keys_p != NULL) {
            g_signal_connect(window, "key_press_event", G_CALLBACK(keypress_handler_), p_sd);
        }
        if (p_sd->keys_r != NULL) {
            g_signal_connect(window, "key_release_event", G_CALLBACK(keyrelease_handler_), p_sd);
        }

        gtk_widget_show_all(window);
        gtk_main();
        free(p_sd);
    }
}

