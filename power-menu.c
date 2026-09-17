//Power menu application


/*INCLUDES*/

#include <gtk/gtk.h>		//For GTK stuff
#include <gtk4-layer-shell.h>	//For layer shell stuff
#include <stdio.h>		//For printing errors
#include <string.h>		//For strcmp and strncmp
#include <stdlib.h>		//For free and calloc

/*VARIABLES*/

//Configuration
typedef struct {
	int width;		//Window width
	int height;		//Window height
	int x_pos;		//Horizontal window position
	int y_pos;		//Vertical window position
	bool has_position;	//Flag to check if there's x_pos or y_pos
	char *label_align;	//Alignment of the option label
	int spacing;		//Space between icon and label
} Config;

//Option
typedef struct Option {
	char *icon;	//Option icon
	char *label;	//Option label
	char *command;	//Option command
} Option;

//App
typedef struct {
	GtkApplication *application;	//App object
	GtkWindow *window;		//App window
	GtkListBox *listbox;		//Window listbox
	Config config;			//Window configuration
	size_t label_count;		//Number of labels in the config file
	Option *options;		//Window options
} App;

//Section
typedef enum {
	NONE,		//Default
	OPTIONS,	//Options section of the config file
	CONFIG		//Configuration section of the config file
} Section;

Section section = NONE;

/*FUNCTIONS*/

//It gets the amount of labels in the config file
static size_t get_label_count(const char *config_file, App *app) {

	//Open file
	FILE *f = fopen(config_file, "r");
	
	//Line buffer
	char line[512];

	//Go through each line in the file and assign
	while (fgets(line, sizeof(line), f) != NULL) {
	
		//Replacing the newline terminator for a null one
		line[strcspn(line, "\n")] = '\0';

		//Check if we are in the labels line
		if (strncmp(line, "labels=", 7) == 0) {
			
			//Labels
			char *label = strtok(line + 7, ",");

			//Label count
			int label_count = 0;

			//Count the amount of options
			while (label != NULL) {
    				label_count++;
    				label = strtok(NULL, ",");
			}
			
			fclose(f);
			app->label_count = label_count;
			return 0;
			break;

		}

	}
	
	fclose(f);
	return 1;

}

//Goes through the configuration file
static int load_config(const char *config_file, App *app) {
	
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
	
		//Change the string terminator from new line to null
		line[strcspn(line, "\n")] = '\0';

		//Line length
		size_t length = strlen(line);

		//Check if the line is a section header
		if (length >= 2 && line[0] == '[' && line[length - 1] == ']') {
			
			//Remove the [
			char *section_name = line + 1;

			//Remove the ]
			section_name[strlen(section_name) - 1] = '\0';
			
			//Check if we are in the options section
			if (strcmp(section_name, "options") == 0) {
				section = OPTIONS;
			}

			//Check if we are in the options section
			if (strcmp(section_name, "config") == 0) {
				section = CONFIG;
			}

			continue;

		}

		//Check if we are in the options section
		if (section == OPTIONS) {
			
			//Check if we are on the icons line
			if (strncmp(line, "icons=", 6) == 0) {
					
				//Icons list
				char *icon = strtok(line + 6, ",");

				//Index
				size_t i = 0;

				//Assign each icon
				while (icon != NULL && i < app->label_count) {
					app->options[i].icon = strdup(icon);
    					i++;
    					icon = strtok(NULL, ",");
				}

			}


			//Check if we are on the labels line
			if (strncmp(line, "labels=", 7) == 0) {
					
				//Label list
				char *label = strtok(line + 7, ",");

				//Index
				size_t i = 0;

				//Assign each label
				while (label != NULL && i < app->label_count) {
					app->options[i].label = strdup(label);
    					i++;
    					label = strtok(NULL, ",");
				}

			}

			//Check if we are on the commands line
			if (strncmp(line, "commands=", 9) == 0) {
					
				//Commands list
				char *command = strtok(line + 9, ",");

				//Index
				size_t i = 0;

				//Assign each command
				while (command != NULL && i < app->label_count) {
					app->options[i].command = strdup(command);
    					i++;
    					command = strtok(NULL, ",");
				}

			}

		}
		

		if (section == CONFIG) {

			if (strncmp(line, "width=", 6) == 0) {
				char *width = line + 6;
				app->config.width = atoi(width);
			}

			if (strncmp(line, "height=", 7) == 0) {
				char *height = line + 7;
				app->config.height = atoi(height);
			}
		
			if (strncmp(line, "x_pos=", 6) == 0) {
				char *x_pos = line + 6;
				app->config.x_pos = atoi(x_pos);
				app->config.has_position = true;
			}

			if (strncmp(line, "y_pos=", 6) == 0) {
				char *y_pos = line + 6;
				app->config.y_pos = atoi(y_pos);
				app->config.has_position = true;
			}
				
			if (strncmp(line, "label_align=", 12) == 0) {
				char *label_align = line + 12;
				app->config.label_align = strdup(label_align);
			}

			if (strncmp(line, "spacing=", 8) == 0) {
				char *spacing = line + 8;
				app->config.spacing = atoi(spacing);
			}

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
	Option *option;

	//Detects the currently focused window
	row = gtk_list_box_get_selected_row(app->listbox);
	
	//Checks the pressed key
	if (keyval == GDK_KEY_Return) {

		option = g_object_get_data(G_OBJECT(row), "option");

		//Check if the option has a command
		if (option != NULL && option->command != NULL) {

			//If enter is pressed, close the window
			//and execute the associated command
			gtk_window_destroy(app->window);
			system(option->command);

		}
		else {
			gtk_window_destroy(app->window);
			printf("No command associated with \"%s\"\n", option->label);
		}

	}

	if (keyval == GDK_KEY_Escape) {

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
	char *config;
	char *style;

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
	g_signal_connect(controller,"key-pressed", G_CALLBACK(on_key_pressed), app);
	gtk_widget_add_controller(GTK_WIDGET(app->window), controller);

	
	//Check if there's a config file
	config = g_build_filename(g_get_user_config_dir(), "power-menu", "config", NULL);
	
	//Check what load_config returns
	if (g_file_test(config, G_FILE_TEST_EXISTS)) {	

		//Set the amount of options and check
		//if get_label_count found no labels
		if (get_label_count(config, app) != 0) {
		
			printf("No label settings found\n");
			exit(1);
		
		}

		//Allocate space for the options
		app->options = calloc(app->label_count, sizeof(Option));

		//Load the config file
		load_config(config, app);
		
	
	}	
	else {

		//If no config file is found
		printf("config file not found at %s\n", config);

	}
	
	//Style file
	style = g_build_filename(g_get_user_config_dir(), "power-menu", "style.css", NULL);

	//Check if the style file is there
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
		printf("style file not found at %s\n", style);
	}
	
	g_free(config);
	g_free(style);

	//Window size
	gtk_window_set_default_size(app->window, app->config.width, app->config.height);

	//Window position	
	if (app->config.has_position) {
		
		//Set the anchor point
		gtk_layer_set_anchor(app->window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
		gtk_layer_set_anchor(app->window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);	

		//Set the position
		gtk_layer_set_margin(app->window, GTK_LAYER_SHELL_EDGE_TOP, app->config.y_pos);
		gtk_layer_set_margin(app->window, GTK_LAYER_SHELL_EDGE_LEFT, app->config.x_pos);

	}

}

//creates a row to put inside the window
void create_row(App *app, Option *option) {
	
	//local variables
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *row;
	
	//horizontal box
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, app->config.spacing);

	
	//icon
	if (option->icon != NULL && strcmp(option->icon, " ") != 0) {
		GtkWidget *icon = gtk_label_new(option->icon);
		gtk_widget_set_hexpand(icon, FALSE);
		gtk_box_append(GTK_BOX(hbox), icon);
		gtk_widget_set_halign(icon, GTK_ALIGN_CENTER);
		gtk_widget_add_css_class(icon, "power-icon");
	}

	//label
	if (option->label != NULL && strcmp(option->label, " ") != 0) {
	
		label = gtk_label_new(option->label);
		gtk_widget_set_hexpand(label, TRUE);
		gtk_box_append(GTK_BOX(hbox), label);
		gtk_widget_add_css_class(label, "power-label");

		if (strcmp(app->config.label_align, "left") == 0) {
			gtk_widget_set_halign(label, GTK_ALIGN_START);
		}
		else if (strcmp(app->config.label_align, "center") == 0 || strcmp(app->config.label_align, "default") == 0) {
			gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
		}
		else if (strcmp(app->config.label_align, "right") == 0) {
			gtk_widget_set_halign(label, GTK_ALIGN_END);
		}
	}

	//row
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
	for (size_t i = 0; i < app->label_count; i++) {
		create_row(app, &app->options[i]);
	}

	//Present window
	gtk_window_present(app->window);

}


//Main loop
int main(int argc, char *argv[]) {

	//Initial app structure
	App app = {0};

	//Default app configuration
	app.config.width = 100;
	app.config.height = 1;
	app.config.has_position = false;
	app.config.label_align = strdup("center");
	app.config.spacing = 0;

	//Create application
	app.application = gtk_application_new("com.torisaurus.power-menu", G_APPLICATION_DEFAULT_FLAGS);

	//Connect application to activate
	g_signal_connect(app.application, "activate", G_CALLBACK(activate), &app);

	//Initialize application
	int status = g_application_run(G_APPLICATION(app.application), argc, argv);

	//Cleanup
	g_object_unref(app.application);

	for (size_t i = 0; i < app.label_count; i++) {
    		free(app.options[i].icon);
    		free(app.options[i].label);
    		free(app.options[i].command);
	}

	free(app.options);
	free(app.config.label_align);
	
	//Return exit code from status
	return status;

}
