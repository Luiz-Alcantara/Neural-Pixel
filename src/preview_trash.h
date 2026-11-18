#ifndef PREVIEW_TRASH_H
#define PREVIEW_TRASH_H

#include <gtk/gtk.h>
#include "preview_trash.h"
#include "structs.h"

void preview_image_visibility_toggle(GtkButton *btn, gpointer user_data);

void navigate_preview(PreviewImageData *data, int offset);

void send_to_trash(gpointer user_data);

#endif // PREVIEW_TRASH_H
