#include "buttons.h"
#include "config.h"

void buttons_create(std::vector<Button *> &buttons) {
    sf::Vector2f offset(left_border, bottom_border);

    const std::map<const std::string, const std::string> functional_buttons = {{"assets/light.png", "\x05\x54\x01\x57\xA4"},
                                                                               {"assets/home.png", "\x05\x81\x01\x57\x21"},
                                                                               {"assets/emergency.png", "\x05\x94\x01\x57\xED"},
                                                                               {"assets/PTT.png", "ptt"}};

    for (const auto &[button_asset, button_command] : functional_buttons)
    {  // Creates functional buttons coloumn at the left border
        buttons.push_back(new ButtonCircle(offset, button_radius, button_asset, button_command, ""));

        offset.y += button_offset;
    }

    int               signal_button_number = 0;
    const std::string signal_button_asset  = "assets/point.png";

    const std::string signal_button_commands[] = {"\x05\x83\x02\x57\x7A", "\x05\x89\x02\x57\x24",  //"\x05\x84\x02\x57\xB5",
                                                  "\x05\x8A\x02\x57\xB4", "\x05\x85\x02\x57\xC5"};

    offset.y -= button_offset;
    for (const auto &signal_button_command : signal_button_commands)    { 
         // Creates signal buttons row at the bottom of functional buttons column
        offset.x += button_offset;
        buttons.push_back(new ButtonCircle(offset, button_radius, signal_button_asset, signal_button_command,
                                           std::to_string(++signal_button_number)));
    }

    const std::map<const std::string, std::string> arrow_buttons = {
        {"assets/arrow.png", "\x05\x04\xFF\x57\xCC"},  // Левая стрелка
        {"assets/arrow_.png", "\x05\x04\x01\x57\xF5"}   // Правая стрелка
    };
    
    
    std::cout << "Creating arrow buttons...\n";
    for (const auto &[button_asset, button_data] : arrow_buttons) {
        const auto button_command = button_data;
        offset.x += button_offset;

        //std::cout << "Button at: " << offset.x << ", " << offset.y 
        //        << " | Flip: " << std::endl;

        ButtonCircle *button = new ButtonCircle(offset, button_radius, button_asset, button_command, "");
        //std::cout << "Creating button with texture: " << button_asset
        //  << " | Command: " << button_command
        //  << " | Flip: " <<  std::endl;
        //std::cout << "Total buttons in map: " << arrow_buttons.size() << std::endl;

        buttons.push_back(button);
    }

    std::cout << "Total buttons: " << buttons.size() << std::endl;

    
    
}