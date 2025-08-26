#include <stdlib.h>

#include "app.h"

int main(int argc, char *argv[]) {
    app_t app;
    if (!app_init(&app)) {
        return EXIT_FAILURE;
    }
    app_main_loop(&app);
    app_deinit(&app);
    return EXIT_SUCCESS;
}
