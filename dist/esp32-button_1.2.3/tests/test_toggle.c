#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "toggle.h"

static void test_callback(bool high, void *context) {
        (void) high;
        (void) context;
}

int main(void) {
        const gpio_num_t gpio = 4;

        int result = toggle_create(gpio, NULL, NULL);
        assert(result == -5);

        int second = toggle_create(gpio, NULL, NULL);
        assert(second == -5);

        int success = toggle_create(gpio, test_callback, NULL);
        assert(success == 0);

        toggle_delete(gpio);

        int recreate = toggle_create(gpio, test_callback, NULL);
        assert(recreate == 0);

        toggle_delete(gpio);

        puts("toggle_create NULL callback test passed");
        return 0;
}
