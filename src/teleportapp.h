#ifndef __TELEPORTAPP_H
#define __TELEPORTAPP_H

#include <gtk/gtk.h>


#define TELEPORT_APP_TYPE (teleport_app_get_type ())
G_DECLARE_FINAL_TYPE (TeleportApp, teleport_app, TELEPORT, APP, GtkApplication)


TeleportApp     *teleport_app_new         (void);
extern void teleport_app_add_peer();
extern void teleport_app_remove_peer();


#endif /* __TELEPORTAPP_H */
