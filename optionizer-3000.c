//Power menu application


/*INCLUDES*/

#include <gtk/gtk.h>		//For GTK stuff
#include <gtk4-layer-shell.h>	//For layer shell stuff
#include <stdio.h>		//For printing errors
#include <string.h>		//For strcmp and strncmp
#include <stdlib.h>		//For free and calloc
#include <getopt.h>		//For the option handling
#include <stdbool.h>		//For boolean support

/*VARIABLES*/

//Configuration
typedef struct {
	int width;		//Window width
	int height;		//Window height
	bool has_position;	//Flag to check if there's x_pos or y_pos
	char *label_align;	//Alignment of the option label
	int label_spacing;	//Space between icon and label
	int listbox_spacing;	//Space between listboxes
	size_t max_rows;	//Maximum number of rows per listbox
	char *title;		//Window title
	bool set_as_layer;	//Flag to check the window as layer
	char *namespace;	//Layer namespace
	int x_pos;		//Horizontal window position
	int y_pos;		//Vertical window position
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
	GtkBox *box;			//Window box
	GtkListBox **listboxes;		//Window listboxes
	Config config;			//Window configuration
	size_t label_count;		//Number of labels in the config file
	size_t listbox_count;		//Number of listboxes
	Option *options;		//Window options
	char *config_file;		//Configuration file path
	char *style_file;		//Style file path
	GtkCssProvider *css;		//CSS provider
	GFile *file;			//Style file object
	GFileMonitor *monitor;		//Style file monitor
} App;

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

	//If the file doesn't exist return the fail flag
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
		
		//Check if we are on the width line
		if (strncmp(line, "width=", 6) == 0) {

			//Clean the line and store width as an int
			char *width = line + 6;
			app->config.width = atoi(width);

		}

		//Check if we are on the height line
		if (strncmp(line, "height=", 7) == 0) {

			//Clean the line and store height as an int
			char *height = line + 7;
			app->config.height = atoi(height);

		}
			
		//Check if we are on the label_align line	
		if (strncmp(line, "label_align=", 12) == 0) {

			//Clean the line and pass ownership
			//of the string to app
			char *label_align = line + 12;
			app->config.label_align = strdup(label_align);
			
		}

		//Check if we are on the label_spacing line
		if (strncmp(line, "label_spacing=", 14) == 0) {

			//Clean the line and pass ownership
			//of the string to app
			char *label_spacing = line + 14;
			app->config.label_spacing = atoi(label_spacing);

		}
		
		//Check if we are on the listbox_spacing line
		if (strncmp(line, "listbox_spacing=", 16) == 0) {

			//Clean the line and pass ownership
			//of the string to app
			char *listbox_spacing = line + 16;
			app->config.listbox_spacing = atoi(listbox_spacing);

		}

		//Check if we are on the max_rows line
		if (strncmp(line, "max_rows=", 9) == 0) {

			char *end;	//End character of the string

			//Clean the string and turn it to unsigned long
			long max_rows = strtol(line + 9, &end, 10);
			
			//Check that the value is a positive integer
			if (end == line + 9 || *end != '\0' || max_rows <= 0) {

				//If the value is not a positive integer, print
				//the error message and return the fail flag
				printf("max_rows has to be a positive integer\n");
    				return 1;

			}
			
			//Pass max_rows to app as size_t
			app->config.max_rows = (size_t)max_rows;
		}

		//Check if we are on the title line
		if (strncmp(line, "title=", 6) == 0) {

			//Clean the line and pass ownership
			//of the string to app
			char *title = line + 6;
			app->config.title = strdup(title);

		}

		//Check if we are on the set_as_layer line
		if (strncmp(line, "set_as_layer=", 13) == 0) {

			//Clean the line and pass ownership
			//of the string to app
			char *set_as_layer = line + 13;

			//Set the layer shell flag
			if (strcmp(set_as_layer, "true") == 0) {
				app->config.set_as_layer = true;
			}

			if (strcmp(set_as_layer, "false") == 0) {
				app->config.set_as_layer = false;
			}

		}

		//Check if we are on the namespace line
		if (strncmp(line, "namespace=", 10) == 0) {

			//Clean the line and pass ownership
			//of the string to app
			char *namespace = line + 10;
			app->config.namespace = strdup(namespace);

		}

		//Check if we are on the x_pos line
		if (strncmp(line, "x_pos=", 6) == 0) {

			//Clean the line and store x_pos as an int
			char *x_pos = line + 6;
			app->config.x_pos = atoi(x_pos);

			//Enable the has_position flag
			app->config.has_position = true;

		}

		//Check if we are on the y_pos line
		if (strncmp(line, "y_pos=", 6) == 0) {

			//Clean the line and store y_pos as an int
			char *y_pos = line + 6;
			app->config.y_pos = atoi(y_pos);

			//Enable the has_position flag
			app->config.has_position = true;

		}
	
	}
	
	//Close the file and return the success flag
	fclose(f);
	return 0;

}

//Key handler
static void on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {

	//Unused arguments
	(void)controller;
	(void)keycode;
	(void)state;

	//Local variables
	App *app = user_data;			//App
	GtkListBoxRow *row;			//Selected row
	GtkListBox *listbox = NULL;		//Selected row listbox	
	GtkListBox *next_listbox = NULL;	//Listbox right to the current listbox
	GtkListBox *prev_listbox = NULL;	//Listbox left to the current listbox
	
	//Iterate over all the listboxes
	for (size_t i = 0; i < app->listbox_count; i++) {
    		
		//Try to catch the row in the
		//currently iterated listbox
		row = gtk_list_box_get_selected_row(app->listboxes[i]);

		//Check if the slected row exists
    		if (row != NULL) {
			
			//Get the current listbox
			listbox = app->listboxes[i];

			//Check if the next listbox exists
			//if not, set it as the first one
			if (i + 1 < app->listbox_count) {
				next_listbox = app->listboxes[i + 1];
			}
			else {
				next_listbox = app->listboxes[0];
			}

			//Check if the previous listbox exists
			//if not, set it as the last one
			if (i > 0) {
				prev_listbox = app->listboxes[i - 1];
			}
			else {
				prev_listbox = app->listboxes[app->listbox_count - 1];
			}
			
			//Break the iteration
        		break;
		}

	}
	
	//If the right key is pressed
	if (keyval == GDK_KEY_Right) {
	
		//Current row index
		int row_index = gtk_list_box_row_get_index(row);

		//Get the row over the next listbox with the same index
		GtkListBoxRow *next_row = gtk_list_box_get_row_at_index(next_listbox, row_index);
	
		//If there's not a row next to the current
		//one, just jump to the first row of the
		//next listbox	
		while (! next_row) {
			row_index--;
			next_row = gtk_list_box_get_row_at_index(next_listbox, row_index);
		}
		
		//Pass the focus and selection to the next row
	    	gtk_list_box_unselect_all(listbox);
        	gtk_list_box_select_row(next_listbox, next_row);
        	gtk_widget_grab_focus(GTK_WIDGET(next_row));

	}
	
	//If the left key is pressed
	if (keyval == GDK_KEY_Left) {
		
		//Current row index
		int row_index = gtk_list_box_row_get_index(row);

		//Get the row over the previous listbox with the same index
		GtkListBoxRow *prev_row = gtk_list_box_get_row_at_index(prev_listbox, row_index);
		
		//Pass the focus and selection to the previous row
	    	if (prev_row != NULL) {
			gtk_list_box_unselect_all(listbox);
        		gtk_list_box_select_row(prev_listbox, prev_row);
        		gtk_widget_grab_focus(GTK_WIDGET(prev_row));
    		}

	}
	
	//If the up key is pressed
	if (keyval == GDK_KEY_Up) {
		
		//Current row index
		int row_index = gtk_list_box_row_get_index(row);

		//If we are at the first row of the listbox
		if (row_index == 0) {

			//Last index for any listbox
			int prev_index = app->config.max_rows - 1;
			
			//Get the last possible row from the previous listbox
			GtkListBoxRow *prev_row = gtk_list_box_get_row_at_index(prev_listbox, prev_index);

			//If the previous row isn't at that index,
			//keep decreasing it until there's one
			while (! prev_row) {
				prev_index--;
				prev_row = gtk_list_box_get_row_at_index(prev_listbox, prev_index);
			}

			//Pass the selection to the previous row
			gtk_list_box_unselect_all(listbox);
        		gtk_list_box_select_row(prev_listbox, prev_row);
        		gtk_widget_grab_focus(GTK_WIDGET(prev_row));

		}
		else {
			
			//Get the previous row
			GtkListBoxRow *prev_row = gtk_list_box_get_row_at_index(listbox, row_index - 1);

			//Pass the selection to the previous row
			gtk_list_box_unselect_all(listbox);
        		gtk_list_box_select_row(listbox, prev_row);
        		gtk_widget_grab_focus(GTK_WIDGET(prev_row));
		
		}

	}

	//If the down key is pressed
	if (keyval == GDK_KEY_Down) {
		
		//Current row index
		int row_index = gtk_list_box_row_get_index(row);

		//Get the next row
		GtkListBoxRow *next_row = gtk_list_box_get_row_at_index(listbox, row_index + 1);

		//Check if the next row isn't there, if so, change
		//it to the first row of the next listbox
		if (! next_row) {
			
			GtkListBoxRow *next_row = gtk_list_box_get_row_at_index(next_listbox, 0);

			//Pass the selection to the next row
			gtk_list_box_unselect_all(listbox);
        		gtk_list_box_select_row(next_listbox, next_row);
        		gtk_widget_grab_focus(GTK_WIDGET(next_row));

		}
		else {
			
			//Pass the selection to the next row
			gtk_list_box_unselect_all(listbox);
        		gtk_list_box_select_row(listbox, next_row);
        		gtk_widget_grab_focus(GTK_WIDGET(next_row));
		
		}

	}

	//If the enter key is pressed
	if (keyval == GDK_KEY_Return) {
		
		//Get the option from the selected row
		Option *option = g_object_get_data(G_OBJECT(row), "option");

		//Check if the option exists and has a command
		if (option != NULL && option->command != NULL) {

			//If enter is pressed, close the window
			//and execute the associated command
			gtk_window_destroy(app->window);
			system(option->command);

		}
		else {

			//If the option doesn't have a command, destroy the
			//window and print an error message
			gtk_window_destroy(app->window);
			printf("No command associated with \"%s\"\n", option->label);

		}

	}

	//If the escape key is pressed
	if (keyval == GDK_KEY_Escape) {

		//Destroy the window
		gtk_window_destroy(app->window);

	}

}

//CSS monitor callback
static void css_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event, gpointer user_data) {

	//Unused arguments
	(void)monitor;
	(void)other_file;

	//Store the app structure
	App *app = user_data;

	//Reload the style file 
	if (event == G_FILE_MONITOR_EVENT_CHANGED || event == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT) {
	
		gtk_css_provider_load_from_path(app->css, g_file_peek_path(file));
	
	}

}

//Creates the window
static void create_window(App *app) {

	//Local variables
	char *config;		//Config file path
	char *style;		//Style file path
	
	//Check if a config file path was set by the user
	if (app->config_file) {
	
		//if so, pass the ownership of the path
		//to config and destroy the old pointer
		config = app->config_file;
		app->config_file = NULL;
	
	}
	else {

		//If the user didn't set one, use the default path
		config = g_build_filename(g_get_user_config_dir(), "optionizer-3000", "config", NULL);

	}	

	//Check if the config file exists
	if (g_file_test(config, G_FILE_TEST_EXISTS)) {	

		//If the file exists, check if get_label_count
		//can get the number of labels from it
		if (get_label_count(config, app) != 0) {
		
			//If there are no labels, print
			//the error message and exit
			printf("No label settings found in %s\n", config);
			exit(1);
		
		}
	
		//Allocate space for the options
		app->options = calloc(app->label_count, sizeof(Option));

		//Load the config file and check the return
		//status in case something goes wrong
		if (load_config(config, app) != 0) {
			exit(1);
		}

	}	
	else {

		//If no config file is found, print an error
		//message
		printf("config file not found at %s\n", config);
		
		//Create a dummy label_count and options
		app->label_count = 4;
		app->options = calloc(app->label_count, sizeof(Option));
		app->options[0].icon = strdup("");
		app->options[0].label = strdup("Reboot");
		app->options[0].command = strdup("systemctl reboot");
		app->options[1].icon = strdup("󰐥");
		app->options[1].label = strdup("Power off");
		app->options[1].command = strdup("systemctl poweroff");
		app->options[2].icon = strdup("󰌾");
		app->options[2].label = strdup("Lock");
		app->options[2].command = strdup("hyprlock");
		app->options[3].icon = strdup("󰤄");
		app->options[3].label = strdup("Suspend");
		app->options[3].command = strdup("systemctl suspend");

	}

	//check if a style file was set by the user
	if (app->style_file) {
	
		//if so, pass the ownership to style
		//and destroy the pointer variable
		style = app->style_file;
		app->style_file = NULL;
	
	}
	else {
		style = g_build_filename(g_get_user_config_dir(), "optionizer-3000", "style.css", NULL);
	}
	
	//Create window
	app->window = GTK_WINDOW(gtk_application_window_new(app->application));
	
	//Check if the style file is there
	if(g_file_test(style, G_FILE_TEST_EXISTS)) {

		//Default CSS style provider
		app->css = gtk_css_provider_new();
	
		//Load the style.css for the provider
		gtk_css_provider_load_from_path(app->css, style);

		//Add style provider for display (whatever that means)
		gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(app->window)), GTK_STYLE_PROVIDER(app->css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
		
		//CSS file
		app->file = g_file_new_for_path(style);

		//Create monitor for CSS file
		app->monitor = g_file_monitor_file(app->file, G_FILE_MONITOR_NONE, NULL, NULL);

		//Connect the monitor to callback
		g_signal_connect(app->monitor, "changed", G_CALLBACK(css_changed), app);

	}
	else {
		printf("style file not found at %s\n", style);
	}

	//Window title
	gtk_window_set_title(app->window, app->config.title);
	gtk_widget_add_css_class(GTK_WIDGET(app->window), "window");

	//Window size
	gtk_window_set_default_size(app->window, app->config.width, app->config.height);

	//Check if the window should follow the layer shell protocol
	if (app->config.set_as_layer) {

		//Set window as layer shell
		gtk_layer_init_for_window(app->window);

		//Set the window's namespace
		gtk_layer_set_namespace(app->window, app->config.namespace);

		//Focus the keyboard on the window
		gtk_layer_set_keyboard_mode(app->window, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

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
	//Horizontal box for listboxes
	app->box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, app->config.listbox_spacing));
	gtk_window_set_child(app->window,GTK_WIDGET(app->box));

	//Amount of listboxes to allocate
	app->listbox_count = (app->label_count + app->config.max_rows - 1) / app->config.max_rows;

	//Allocate space for listboxes
	app->listboxes = calloc(app->listbox_count, sizeof(GtkListBox *));
	
	//Create the key controller
	GtkEventController *controller = gtk_event_controller_key_new();
	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(controller), GTK_PHASE_CAPTURE);

	//Connect the controller to the key handler
	g_signal_connect(controller,"key-pressed", G_CALLBACK(on_key_pressed), app);

	//Add the controller to the window
	gtk_widget_add_controller(GTK_WIDGET(app->window), controller);

	//Cleanup
	if (config) {
		g_free(config);
	}

	if (style) {
		g_free(style);
	}

}

//creates a row to put inside the window
static void create_row(App *app, Option *option, GtkListBox *listbox) {
	
	//Box to store the labels
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, app->config.label_spacing);

	//icon label
	if (option->icon != NULL && strcmp(option->icon, " ") != 0) {

		//Create the icon widget
		GtkWidget *icon = gtk_label_new(option->icon);
		gtk_widget_set_hexpand(icon, FALSE);
		gtk_widget_set_halign(icon, GTK_ALIGN_CENTER);
		gtk_widget_add_css_class(icon, "icon");

		//Append the icon to box
		gtk_box_append(GTK_BOX(box), icon);

	}

	//option label
	if (option->label != NULL && strcmp(option->label, " ") != 0) {
	
		//Create the label widget
		GtkWidget *label = gtk_label_new(option->label);
		gtk_widget_set_hexpand(label, TRUE);
		gtk_widget_add_css_class(label, "label");

		//Check what label alignment to use
		if (strcmp(app->config.label_align, "left") == 0) {
			gtk_widget_set_halign(label, GTK_ALIGN_START);
		}
		else if (strcmp(app->config.label_align, "center") == 0) {
			gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
		}
		else if (strcmp(app->config.label_align, "right") == 0) {
			gtk_widget_set_halign(label, GTK_ALIGN_END);
		}

		//Append the label to box
		gtk_box_append(GTK_BOX(box), label);

	}

	//Create a row
	GtkWidget *row = gtk_list_box_row_new();
	gtk_widget_add_css_class(row, "row");

	//Set the box as the row's child
	gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);

	//Store the option in the row
	g_object_set_data(G_OBJECT(row), "option", (gpointer)option);

	//Append the row to the listbox
	gtk_list_box_append(listbox, row);
	
}

//Commandline callback
static void on_command_line(GApplication *application, GApplicationCommandLine *cmdline, gpointer user_data) {
	
	//App
	App *app = user_data;

	//Executable options
	GVariantDict *options = g_application_command_line_get_options_dict(cmdline);
	
	//Config file
	char *config_file;
	
	//Style file
	char *style_file;

	//Check if there's a config file
	//in the options and store it
    	if (g_variant_dict_lookup(options, "config", "&s", &config_file)) {
		app->config_file = g_strdup(config_file);
	}
	
	//Check if there's a style file
	//in the options and store it
    	if (g_variant_dict_lookup(options, "style", "&s", &style_file)) {
		app->style_file = g_strdup(style_file);
	}

	//Activate the application
	g_application_activate(application);

}

//Activation callback
static void activate(GtkApplication *application, gpointer user_data) {

	//Unused arguments
	(void)application;

	//App
	App *app = user_data;

	//Create window
	create_window(app);

	//Option counter
	size_t n = 1;

	//Listbox index
	size_t i = 0;

	//Initial listbox
	app->listboxes[i] = GTK_LIST_BOX(gtk_list_box_new());
	gtk_widget_add_css_class(GTK_WIDGET(app->listboxes[i]), "listbox");
	
	//Append the listbox to window's box
	gtk_box_append(app->box, GTK_WIDGET(app->listboxes[i]));

	//Iterate over the options
	for (size_t j = 0; j < app->label_count; j++) {
		
		//Create a row for an option
		create_row(app, &app->options[j], app->listboxes[i]);
		
		//Increase the counter
		n++;

		//Check if the counter has passed
		//the maximum number of rows
		if (n > app->config.max_rows) {
			
			//Increase the listbox index
			i++;

			//Create a new listbox
			app->listboxes[i] = GTK_LIST_BOX(gtk_list_box_new());
			gtk_widget_add_css_class(GTK_WIDGET(app->listboxes[i]), "listbox");

			//Append the new listbox to the window's box
			gtk_box_append(app->box, GTK_WIDGET(app->listboxes[i]));

			//Reset the counter
			n = 1;
		
		}

	}

	//Present window
	gtk_window_present(app->window);

}

//Main
int main(int argc, char *argv[]) {

	//Initial app structure
	App app = {0};

	//Default app configuration
	app.config.width = 100;
	app.config.height = 1;
	app.config.has_position = false;
	app.config.label_align = strdup("left");
	app.config.label_spacing = 10;
	app.config.listbox_spacing = 0;
	app.config.max_rows=4;
	app.config.title = strdup("optionizer-3000");	
	app.config.set_as_layer = true;
	app.config.namespace = strdup("optionizer-3000");

	//Create application
	app.application = gtk_application_new("com.torisaurus.optionizer-3000", G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_NON_UNIQUE);
	
	//Executable options
	g_application_add_main_option(G_APPLICATION(app.application), "config", 'c', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, "Path to configuration file", "FILE");
	g_application_add_main_option(G_APPLICATION(app.application), "style", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, "Path to style file", "FILE");

	//Connect application to commandline callback
	g_signal_connect(app.application, "command-line", G_CALLBACK(on_command_line), &app);

	//Connect application to activation callback
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
	free(app.listboxes);
	free(app.config.label_align);
	free(app.config.title);
	free(app.config.namespace);

	if (app.css) {
		g_object_unref(app.css);
	}

	if (app.file) {
		g_object_unref(app.file);
	}

	if (app.monitor) {
		g_object_unref(app.monitor);
	}
	
	//Return exit code from status
	return status;

}
