
/*
    This is a TEMPLATE .c file for apps. This file is never compiled. If you want
    to make a new app, copy this file and replace everything with your desired 
    functionality. 
    
    See color_screen for an example of a (basic) implemented app.

    EACH FILE NEEDS THE FOLLOWING:
     - an init function to initialize the wiimote_objs
     - a draw function that draws NON-WIIMOTE objs (there is another function that actually
       draws wiimote objects)

*/

// Create int variables for any Wiimote objs at the top of the file

// Change [CHANGE_THIS] to the name of the file
bool init_check_CHANGE_THIS = false;

// Change [CHANGE_THIS] to the name of the file, and don't add any parameters
void init_CHANGE_THIS () {

    // Assign the variables you initialized above using calls to create_*

}

// Change [CHANGE_THIS] to the name of the file, and don't add any parameters. This function will
// be run on loop! It is necessary for EVERYTHING to be redrawn on every frame.
void draw_CHANGE_THIS () {

    // Init objs only on first run. Change variable to what you defined above
    if (!init_check_CHANGE_THIS) {
        init_CHANGE_THIS();
        init_check_CHANGE_THIS = true;
    }

    // Other game logic can go here

}

