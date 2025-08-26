#include <stdlib.h>

#include "app.h"

int main(int argc, char *argv[]) {
    app_t app;
    if (!app_create(&app)) {
        return EXIT_FAILURE;
    }
    app_run(&app);
    app_destroy(&app);
    return EXIT_SUCCESS;
}
