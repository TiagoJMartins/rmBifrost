#include "compositor.h"

#include <spdlog/spdlog.h>
#include "../gui/boot_screen.h"

#include <utility>
compositor::compositor(display_config cfg)
    : cfg(cfg)
    , socket(std::make_unique<unix_socket>("/run/bifrost_comp_ctl.sock", true))
    , renderer(std::make_shared<lvgl_renderer>(
          cfg.fb,
          fb_mutex,
          std::bind(&compositor::request_refresh, this, std::placeholders::_1, std::placeholders::_2)))
{
    renderer->initialize();
}

void compositor::start()
{
    spdlog::info("Starting bifrost compositor");
    running = true;

    render_thread = std::thread([this]() {
        int freq = 1000000 / 85;
        long last_tick = 0;
        while (running) {
            render_clients();
            renderer->tick();
            for (const auto& [req_region, req_type] : pending_refresh_requests) {
                refresh(req_region.p1, req_region.p2, req_type);
            }
            pending_refresh_requests.clear();
            long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            long sleep_time = freq - (now - last_tick);
            if (sleep_time > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
            }
            last_tick = now;
        }
    });

    auto boot_screen_inst = std::make_shared<boot_screen>(renderer);
    auto boot_screen_thread = std::thread([&] { boot_screen_inst->start(); });

    boot_screen_thread.join();

    if (boot_screen_inst->state == RM_STOCK_OS) {
        running = false;;
        render_thread.join();

        spdlog::debug("Relinquished control flow to the stock OS");
        return;
    }

    boot_screen_inst.reset();

    refresh({0, 0}, {SCREEN_WIDTH, SCREEN_HEIGHT}, FULL);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    listener_thread = std::thread(&compositor::listener, this);
    render_thread.join();
}

void compositor::request_refresh(rect update_region, refresh_type type)
{
    if (pending_refresh_requests.empty()) {
        pending_refresh_requests.push_back({ update_region, type });
    } else {
        for (auto& [req_region, req_type] : pending_refresh_requests) {
            if (req_region.intersects(update_region)) {
                req_region = req_region.union_(update_region);
                req_type = std::max(req_type, type);
                return;
            }
        }
        pending_refresh_requests.push_back({ update_region, type });
    }
}

void compositor::render_clients()
{
    // Remove disconnected clients
    clients.erase(std::remove_if(clients.begin(), clients.end(), [](const auto& client) {
        if (client->state == compositor_client::client_state::DISCONNECTED) {
            spdlog::info("Client {} has disconnected", client->application_name);
            return true;
        }
        return false;
    }), clients.end());

    for (const auto& client : clients) {
        auto swapchain_image = client->get_swapchain_image();
        if (!swapchain_image) {
            return;
        }
        auto [frame_id, submit_frame, composite_region, image_data] = *swapchain_image;

        void* image_base_ptr = reinterpret_cast<void*>(image_data);
        rect src_update_region = submit_frame.dirty_rect;
        rect dst_update_region = { composite_region.p1 + src_update_region.p1, composite_region.p1 + src_update_region.p2 };
        spdlog::debug("Rendering client {} with src_update_region [{},{},{},{}] and dst_update_region [{},{},{},{}]", client->application_name, src_update_region.p1.x, src_update_region.p1.y, src_update_region.p2.x, src_update_region.p2.y, dst_update_region.p1.x, dst_update_region.p1.y, dst_update_region.p2.x, dst_update_region.p2.y);
        for (uint32_t y = dst_update_region.p1.y; y <= dst_update_region.p2.y; y++) {
            QRgb* line = reinterpret_cast<QRgb*>(cfg.fb->scanLine(y));
            uint8_t* dst_line_ptr = reinterpret_cast<uint8_t*>(line);
            uint8_t* src_line_ptr = reinterpret_cast<uint8_t*>(image_base_ptr) + (y - composite_region.p1.y) * composite_region.width() * 4;
            spdlog::debug("Copying line {}; first pixel is {}", y, *reinterpret_cast<uint32_t*>(src_line_ptr));
            std::memcpy(dst_line_ptr + dst_update_region.p1.x * 4, src_line_ptr + src_update_region.p1.x * 4, (dst_update_region.p2.x - dst_update_region.p1.x + 1) * 4);
        }

        request_refresh(dst_update_region, submit_frame.preferred_refresh_type);

        client->release_swapchain_image(frame_id);
    }
}

void compositor::stop()
{
    running = false;
    for (const auto& client : clients) {
        client->stop();
    }
    listener_thread.join();
}

void compositor::listener()
{
    while (running) {
        spdlog::debug("Waiting for connection");
        auto connection = socket->accept_connection();
        spdlog::debug("Accepted connection");
        auto client = std::make_shared<compositor_client>(std::move(connection), compositor_client::compositor_client_config {
                                                                                     .title_bar_height = 0,
                                                                                 });
        clients.push_back(client);
        client->start();
    }
}

void compositor::refresh(point p1, point p2, refresh_type type)
{
    switch (type) {
    case MONOCHROME:
        cfg.screen_update_func(cfg.epfb_inst, p1, p2, 0, 0, 0);
        break;
    case COLOR_ANIMATION:
        cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 0, 0);
        break;
    case COLOR_CONTENT:
        cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 4, 0);
        break;
    case COLOR_FAST:
        cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 1, 0);
        break;
    case COLOR_1:
        cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 3, 0);
        break;
    case COLOR_2:
        cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 5, 0);
        break;
    case FULL:
        cfg.screen_update_func(cfg.epfb_inst, p1, p2, 1, 4, 1);
        break;
    }
}
