#include <gio/gio.h>

#define MPLAYER2_OBJECT_PATH "/org/mpris/MediaPlayer2"
#define MPLAYER2_INTERFACE "org.mpris.MediaPlayer2.Player"
/**
 * Parses given GVariant result and returns the active player
 */
gchar* get_active_bus_name(GVariant* names)
{
    gchar** name_list;
    g_variant_get(names, "(^as)", &name_list);
    gchar* active_player = NULL;
    for (int i = 0; name_list[i] != NULL; i++) {
        if (g_str_has_prefix(name_list[i], "org.mpris.MediaPlayer2.")) {
            active_player = g_strdup(name_list[i]);
            g_print("PLAYER: %s\n", name_list[i]);
            break;
        }
    }
    g_strfreev(name_list);
    return active_player;
}

gchar* get_active_audio_dbus(GDBusConnection* connection, GError* error)
{
    const char* service_name = "org.freedesktop.DBus";
    const char* object_path = "/org/freedesktop/DBus";
    const char* interface_name = "org.freedesktop.DBus";

    GVariant* names = g_dbus_connection_call_sync(
        connection,
        service_name,
        object_path,
        interface_name,
        "ListNames",
        NULL,
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error);

    if (error != NULL) {
        g_print("Error getting list of names: %s\n", error->message);
        g_error_free(error);
        g_object_unref(connection);
        return NULL;
    }
    gchar* active_player = get_active_bus_name(names);
    g_variant_unref(names);
    // Check if a media player was found
    if (active_player == NULL) {
        return NULL;
    }
    return active_player;
}

int main()
{
    GError* error = NULL;
    g_type_init();
    GDBusConnection* connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (error != NULL) {
        g_print("Error connecting to bus: %s\n", error->message);
        g_error_free(error);
        return 1;
    }
    // Specify the target service and object path
    gchar* active_bus = get_active_audio_dbus(connection, error);
    if (!active_bus) {
        g_print("No compatible media player found.\n");
        exit(1);
    }

    // Build the method call
    GVariant* result = g_dbus_connection_call_sync(
        connection,
        active_bus,
        MPLAYER2_OBJECT_PATH,
        MPLAYER2_INTERFACE,
        "PlayPause",
        NULL,
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error);

    if (error != NULL) {
        g_print("Error calling method: %s\n", error->message);
        g_error_free(error);
    } else {
        g_variant_unref(result);
        g_print("Method called successfully.\n");
    }

    // Clean up
    g_free(active_bus);
    g_object_unref(connection);

    return 0;
}
