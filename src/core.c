#include "config.h" // Includes standard libraries like stdio.h via config.h
#include "core.h"

// TODO: Include other members' header files when ready
// #include "user.h"
// #include "plate.h"
// #include "request.h"
// #include "chat.h"

/**
 * Prints the formatted main menu screen.
 */
void displayMenu() {
    printf("\n--- PlateShare Pro ---\n");
    printf("1. User Management\n");
    printf("2. Plate Management\n");
    printf("3. Request Management\n");
    printf("4. Chat Room\n");
    printf("5. Settings\n");
    printf("6. Exit\n");
    printf("Choose an option: ");
}

/**
 * Routes the user to the correct module based on their menu selection.
 */
void handleMenuChoice(int choice) {
    switch(choice) {
        case 1:
            printf("\n[Navigating to User Management...]\n");
            // Call Sahithiyan's function here: userMenu();
            break;
        case 2:
            printf("\n[Navigating to Plate Management...]\n");
            // Call Senuri's function here: plateMenu();
            break;
        case 3:
            printf("\n[Navigating to Request Management...]\n");
            // Call Shaliya's function here: requestMenu();
            break;
        case 4:
            printf("\n[Navigating to Chat Room...]\n");
            // Call Salaah's function here: chatMenu();
            break;
        case 5:
            printf("\n[Navigating to Settings...]\n");
            break;
        case 6:
            printf("\nExiting PlateShare Pro. Thank you!\n");
            break;
        default:
            printf("\nInvalid option! Please choose between 1 and 6.\n");
    }
}

/**
 * Runs the infinite menu loop until the user chooses to exit (Option 6).
 */
void runMenuSystem() {
    int choice = 0;
    while(choice != 6) {
        displayMenu();
        
        // Validate user input to ensure it is an integer
        if (scanf("%d", &choice) != 1) {
            printf("Please enter a valid number!\n");
            while(getchar() != '\n'); // Clear the input buffer to prevent infinite loop
            continue;
        }
        
        handleMenuChoice(choice);
    }
}