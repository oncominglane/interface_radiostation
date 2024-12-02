#include <X11/Xlib.h>
#undef None

#include "buttons.h"
#include "screen.h"
#include "message.h"
#include "TxRxEth.h"

#include "config.h"

#include <thread>
#include <atomic>

bool __listener_thread_running = true;

std::thread ethernetThread;
// Функция прослушивания Ethernet соединения
void ethernetListener(std::vector<std::string> *texts) {
    while (__listener_thread_running) {
        std::string data = receive_eth();

        std::cout << "[RECEIVE]: {" << data << std::endl;
        message(data, texts);

        std::cout << "}\n\n[DATA]: `" << data << "`\n[TEXTS]: `" << texts << "`\n";
    }
}

unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE * sizeof(*buffer));  // Выделение памяти для буфера

// Флаг для управления завершением аудиопередачи
std::atomic<bool> audio_transmit(false);
std::thread audioThread;

// Функция передачи АУДИО
void audio(unsigned char *buffer) {
    while (audio_transmit) {
        audioTxEth(buffer, audio_transmit);
    }
}

int main() {
    XInitThreads();

    sf::RenderWindow window(sf::VideoMode(resolution_x, resolution_y), "Interface Radiostation Project");
    window.setActive(false);

    Screen_main main_screen(
        sf::Vector2f(left_border + button_offset, bottom_border),  // TODO Make vector and use it in texts positioning
        sf::Vector2f(main_screen_width, main_screen_height), "assets/white.png", "Main Screen");

    std::vector<Button *> buttons;
    buttons_create(buttons);

    std::vector<std::string> texts;
    ethernetThread = std::thread(ethernetListener, &texts);


    sf::Font font;
    font.loadFromFile("assets/troika.otf");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {  // Button is pressed -> Sending command
                for (const auto &button : buttons) {
                    button->change_color(sf::Color::White);

                    if (button->isMouseOver(window)) {  
                        if (button->m_command == "ptt") {
                            if (!audio_transmit) { // Проверяем, не идет ли передача
                                audio_transmit = true;
                                audioThread = std::thread(audioTxEth, buffer, std::ref(audio_transmit));  // Передаем флаг по ссылке
                            }
                        }
                        // FIXME Get coordinartes from event not from window directly
                        // const std::string command = button->m_command;
                        else
                           transmit_eth(button->m_command);
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (audio_transmit) {
                    audio_transmit = false; // Устанавливаем флаг завершения
                    if (audioThread.joinable()) {
                        audioThread.join(); // Дожидаемся завершения потока
                    }
                }
            }


            window.clear(sf::Color::Black);

            if (texts.size() > 0)
                main_screen.change_text("Message: " + texts[0]);

            main_screen.draw(window);

            for (const auto &button : buttons)
                button->draw(window);

            int x_offset = left_border + button_offset;
            int y_offset = bottom_border + text_offset;

            for (size_t text_number = 1; text_number < texts.size(); text_number++) {
                std::string screen_text_string = "Button " + std::to_string(text_number) + ": " + texts[text_number];

                sf::Text screen_text(screen_text_string, font);
                screen_text.setPosition(x_offset, y_offset);
                screen_text.setFillColor(sf::Color::Black);

                window.draw(screen_text);

                y_offset += text_offset;
            }

            window.display();
        }
    }

    if (audio_transmit) {
        audio_transmit = false;
        if (audioThread.joinable()) {
            audioThread.join();
        }
    }

    if (ethernetThread.joinable()) 
        ethernetThread.join();

    for (auto &button : buttons)
        delete button;
    free(buffer);
    return 0;
}