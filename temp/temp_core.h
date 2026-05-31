#ifndef CORE_H
#define CORE_H

 // Starts the main menu loop and keeps the application running.
 
void runMenuSystem();


 // Displays the main user interface menu options on the screen.
 
void displayMenu();

/**
 * Handles the navigation to different modules based on user input.
 * @param choice The integer option selected by the user.
 */
void handleMenuChoice(int choice);

#endif // CORE_H