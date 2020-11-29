#pragma once

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief image generator function type
 */
typedef void (*ds_image_generator)(guchar*, int, int);


/**
 * @brief key handler function type
 */
typedef gboolean (*ds_key_handler)(guint keyval, gpointer data);


/**
 * @brief type regrouping all the parameters needed for a Simple Image Displayer
 */
typedef struct {
    int width;
    int height;
    ds_image_generator gen;
    ds_key_handler keys_p; // key press
    ds_key_handler keys_r; // key release
    unsigned char key_status; // 8 bits for key status (use as you may need)
    guint time;
    guint timeout_id;
    const char* title;
    GtkWidget* image;
} simple_image_displayer_t;


/**
 * @brief Create (and initialize) a Simple Image Displayer
 *
 * @param title title of the window
 * @param width width of the image(s) to be displayed
 * @param height height of the image(s) to be displayed
 * @param time timelaps between two image refresh (in milliseconds); if 0, no refresh at all
 * @param generator image generating function (will be called each `time` milliseconds); if 0, no refresh at all
 * @param key_p_handler a key handling function used on key-press event; if NULL, ds_simple_key_handler() is used
 * @param key_r_handler a key handling functionused on key-release event
 * @return a pointer to the newly created Simple Image Displayer
 */
simple_image_displayer_t* sd_init(const char* title, int width, int height, guint time,
                                  ds_image_generator generator,
                                  ds_key_handler key_p_handler, ds_key_handler key_r_handler);


/**
 * @brief Run a Simple Image Displayer
 *
 * @param p_argc a pointer to main argc
 * @param p_argv a pointer to main argv
 * @param p_sd a pointer to the Simple Image Displayer to be launched
 */
void sd_launch(int* p_argc, char*** p_argv, simple_image_displayer_t* p_sd);


/**
 * @brief a simple keypress handler, which pauses timer on 'SPACE' and quit application on 'Q' or 'q'.
 *
 * @param keyval the key value
 * @param data some data if needed
 * @return 'TRUE' if key was handled, 'FALSE' otherwise.
 */
gboolean ds_simple_key_handler(guint keyval, gpointer data);

#ifdef __cplusplus
}
#endif
