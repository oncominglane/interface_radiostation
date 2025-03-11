#include "buttons.h"
#include "config.h"

void buttons_create(std::vector<ButtonCircle *> &buttons) {
    sf::Vector2f offset(__left_ui_border, __bottom_ui_border);

    const std::string signal_button_asset = "assets/point.png";

    const std::tuple<const std::string, const std::string, std::string> buttons_map[] = {
         {"ptt",                  "assets/PTT.png",       ""},  {"\x4B\x4E\x05\x54\x01\x57\xA4", "assets/light.png", ""},
         {"\x4B\x4E\x05\x94\x01\x57\xED", "assets/emergency.png", ""},  {"\x4B\x4E\x05\x81\x01\x57\x21", "assets/home.png", ""},

        // {"\x05\x54\x01\x57\xA4", "assets/light.png", ""},
        // для добавления кнопок меняем параметр __max_buttons_in_border   

        {"\x4B\x4E\x05\x83\x02\x57\x7A", signal_button_asset, "1"},    {"\x4B\x4E\x05\x89\x02\x57\x24", signal_button_asset, "2"},
        {"\x4B\x4E\x05\x84\x02\x57\xB5", signal_button_asset, "3"},    {"\x4B\x4E\x05\x8A\x02\x57\xB4", signal_button_asset, "4"},
        {"\x4B\x4E\x05\x85\x02\x57\xC5", signal_button_asset, "5"},
        {"\x4B\x4E\x05\x04\x01\x57\xF5", "assets/left_arrow.png", ""}, {"\x4B\x4E\x05\x04\xFF\x57\xCC", "assets/right_arrow.png", ""}};

    // Fill order:
    // |
    // |
    // o---->

    int button_number = 1;

    for (const auto &[button_command, button_asset, button_label] : buttons_map) {
        buttons.push_back(new ButtonCircle(offset, __button_radius, button_asset, button_command, button_label));

        if (button_number++ < __max_buttons_in_border)
            offset.y += __graphic_object_offset;
        else
            offset.x += __graphic_object_offset;
    }
}