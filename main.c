#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EARTH_RADIUS 6371.0
#define GEO_ALTITUDE 35786.0

// азимут
double calculate_azimuth(double lat_user, double lon_user, double lon_sat) {
    double delta_lon = (lon_sat - lon_user) * M_PI / 180.0;
    double y = cos(lat_user * M_PI / 180.0) * sin(delta_lon);
    double x = sin(lat_user * M_PI / 180.0) * cos(0) - cos(lat_user * M_PI / 180.0) * sin(0) * cos(delta_lon);
    double azimuth = atan2(y, x) * 180.0 / M_PI;
    return fmod(azimuth + 360.0, 360.0); // нормализация 
}

// углол места
double calculate_elevation(double lat_user, double lon_user, double lon_sat) {
    double delta_lon = (lon_sat - lon_user) * M_PI / 180.0;
    double theta = acos(cos(delta_lon));
    double elevation = atan2(GEO_ALTITUDE, sqrt(pow(EARTH_RADIUS, 2) + pow(GEO_ALTITUDE, 2) - 2 * EARTH_RADIUS * GEO_ALTITUDE * cos(theta)));
    return elevation * 180.0 / M_PI;
}

// координаты в десятичные градусы
double parse_coordinates(const char *deg, const char *min, const char *sec) {
    double d = atof(deg);
    double m = atof(min) / 60.0;
    double s = atof(sec) / 3600.0;
    return d + m + s;
}

// виджеты
typedef struct {
    GtkWidget *lat_deg_entry;
    GtkWidget *lat_min_entry;
    GtkWidget *lat_sec_entry;
    GtkWidget *lon_deg_entry;
    GtkWidget *lon_min_entry;
    GtkWidget *lon_sec_entry;
    GtkWidget *sat_combo;
    GtkWidget *result_label;
} Widgets;

// кнопка Рассчитать
void on_calculate_button_clicked(GtkWidget *widget, gpointer data) {
    Widgets *widgets = (Widgets *)data;

    const char *lat_deg = gtk_entry_get_text(GTK_ENTRY(widgets->lat_deg_entry));
    const char *lat_min = gtk_entry_get_text(GTK_ENTRY(widgets->lat_min_entry));
    const char *lat_sec = gtk_entry_get_text(GTK_ENTRY(widgets->lat_sec_entry));
    const char *lon_deg = gtk_entry_get_text(GTK_ENTRY(widgets->lon_deg_entry));
    const char *lon_min = gtk_entry_get_text(GTK_ENTRY(widgets->lon_min_entry));
    const char *lon_sec = gtk_entry_get_text(GTK_ENTRY(widgets->lon_sec_entry));
    char *satellite = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets->sat_combo));

    if (!lat_deg || !lat_min || !lat_sec || !lon_deg || !lon_min || !lon_sec || !satellite) {
        gtk_label_set_text(GTK_LABEL(widgets->result_label), "Ошибка: заполните все поля!");
        if (satellite) g_free(satellite);
        return;
    }

    double lat_user = parse_coordinates(lat_deg, lat_min, lat_sec);
    double lon_user = parse_coordinates(lon_deg, lon_min, lon_sec);

    char *comma_pos = strchr(satellite, ',');
    if (!comma_pos || comma_pos == satellite) {
        gtk_label_set_text(GTK_LABEL(widgets->result_label), "Ошибка: неверный формат данных спутника!");
        g_free(satellite);
        return;
    }

    double lon_sat = atof(comma_pos + 1);
    double azimuth = calculate_azimuth(lat_user, lon_user, lon_sat);
    double elevation = calculate_elevation(lat_user, lon_user, lon_sat);

    char result[256];
    snprintf(result, sizeof(result), "Азимут: %.2f°\nУгол места: %.2f°", azimuth, elevation);
    gtk_label_set_text(GTK_LABEL(widgets->result_label), result);

    g_free(satellite);
}

void apply_custom_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    
    const gchar *css = 
        "* {"
        "background-color: #f0f3f6;"
        "color: #2c3e50;"
        "font-size: 14px;"
        "}"

        "box {"
        "   padding: 20px;"
        "}"
        "entry {"
        "   background-color: #ffffff;"
        "   border: 1px solid #d1d8e0;"
        "   border-radius: 6px;"
        "   padding: 10px;"
        "   margin: 5px;"
        "   transition: all 0.3s ease;"
        "}"
        "entry:focus {"
        "   border-color: #3498db;"
        "   box-shadow: 0 0 5px rgba(52,152,219,0.3);"
        "}"
        "label {"
        "   color: #2c3e50;"
        "   margin-bottom: 5px;"
        "}"
        "button {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   transition: background-color 0.3s;"
        "}"
        "button:hover {"
        "   background-color: #2980b9;"
        "}"
        ".result-label {"
        "   background-color: #ecf0f1;"
        "   border-radius: 6px;"
        "   padding: 10px;"
        "   margin-top: 10px;"
        "}";

    GdkScreen *screen = gdk_screen_get_default();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        screen, 
        GTK_STYLE_PROVIDER(provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

void activate(GtkApplication *app, gpointer user_data) {
    // главное окно
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Satellite Tracker");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);
    
    apply_custom_css(window);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    // заголовок
    GtkWidget *header = gtk_label_new("Спутниковый Трекер");
    PangoAttrList *attr_list = pango_attr_list_new();
    PangoAttribute *attr = pango_attr_scale_new(1.5);
    pango_attr_list_insert(attr_list, attr);
    gtk_label_set_attributes(GTK_LABEL(header), attr_list);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 10);

    GtkWidget *grid_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), grid_container, TRUE, TRUE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(grid_container), grid, TRUE, TRUE, 0);

    // виджеты
    Widgets *widgets = g_new0(Widgets, 1);

    // Широта
    GtkWidget *lat_label = gtk_label_new("Широта (северная):");
    widgets->lat_deg_entry = gtk_entry_new();
    widgets->lat_min_entry = gtk_entry_new();
    widgets->lat_sec_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->lat_deg_entry), "Градусы");
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->lat_min_entry), "Минуты");
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->lat_sec_entry), "Секунды");

    // Долгота
    GtkWidget *lon_label = gtk_label_new("Долгота (восточная):");
    widgets->lon_deg_entry = gtk_entry_new();
    widgets->lon_min_entry = gtk_entry_new();
    widgets->lon_sec_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->lon_deg_entry), "Градусы");
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->lon_min_entry), "Минуты");
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->lon_sec_entry), "Секунды");

    // Спутник
    GtkWidget *sat_label = gtk_label_new("Спутник:");
    widgets->sat_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->sat_combo), "Спутник 1,10.0");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->sat_combo), "Спутник 2,20.0");

    // Кнопка и результат
    GtkWidget *calculate_button = gtk_button_new_with_label("Рассчитать");
    widgets->result_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(widgets->result_label), TRUE);
    gtk_widget_set_name(widgets->result_label, "result-label");

    gtk_grid_attach(GTK_GRID(grid), lat_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->lat_deg_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->lat_min_entry, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->lat_sec_entry, 3, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), lon_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->lon_deg_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->lon_min_entry, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->lon_sec_entry, 3, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), sat_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->sat_combo, 1, 2, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), calculate_button, 0, 3, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->result_label, 0, 4, 4, 1);

    g_signal_connect(calculate_button, "clicked", G_CALLBACK(on_calculate_button_clicked), widgets);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.satellite_tracker", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
