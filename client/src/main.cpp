#include <X11/Xlib.h>
#undef None

#include "config.h"
#include "buttons.h"
#include "lamps.h" 
#include "screen.h"
#include "message.h"

#include "TxRxEth.h"

#include <thread>
#include <atomic>
#include <mutex>

void customTerminate() {
    std::cerr << "Custom terminate handler called!" << std::endl;
    std::abort();
}

bool __listener_thread_running = true;
std::thread ethernetThread;

void ethernetListener(std::vector<std::string> *texts) {// Функция прослушивания Ethernet соединения
    try {
        while (__listener_thread_running) {
        std::string data = receive_eth();

        std::cout << "[RECEIVE]: {" << data << std::endl;
        message(data, texts);

        std::cout << "}\n\n[DATA]: `" << data << "`\n[TEXTS]: `" << texts << "`\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in ethernetListener: " << e.what() << std::endl;
        __listener_thread_running = false;
    } catch (...) {
        std::cerr << "Unknown exception in ethernetListener!" << std::endl;
        __listener_thread_running = false;
    }
}

std::vector<unsigned char> buffer(BUFFER_SIZE); 
std::mutex buffer_mutex;                // Мьютекс для защиты buffer

std::atomic<bool> audio_receive(true);  // Флаг для управления потоком приёма
std::atomic<bool> audio_transmit(false); // Флаг для передачи

std::thread audioRxThread;
std::thread audioTxThread;

void audioReceiver(std::vector<unsigned char>& buffer, std::atomic<bool>& running) {
    try { while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::vector<unsigned char> localBuffer;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            localBuffer = buffer; // Копируем данные
        }
        try {
            audioRxEth(localBuffer.data(), running);
        } catch (const std::exception& e) {
            std::cerr << "audioRxEth exception: " << e.what() << std::endl;
            throw; // Либо обработать, либо передать дальше
        }
    }
     } catch (const std::exception& e) {
        std::cerr << "Exception in audioReceiver: " << e.what() << std::endl;
        running = false;
    } catch (...) {
        std::cerr << "Unknown exception in audioReceiver!" << std::endl;
        running = false;
    }
}

void audioTransmitter(std::vector<unsigned char>& buffer, std::atomic<bool>& running) {
    try {while (running) {
        std::vector<unsigned char> localBuffer;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            localBuffer = buffer; // Копируем данные
        }
        audioTxEth(localBuffer.data(), running);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in audioTransmitter: " << e.what() << std::endl;
        running = false;
    } catch (...) {
        std::cerr << "Unknown exception in audioTransmitter!" << std::endl;
        running = false;
    }
}



  
int main() {
    std::set_terminate(customTerminate);
    try{
    XInitThreads();

    // Запускаем поток приёма звука
    audioRxThread = std::thread(audioReceiver, std::ref(buffer), std::ref(audio_receive));


    sf::RenderWindow window(sf::VideoMode(resolution_x, resolution_y), "Interface Radiostation Project");
    window.setActive(false);

    Screen_main main_screen(
        sf::Vector2f(left_border + button_offset, bottom_border),  // TODO Make vector and use it in texts positioning
        sf::Vector2f(main_screen_width, main_screen_height), "assets/white.png", "Main Screen");

    // Вектор лампочек
    std::vector<Lamp> lamps;


    std::vector<Button *> buttons;
    buttons_create(buttons);

    std::vector<std::string> texts;

    /*try {
        ethernetThread = std::thread(ethernetListener, &texts);
        pthread_setname_np(ethernetThread.native_handle(), "EthernetThread");
    } catch (const std::exception& e) {
        std::cerr << "Exception while starting ethernetThread: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception while starting ethernetThread!" << std::endl;
    }*/

    


    sf::Font font;
    try {
        if (!font.loadFromFile("assets/troika.otf")) {
            throw std::runtime_error("Failed to load font!");
        }
    } catch (const std::exception& e) {
        std::cerr << "Font loading error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Создаем лампочки
    lamp_create(lamps);  


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
                                lamps[0].changeColor(sf::Color::Red);
                                audio_receive = false; // Останавливаем приём звука
                                audio_transmit = true;
                                audioTxThread = std::thread(audioTransmitter, std::ref(buffer), std::ref(audio_transmit));
                            }
                        }
                        // FIXME Get coordinartes from event not from window directly
                        else
                           transmit_eth(button->m_command);
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (audio_transmit) {
                    audio_transmit = false; // Устанавливаем флаг завершения
                    try {
                        if (audioTxThread.joinable()) audioTxThread.join();
                    } catch (const std::exception& e) {
                        std::cerr << "Exception while joining audioRxThread: " << e.what() << std::endl;
                    }
                    
                    audio_receive = true; // Возобновляем приём звука
                    lamps[0].changeColor(sf::Color::Black);
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


            // Отрисовка лампочек
            for (auto &lamp : lamps) {
                lamp.draw(window);
            }

            window.display();
        }
       


   } 
    // Завершение потоков корректно
        audio_receive = false;
        try {
        if (!audio_receive) {
            if (audioRxThread.joinable()) audioRxThread.join();
        }
        }catch (const std::exception& e) {
                        std::cerr << "Exception while joining audioRxThread: " << e.what() << std::endl;
                    }
        
        try {
        if (!audio_transmit) {
            if (audioTxThread.joinable()) audioTxThread.join();
        }
        }catch (const std::exception& e) {
                        std::cerr << "Exception while joining audioTxThread: " << e.what() << std::endl;
                    }

        /*try {
        if (ethernetThread.joinable()) ethernetThread.join();
        }catch (const std::exception& e) {
                        std::cerr << "Exception while joining ethernetThread: " << e.what() << std::endl;
                    }
        */

    for (auto &button : buttons)
        delete button;

//FIXME удаление лампочек 

    } catch (const std::exception& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown exception caught in main!" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}