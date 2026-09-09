# power-menu
I was using wofi as my power menu, but the window size
kept bugging out, so yeah.

## Features
- Reboot: if you want your computer to turn off and back on real quick
- Power off: if you just want your computer to turn off forever
- Lock: if you want your computer to lock (remember your password)
- Suspend: if you want your computer to quiet down for a bit

Want it to do something else? add a new option to the [options array](https://github.com/diego-zarate-2003/power-menu/blob/main/power-menu.c#L38-L43) like this:

```c
//Options
MenuOption options[] = {
    {"Reboot", reboot},
    {"Power off", poweroff},
    {"Lock", lock},
    {"Suspend", suspend},

    //THIS IS NEW!!!
    {"New option label", new_option_function}
};
```

Then add its function to the rest of the [action functions](https://github.com/diego-zarate-2003/power-menu/blob/main/power-menu.c#L181-L196):

```c
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

//THIS IS NEW!!!
void new_option_function(void) {
    system("new_option_command");
}
```

Or just gut it, remove everything and add whatever you want. This just executes a
command by selecting its respective label inside the window.

## Requirements

Application dependencies:

- GTK 4
- GTK 4 Layer Shell

Build dependencies:

- GCC
- GNU Make
- pkg-config

Runtime dependencies:

- A Wayland compositor with layer shell support like Sway or Hyprland
- systemd for the reboot, power off and suspend commands
- hyprlock for the lock command

The reboot, power off, lock and suspend commands can be changed in the [action functions](https://github.com/diego-zarate-2003/power-menu/blob/main/power-menu.c#L181-L196),
so systemd and hyprlock aren't strict dependencies.

## Installation

If you also use Arch, you can install the dependencies with:

```bash
sudo pacman -S gcc make pkgconf gtk4 gtk4-layer-shell
```

Compile the source code:

```bash
# Clone the repository and compile
git clone https://github.com/diego-zarate-2003/power-menu.git
cd power-menu
make

# Run the executable
./power-menu
```
You can also compile with just gcc:

```bash
gcc -O2 -march=native -flto power-menu.c -o power-menu $(pkg-config --cflags --libs gtk4 gtk4-layer-shell-0)
```

## Styling

From the repository folder:

```bash
#Create the configuration folder
mkdir -p ~/.config/power-menu

#Copy the example style.css
cp style.css ~/.config/power-menu/

#Edit the style configuration
cd ~/.config/power-menu
vim style.css
```

The example file is fully commented and contains everything needed for a basic configuration.
