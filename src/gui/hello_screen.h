#ifndef HELLO_SCREEN_H
#define HELLO_SCREEN_H

#include <memory>
#include <vector>
#include "lvgl_renderer.h"
#include <lvgl.h>

class hello_screen : public std::enable_shared_from_this<hello_screen> {
public:
    explicit hello_screen(const std::shared_ptr<lvgl_renderer>& renderer)
        : lvgl_renderer_inst(renderer) { }

    ~hello_screen();

    void start();

private:
    std::shared_ptr<lvgl_renderer> lvgl_renderer_inst;
    std::vector<lv_obj_t*> deletion_queue;

    void setup_screen_content();
    lv_obj_t* create_back_button();
};

#endif // HELLO_SCREEN_H
