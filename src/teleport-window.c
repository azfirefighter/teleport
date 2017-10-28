#include <gtk/gtk.h>

#include "teleport-app.h"
#include "teleport-window.h"
#include "teleport-server.h"
#include "teleport-peer.h"
#include "teleport-remote-device.h"

TeleportWindow *mainWin;

struct _TeleportWindow
{
  GtkApplicationWindow parent;
};

typedef struct _TeleportWindowPrivate TeleportWindowPrivate;

struct _TeleportWindowPrivate
{
  GSettings *settings;
  GtkWidget *gears;
  GtkWidget *remote_devices_box;
  GtkWidget *this_device_name_label;
  GtkWidget *remote_no_devices;
};

G_DEFINE_TYPE_WITH_PRIVATE(TeleportWindow, teleport_window, GTK_TYPE_APPLICATION_WINDOW);

static void
change_download_directory_cb (GtkWidget *widget,
                              gpointer user_data) {
  GSettings *settings;
  gchar * newDownloadDir;
  settings = (GSettings *)user_data;

  newDownloadDir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
  g_print ("Change download directory\n");
  g_settings_set_string (settings,
                         "download-dir",
                         newDownloadDir);
  g_free(newDownloadDir);
}

static void
teleport_window_init (TeleportWindow *win)
{
  TeleportWindowPrivate *priv;
  GtkBuilder *builder;
  GtkWidget *menu;
  GtkFileChooserButton *downloadDir;
  mainWin = win;

  priv = teleport_window_get_instance_private (win);
  priv->settings = g_settings_new ("com.frac_tion.teleport");

  if (g_settings_get_user_value (priv->settings, "download-dir") == NULL) {
    g_print ("Download dir set to XDG DOWNLOAD directory\n");
    if (g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD) != NULL) {
      g_settings_set_string (priv->settings,
                             "download-dir",
                             g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD));
    }
    else {
      g_print ("Error: XDG DOWNLOAD is not set.\n");
    }
  }

  if (g_settings_get_user_value (priv->settings, "device-name") == NULL) {
    g_settings_set_string (priv->settings,
                           "device-name",
                           g_get_host_name());
  }

  gtk_widget_init_template (GTK_WIDGET (win));

  builder = gtk_builder_new_from_resource ("/com/frac_tion/teleport/settings.ui");
  menu = GTK_WIDGET (gtk_builder_get_object (builder, "settings"));
  downloadDir = GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object (builder, "settings_download_directory"));

  gtk_menu_button_set_popover(GTK_MENU_BUTTON (priv->gears), menu);

  gtk_label_set_text (GTK_LABEL (priv->this_device_name_label),
                      g_settings_get_string (priv->settings, "device-name"));

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (downloadDir),
                                       g_settings_get_string(priv->settings,
                                                             "download-dir"));

  g_signal_connect (downloadDir, "file-set", G_CALLBACK (change_download_directory_cb), priv->settings);
  /*g_settings_bind (priv->settings, "download-dir",
    GTK_FILE_CHOOSER (downloadDir), "current-folder",
    G_SETTINGS_BIND_DEFAULT);
    */

  //g_object_unref (menu);
  //g_object_unref (label);
  g_object_unref (builder);
}


void
update_remote_device_list(TeleportWindow *win,
                          Peer           *device)
{
  TeleportWindowPrivate *priv;
  GtkWidget *remote_device;

  priv = teleport_window_get_instance_private (win);

  gtk_widget_hide (priv->remote_no_devices);

  remote_device = teleport_remote_device_new (device);

  gtk_box_pack_end (GTK_BOX (priv->remote_devices_box),
                    remote_device,
                    TRUE,
                    TRUE,
                    0);
}

static void
remove_remote_peer (GtkWidget *widget,
                    gpointer data)
{
  if (TELEPORT_IS_REMOTE_DEVICE (widget) && teleport_remote_device_get_peer(widget) == ((Peer *) data)) {
    gtk_widget_destroy (widget);
  }
}

void
update_remote_device_list_remove(TeleportWindow *win,
                                 Peer *device)
{
  TeleportWindowPrivate *priv;

  priv = teleport_window_get_instance_private (win);

  gtk_container_foreach (GTK_CONTAINER(priv->remote_devices_box),
                         remove_remote_peer,
                         device);
}

static void
teleport_window_dispose (GObject *object)
{
  TeleportWindow *win;
  TeleportWindowPrivate *priv;

  win = TELEPORT_WINDOW (object);
  priv = teleport_window_get_instance_private (win);

  g_clear_object (&priv->settings);

  G_OBJECT_CLASS (teleport_window_parent_class)->dispose (object);
}

static void
teleport_window_class_init (TeleportWindowClass *class)
{
  G_OBJECT_CLASS (class)->dispose = teleport_window_dispose;

  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/com/frac_tion/teleport/window.ui");

  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TeleportWindow, gears);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TeleportWindow, this_device_name_label);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TeleportWindow, remote_no_devices);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), TeleportWindow, remote_devices_box);
}

TeleportWindow *
teleport_window_new (TeleportApp *app)
{
  return g_object_new (TELEPORT_WINDOW_TYPE, "application", app, NULL);
}

gchar * 
teleport_get_device_name (void) 
{
  TeleportWindowPrivate *priv;
  priv = teleport_window_get_instance_private (mainWin);

  return g_settings_get_string (priv->settings, "device-name");
}

gchar * 
teleport_get_download_directory (void) 
{
  TeleportWindowPrivate *priv;
  priv = teleport_window_get_instance_private (mainWin);

  return g_settings_get_string (priv->settings, "download-dir");
}

void
teleport_show_no_device_message (TeleportWindow *self)
{
  TeleportWindowPrivate *priv;
  priv = teleport_window_get_instance_private (self);
  gtk_widget_show (priv->remote_no_devices);
}

void
teleport_window_open (TeleportWindow *win,
                      GFile *file)
{
  //TeleportWindowPrivate *priv;
  //priv = teleport_window_get_instance_private (win);
}
