// Header setup whereby we can access and manipulate variables from multiple classes.

// These vars are instantiated in the Main class but declaring them as extern beforehand allows us to access them in classes outside of Main
extern const bool showDemoScenes;   // Should we display the demo scenes - flip to to hide the demo scenes when working on your own stuff
extern const int  demoSceneCount;   // How many demo scenes are there in total for us to navigate through?
extern       int  currentDemoScene; // If we are displaying the demo scenes, which one?

// Static methods to move between demos - the actual methods are implemented in the Window class to respond to keypress handlers
static void nextDemo();
static void previousDemo();
