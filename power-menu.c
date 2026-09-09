//Power menu application


/*INCLUDES*/

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <stdio.h>


/*STRUCTURES*/

//Configuration
typedef struct {
	int width;
	int height;
	char *reboot;
	char *poweroff;
	char *lock;
	char *suspend;
	int x_pos;
	int y_pos;
	bool has_position;
	char *label_align;
	char *reboot_icon;
	char *poweroff_icon;
	char *lock_icon;
	char *suspend_icon;
	int spacing;
} Config;

//Options
typedef struct {
	const char *label;
	char *icon;
} Icons;

//App
typedef struct {
	GtkApplication *application;
	GtkWindow *window;
	GtkListBox *listbox;
	Config config;
} App;

void reboot(App *app);
void poweroff(App *app);
void lock(App *app);
void suspend(App *app);


//Options
typedef struct {
	const char *label;
	void (*action)(App *app);
} MenuOption;


/*VARIABLES*/

#define OPTION_COUNT 4

//Options
MenuOption options[] = {
	{"Reboot", reboot},
	{"Power off", poweroff},
	{"Lock", lock},
	{"Suspend", suspend}
};

//Icons
static char *icons[OPTION_COUNT] = {0};

/*FUNCTIONS*/

//Goes through the configuration file
static int load_config(const char *config_file, Config *config) {
	
	//Open file
	FILE *f = fopen(config_file, "r");

	//If f is NULL: return the fail flag
	if (f == NULL) {
    		return 1;
	}
	
	//Line buffer
	char line[512];

	//Go through each line in the file and assign
	//its value (if there is one)
	while (fgets(line, sizeof(line), f) != NULL) {
	
		if (strncmp(line, "width=", 6) == 0) {
			char *width = line + 6;
			width[strcspn(width, "\n")] = '\0';
			config->width = atoi(width);
		}

		if (strncmp(line, "height=", 7) == 0) {
			char *height = line + 7;
			height[strcspn(height, "\n")] = '\0';
			config->height = atoi(height);
		}
		
		if (strncmp(line, "reboot=", 7) == 0) {
			char *reboot = line + 7;
			reboot[strcspn(reboot, "\n")] = '\0';
			config->reboot = strdup(reboot);
		}

		if (strncmp(line, "poweroff=", 9) == 0) {
			char *poweroff = line + 9;
			poweroff[strcspn(poweroff, "\n")] = '\0';
			config->poweroff = strdup(poweroff);
		}

		if (strncmp(line, "lock=", 5) == 0) {
			char *lock = line + 5;
			lock[strcspn(lock, "\n")] = '\0';	
			config->lock = strdup(lock);
		}

		if (strncmp(line, "suspend=", 8) == 0) {
			char *suspend = line + 8;
			suspend[strcspn(suspend, "\n")] = '\0';
			config->suspend = strdup(suspend);
		}

		if (strncmp(line, "x_pos=", 6) == 0) {
			char *x_pos = line + 6;
			x_pos[strcspn(x_pos, "\n")] = '\0';
			config->x_pos = atoi(x_pos);
			config->has_position = true;
		}

		if (strncmp(line, "y_pos=", 6) == 0) {
			char *y_pos = line + 6;
			y_pos[strcspn(y_pos, "\n")] = '\0';
			config->y_pos = atoi(y_pos);
			config->has_position = true;
		}
					
		if (strncmp(line, "label_align=", 12) == 0) {
			char *label_align = line + 12;
			label_align[strcspn(label_align, "\n")] = '\0';
			config->label_align = strdup(label_align);
		}
						
		if (strncmp(line, "reboot_icon=", 12) == 0) {
			char *reboot_icon = line + 12;
			reboot_icon[strcspn(reboot_icon, "\n")] = '\0';
			icons[0] = strdup(reboot_icon);
		}
				
		if (strncmp(line, "poweroff_icon=", 14) == 0) {
			char *poweroff_icon = line + 14;
			poweroff_icon[strcspn(poweroff_icon, "\n")] = '\0';
			icons[1] = strdup(poweroff_icon);
		}
				
		if (strncmp(line, "lock_icon=", 10) == 0) {
			char *lock_icon = line + 10;
			lock_icon[strcspn(lock_icon, "\n")] = '\0';
			icons[2] = strdup(lock_icon);
		}
				
		if (strncmp(line, "suspend_icon=", 13) == 0) {
			char *suspend_icon = line + 13;
			suspend_icon[strcspn(suspend_icon, "\n")] = '\0';
			icons[3] = strdup(suspend_icon);
		}

		if (strncmp(line, "spacing=", 8) == 0) {
			char *spacing = line + 8;
			spacing[strcspn(spacing, "\n")] = '\0';
			config->spacing = atoi(spacing);
		}

	}
	
	//Close the file and return the success flag
	fclose(f);
	return 0;

}

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
		option->action(app);

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
	GtkCssProvider *user_css;
	char *exe;
	char *dir;
	char *config;
	char *user_config;
	char *style;
	char *user_style;

	//Creates window
	app->window = GTK_WINDOW(gtk_application_window_new(app->application));
	gtk_widget_add_css_class(GTK_WIDGET(app->window), "power-menu");

	//Sets window as layer shell
	gtk_layer_init_for_window(app->window);
	gtk_layer_set_namespace(app->window, "power-menu");

	//Focuses the keyboard on the window
	gtk_layer_set_keyboard_mode(app->window, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

	//Window title
	gtk_window_set_title(app->window, "power-menu");

	//Listbox for rows
	app->listbox = GTK_LIST_BOX(gtk_list_box_new());
	gtk_window_set_child(app->window,GTK_WIDGET(app->listbox));
	gtk_widget_add_css_class(GTK_WIDGET(app->listbox), "power-listbox");


	//Keyboard controller
	controller = gtk_event_controller_key_new();
	g_signal_connect(controller,"key-pressed",G_CALLBACK(on_key_pressed), app);
	gtk_widget_add_controller(GTK_WIDGET(app->window), controller);

	//Executable relative path
	exe = g_file_read_link("/proc/self/exe", NULL);

	//Executable directory
	dir = g_path_get_dirname(exe);


	/* DEFAULT CONFIGURATION */

	//Default config file
	config = g_build_filename(dir, "config", "power-menu.conf", NULL);
	
	if (load_config(config, &app->config) != 0) {
		printf("default config file not found at %s\n", dir);
		exit(1);
	}

	//Default style.css
	style = g_build_filename(dir, "config", "style.css", NULL);

	//Check if the default style is there
	if(g_file_test(style, G_FILE_TEST_EXISTS)) {

		//Default CSS style provider
		css = gtk_css_provider_new();
	
		//Load the style.css for the provider
		gtk_css_provider_load_from_path(css, style);

		//Add style provider for display (whatever that means)
		gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(app->window)), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	
		//Cleanup
		g_object_unref(css);
	}
	else {
		printf("default style file not found in %s\n", dir);
	}
	
	g_free(exe);
	g_free(dir);
	g_free(config);
	g_free(style);


	/* USER CONFIGURATION */

	//User config file
	user_config = g_build_filename(g_get_user_config_dir(), "power-menu", "power-menu.conf", NULL);

	if (load_config(user_config, &app->config) != 0) {
		printf("user config file not found at ~/.config/power-menu/\n");
	}

	//user style.css
	user_style = g_build_filename(g_get_user_config_dir(), "power-menu", "style.css", NULL);

	//If the file exists, just do the same as before
	if(g_file_test(user_style, G_FILE_TEST_EXISTS)) {
	
		user_css = gtk_css_provider_new();
	
		gtk_css_provider_load_from_path(user_css, user_style);

		gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(app->window)), GTK_STYLE_PROVIDER(user_css), GTK_STYLE_PROVIDER_PRIORITY_USER);

		g_object_unref(user_css);
	
	}
	else {
		printf("user style file not found in ~/.config/power-menu\n");
	}


	g_free(user_config);
	g_free(user_style);

	//Window size
	gtk_window_set_default_size(app->window, app->config.width, app->config.height);

	//Window position	
	if (app->config.has_position) {
		
		//Set the anchor point
		gtk_layer_set_anchor(app->window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
		gtk_layer_set_anchor(app->window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);	

		//Set the position
		gtk_layer_set_margin(app->window, GTK_LAYER_SHELL_EDGE_TOP, app->config.x_pos);
		gtk_layer_set_margin(app->window, GTK_LAYER_SHELL_EDGE_LEFT, app->config.y_pos);

	}

}

//Creates a row to put inside the window
void create_row(App *app, const MenuOption *option, char *icon) {

	//Local Variables
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *row;

	//Horizontal box
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, app->config.spacing);

	//Icon
	if (icon != NULL) {

		GtkWidget *option_icon = gtk_label_new(icon);
		gtk_widget_set_hexpand(option_icon, FALSE);
		gtk_box_append(GTK_BOX(hbox), option_icon);
		gtk_widget_add_css_class(option_icon, "power-icon");

	}

	//Label
	label = gtk_label_new(option->label);
	gtk_widget_set_hexpand(label, TRUE);
	gtk_box_append(GTK_BOX(hbox), label);
	gtk_widget_add_css_class(label, "power-label");

	if (strcmp(app->config.label_align,"left") == 0) {		
		gtk_widget_set_halign(label, GTK_ALIGN_START);
	}
	if (strcmp(app->config.label_align,"center") == 0) {
		gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
	}
	if (strcmp(app->config.label_align,"right") == 0) {
		gtk_widget_set_halign(label, GTK_ALIGN_END);
	}

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
	for (int i = 0; i < OPTION_COUNT; i++) {
		create_row(app, &options[i], icons[i]);
	}

	//Present window
	gtk_window_present(app->window);

}

//Action functions
void reboot(App *app) {
	system(app->config.reboot);
}
void poweroff(App *app) {
	system(app->config.poweroff);
}
void lock(App *app) {
	system(app->config.lock);
}
void suspend(App *app) {
    system(app->config.suspend);
}

//Main loop
int main(int argc, char *argv[]) {

	//Initial app structure
	App app = {0};
	app.config.has_position = false;

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
