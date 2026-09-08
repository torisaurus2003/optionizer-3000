//Power menu application


/*INCLUDES*/

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>


/*FUNCTION PROTOTYPES*/

void reboot(void);
void poweroff(void);
void lock(void);
void suspend(void);


/*STRUCTURES*/

//Options
typedef struct {
	const char *label;
	void (*action)(void);
} MenuOption;


//App
typedef struct {
	GtkApplication *application;
	GtkWindow *window;
	GtkListBox *listbox;
} App;


/*VARIABLES*/

//Options
MenuOption options[] = {
	{"Reboot", reboot},
	{"Power off", poweroff},
	{"Lock", lock},
	{"Suspend", suspend}
};

//Number of elements in options
const int option_count = sizeof(options) / sizeof(options[0]);


/*FUNCTIONS*/

//Key handler
gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {

	//Unused arguments
	(void)controller;
	(void)keycode;
	(void)state;

	//Local variables
	App *app = user_data;
	GtkListBoxRow *row;
	MenuOption *option;

	//Detects the currently focused window
	row = gtk_list_box_get_selected_row(app->listbox);

	//Checks the pressed key
	if (keyval == GDK_KEY_Return) {

		//If enter is pressed, close the window and execute
		//the corresponding action function
		option = g_object_get_data(G_OBJECT(row), "option");
		gtk_window_destroy(app->window);
		option->action();

	}
	else if (keyval == GDK_KEY_Escape) {

		//If escape is pressed, close the window
		gtk_window_destroy(app->window);

	}

	return TRUE;
	
}

//Creates the window
void create_window(App *app) {

	//Local variables
	GtkEventController *controller;
	GtkCssProvider *css;
	char *css_path;

	//Creates window
	app->window = GTK_WINDOW(gtk_application_window_new(app->application));
	gtk_widget_add_css_class(GTK_WIDGET(app->window), "power-menu");

	//Sets window as layer shell
	gtk_layer_init_for_window(app->window);
	gtk_layer_set_namespace(app->window, "power-menu");

	//Focuses the keyboard on the window
	gtk_layer_set_keyboard_mode(app->window, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

	//Window size
	gtk_window_set_default_size(app->window, 120, 1);

	//Window title
	gtk_window_set_title(app->window, "power-menu");

	//Listbox for rows
	app->listbox = GTK_LIST_BOX(gtk_list_box_new());
	gtk_window_set_child(app->window,GTK_WIDGET(app->listbox));

	//Keyboard controller
	controller = gtk_event_controller_key_new();
	g_signal_connect(controller,"key-pressed",G_CALLBACK(on_key_pressed), app);
	gtk_widget_add_controller(GTK_WIDGET(app->window), controller);

	//Load CSS configuration
	css = gtk_css_provider_new();
	css_path = g_build_filename(g_get_user_config_dir(), "power-menu", "style.css", NULL);
	gtk_css_provider_load_from_path(css, css_path);
	gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(app->window)), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	//Free pointer variables
	g_free(css_path);
	g_object_unref(css);

}

//Creates a row to put inside the window
void create_row(App *app, const MenuOption *option) {

	//Local Variables
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *row;

	//Horizontal box
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_widget_add_css_class(hbox, "power-hbox");

	//Label
	label = gtk_label_new(option->label);
	gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
	gtk_widget_set_hexpand(label, TRUE);
	gtk_box_append(GTK_BOX(hbox), label);

	//Row
	row = gtk_list_box_row_new();
	gtk_widget_add_css_class(row, "power-row");
	gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), hbox);
	g_object_set_data(G_OBJECT(row), "option", (gpointer)option);
	gtk_list_box_append(app->listbox, row);

}

//Activates the app
void activate(GtkApplication *application, gpointer user_data) {

	//Assign arguments to app structure
	App *app = user_data;
	app->application = application;

	//Create window
	create_window(app);

	//Create rows
	for (int i = 0; i < option_count; i++) {
		create_row(app, &options[i]);
	}

	//Present window
	gtk_window_present(app->window);

}

//Action functions
void reboot(void) {
    system("systemctl reboot");
}

void poweroff(void) {
    system("systemctl poweroff");
}

void lock(void) {
    system("pgrep -x hyprlock || hyprlock");
}

void suspend(void) {
    system("systemctl suspend");
}

//Main loop
int main(int argc, char *argv[]) {

	//Initial app structure
	App app = {0};

	//Create application
	app.application = gtk_application_new("com.torisaurus.power-menu", G_APPLICATION_DEFAULT_FLAGS);

	//Connect application to activate
	g_signal_connect(app.application, "activate", G_CALLBACK(activate), &app);

	//Initialize application
	int status = g_application_run(G_APPLICATION(app.application), argc, argv);

	//Unreference application
	g_object_unref(app.application);

	//Return exit code from status
	return status;

}
