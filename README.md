# power-menu

I was using wofi as my power menu, but the window size
kept bugging out, so yeah.

## Features

The classic power menu options:
- Reboot: if you want your computer to turn off and back on real quick
- Power off: if you just want your computer to turn off forever
- Lock: if you want your computer to lock you out (remember your password)
- Suspend: if you want your computer to quiet down for a bit

## Requirements

Build dependencies:
- GCC
- GNU Make
- pkg-config

Runtime dependencies:
- A Wayland compositor with layer shell support like Sway or Hyprland
- GTK 4
- GTK 4 Layer Shell
- systemd (optional)
- hyprlock (optional)
- RobotoMono Nerd Font (optional)
- Symbols Nerd Font (optional)

The reboot, power off, lock and suspend commands can be changed in the [configuration file](config/power-menu.conf#L3-6) and the font configuration can be changed in [the style file](config/style.css#L11), therefore, systemd, hyprlock, RobotoMono Nerd Font and Symbols Nerd Font aren't strict dependencies.

## Installation

```bash
# Clone the repository and compile
git clone https://github.com/diego-zarate-2003/power-menu.git
cd power-menu
make

# Create the user configuration folder
mkdir -p ~/.config/power-menu/

# Copy the configuration files to the user directory
cp ./config/power-menu.conf ~/.config/power-menu/
cp ./config/style.css ~/.config/power-menu/

# Run the executable
./power-menu
```
power-menu loads its default configuration from ```./config/``` and then applies any user configuration found in ```~/.config/power-menu```, overriding conflicting default values. power-menu won't work without its default "power-menu.conf" file.

## Configuration Options

The following options can be set inside the "power-menu.conf" file:

- width: window width in pixels
- height: window height in pixels
- reboot: reboot command
- poweroff: power off command
- lock: lock command
- suspend: suspend command
- x_pos: window horizontal position in pixels
- y_pos: window vertical position in pixels
- label_align: horizontal alignment for the text label (left, center or right)
- reboot_icon: icon for the reboot option
- poweroff_icon: icon for the power off option
- lock_icon: icon for the lock option
- suspend_icon: icon for the suspend option
- spacing: spacing between the icon and the text label in pixels

If there are no lines for x_pos or y_pos, the window position defaults to the center of the screen. The position of the window is measured increasing from left to right and top to bottom, with the origin being the top left of the screen.

If a menu option doesn't have an icon line, then the row for that option will only contain the label, even if other options have an icon.

## Styling configuration

The following classes can be configured inside "style.css":

- .power-menu: the window itself
- .power-listbox: the listbox containing the rows
- .power-icon: the text icon
- .power-label: the text label
- .power-row: the row containing the icon and label

If using nerd fonts, it's recommended to configure the icons like this:

```css
.power-menu {
	font-family: Symbols Nerd Font Mono, RobotoMono Nerd Font Mono;
	font-size: 13px;
}
```
which makes it so the Symbols font overrides the icons in the main font, preventing possible size mismatches.

## Want it to do something else?

Open "power-menu.c" and add new variables inside the [Config structure](power-menu.c#L13-L30) for the new option and its icon:

```c
//Configuration
typedef struct {
	int width;
	int height;
	char *reboot;
	char *poweroff;
	char *lock;
	char *suspend;
    char *new_option;          //THIS IS NEW!!!
	int x_pos;
	int y_pos;
	bool has_position;
	char *label_align;
	char *reboot_icon;
	char *poweroff_icon;
    char *new_option_icon;     //THIS IS NEW!!
	char *lock_icon;
	char *suspend_icon;
	int spacing;
} Config;
```
Create a new [action function prototype](power-menu.c#L46-49) and add it with the rest:

```c
void reboot(App *app);
void poweroff(App *app);
void lock(App *app);
void suspend(App *app);
void new_option_function(App *app);    //THIS IS NEW!!!
```
Make sure that [OPTION_COUNT](power-menu.c#L61) is also equal to the amount of options you now have:

```c
//NOW IT'S 5!!!
#define OPTION_COUNT 5
```

Add the new option label and function to the [options array](power-menu.c#L63-69):

```c
//Options
MenuOption options[] = {
    {"Reboot", reboot},
    {"Power off", poweroff},
    {"Lock", lock},
    {"Suspend", suspend},
    {"New option label", new_option_function}    //THIS IS NEW!!!
};
```

Now, bear with me. Since the program needs to find the actual command to execute from [power-menu.conf](config/power-menu.conf), you need to add it there along with the icon (if needed):

```
width=100
height=1
reboot=systemctl reboot
poweroff=systemctl poweroff
lock=hyprlock
suspend=systemctl suspend
new_option=your command here
new_option_icon=
label_align=center
spacing=0

```
then, inside the [load_config function](power-menu.c#L94-L178) in power-menu.c, add these statements so the program can read the new lines from power-menu.conf:

```c
//11 because "new_option=" has 11 characters
if (strncmp(line, "new_option=", 11) == 0) {
    char *new_option = line + 11;
    new_option[strcspn(new_option, "\n")] = '\0';
    config->new_option = strdup(new_option);
}

//16 because "new_option_icon=" has 16 characters and
//icons[4] because new_option is the 5th option	
if (strncmp(line, "new_option_icon=", 16) == 0) {
    char *new_option_icon = line + 16;
	new_option_icon[strcspn(new_option_icon, "\n")] = '\0';
	icons[4] = strdup(new_option_icon);
}
```
Finally, define the new function and add it to the rest of [action functions](power-menu.c#L425-L437):

```c
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
void new_option_function(App *app) {    //THIS IS NEW!!!
    system(app->config.new_option);
}
```

Or just gut it, remove everything and add whatever you want. This just executes a command by selecting its respective label inside the window.

## Examples

Default configuration:

<p align="center">
    <img width="800" src="screenshots/default.png">
</p>

With icons:

<p align="center">
    <img width="800" src="screenshots/icons.png">
</p>
