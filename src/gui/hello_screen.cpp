#include "hello_screen.h"
#include <spdlog/spdlog.h>

#include "../constants.h"

// Destructor to clean up objects
hello_screen::~hello_screen()
{
    for (auto obj : deletion_queue) {
        lv_obj_delete(obj);
    }
}

void hello_screen::start()
{
    spdlog::info("Displaying Hello Screen");

    setup_screen_content();

    // Refresh the entire screen
    lvgl_renderer_inst->refresh({ 0, 0 }, { SCREEN_WIDTH, SCREEN_HEIGHT }, FULL);
}

void hello_screen::setup_screen_content()
{
    // Create "Hello World" label and center it
    auto hello_label = lv_label_create(lv_screen_active());
    lv_label_set_text(hello_label, "Hello World");
    lv_obj_align(hello_label, LV_ALIGN_CENTER, 0, 0);

    // Add the label to the deletion queue
    deletion_queue.push_back(hello_label);

    // Create the back button
    auto back_btn = create_back_button();
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    deletion_queue.push_back(back_btn);
}

lv_obj_t* hello_screen::create_back_button()
{
    auto btn = lv_btn_create(lv_screen_active());
    auto label = lv_label_create(btn);
    lv_label_set_text(label, "Back");

    lv_obj_center(label);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
        spdlog::info("Returning to Boot Screen");
        // Assuming there is a function to transition back to the boot screen
        // e.g., boot_screen_instance->start();
    },
        LV_EVENT_CLICKED, nullptr);

    return btn;
}
