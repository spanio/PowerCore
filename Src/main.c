/*******************************************************************************
// main.c - C entry point
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
#include "App.h"
// Other Includes
#include "cpu.h" // ESTOP - TODO: Remove
#include <stdint.h> // Defines C99 integer types
#include <stdlib.h> // C exit codes
#include <limits.h>

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// C entry point
int main(void)
{
    // Call the application instance
    // This will initialize modules
    App_Init(0, &appConfig);

    //NVM_Test();

    // Run the main application function -- this just runs the scheduler
    App_Run();

   // Standard C exit code
   return(EXIT_SUCCESS);
}

// Error handling function to be called when an ASSERT is violated
// TODO: Move to error handling module once defined
void __error__(const char *filename, uint32_t line)
{
    // An ASSERT condition was evaluated as false. You can use the filename and
    // line parameters to determine what went wrong.
    ESTOP0;
}
