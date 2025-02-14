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

//FIXME убрать все отловы ошибок try-catch, если все отлично работает

void customTerminate() {
    std::cerr << "Custom terminate handler called!" << std::endl;
    std::abort();
}

bool        __listener_thread_running = true;
std::thread ethernetThread;

void ethernetListener(std::vector<std::string> *texts) {  // Функция прослушивания Ethernet соединения
    try {
        while (__listener_thread_running) {
            std::string data = receive_eth();

            std::cout << "[RECEIVE]: {" << data << std::endl;
            message(data, texts);

            std::cout << "}\n\n[DATA]: `" << data << "`\n[TEXTS]: `" << texts << "`\n";
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Exception caught in ethernetListener: " << e.what() << std::endl;
        __listener_thread_running = false;
    }
    catch (...) {
        std::cerr << "Unknown exception in ethernetListener!" << std::endl;
        __listener_thread_running = false;
    }
}

std::vector<unsigned char> buffer(BUFFER_SIZE);
std::mutex                 buffer_mutex;  // Мьютекс для защиты buffer

std::atomic<bool> audio_receive(true);    // Флаг для управления потоком приёма
std::atomic<bool> audio_transmit(false);  // Флаг для передачи

std::atomic<bool> signal_received(false);  // Флаг для индикации приема сигнала

std::thread audioRxThread;
std::thread audioTxThread;

void audioReceiver(std::vector<unsigned char> &buffer, std::atomic<bool> &running, std::atomic<bool> &signal_received) {
    try {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::vector<unsigned char> localBuffer;
            {
                std::lock_guard<std::mutex> lock(buffer_mutex);
                localBuffer = buffer;  // Копируем данные
            }
            try {
                audioRxEth(localBuffer.data(), running, signal_received);
            }
            catch (const std::exception &e) {
                std::cerr << "audioRxEth exception: " << e.what() << std::endl;
                throw;  // Либо обработать, либо передать дальше
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Exception in audioReceiver: " << e.what() << std::endl;
        running = false;
    }
    catch (...) {
        std::cerr << "Unknown exception in audioReceiver!" << std::endl;
        running = false;
    }
}

void audioTransmitter(std::vector<unsigned char> &buffer, std::atomic<bool> &running) {
    try {
        while (running) {
            std::vector<unsigned char> localBuffer;
            {
                std::lock_guard<std::mutex> lock(buffer_mutex);
                localBuffer = buffer;  // Копируем данные
            }
            audioTxEth(localBuffer.data(), running);
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Exception in audioTransmitter: " << e.what() << std::endl;
        running = false;
    }
    catch (...) {
        std::cerr << "Unknown exception in audioTransmitter!" << std::endl;
        running = false;
    }
}

int main() {
    std::set_terminate(customTerminate);
    try {
        XInitThreads();

        // Запускаем поток приёма звука
        audioRxThread = std::thread(audioReceiver, std::ref(buffer), std::ref(audio_receive), std::ref(signal_received));
        sf::RenderWindow window(sf::VideoMode(resolution_x, resolution_y), "Interface Radiostation Project");
        window.setActive(false);

        Screen_main main_screen(
            sf::Vector2f(left_border + button_offset, bottom_border),  // TODO Make vector and use it in texts positioning
            sf::Vector2f(main_screen_width, main_screen_height), "assets/white.png", "Main Screen");

        // Создаем лампочки
        std::vector<Lamp> lamps;
        lamp_create(lamps);

        // Создаем кнопки
        std::vector<Button *> buttons;
        buttons_create(buttons);

        std::vector<std::string> texts;

        sf::Font font;
        try {
            if (!font.loadFromFile("assets/troika.otf")) {
                throw std::runtime_error("Failed to load font!");
            }
        }
        catch (const std::exception &e) {
            std::cerr << "Font loading error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        // Таймер для управления частотой обновления
        sf::Clock clock;
        const float updateInterval = 1.0f / 30.0f; // 30 обновлений в секунду
        float elapsedTime = 0.0f;

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
                                if (!audio_transmit) {  // Проверяем, не идет ли передача
                                    lamps[0].changeColor(sf::Color::Red); //Лампочка - индикатор передачи, красная
                                    audio_receive  = false;  // Останавливаем приём звука
                                    audio_transmit = true;
                                    audioTxThread  = std::thread(audioTransmitter, std::ref(buffer), std::ref(audio_transmit));
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
                        audio_transmit = false;  // Устанавливаем флаг завершения
                        try {
                            if (audioTxThread.joinable())
                                audioTxThread.join();
                        }
                        catch (const std::exception &e) {
                            std::cerr << "Exception while joining audioRxThread: " << e.what() << std::endl;
                        }

                        audio_receive = true;  // Возобновляем приём звука
                        lamps[0].changeColor(sf::Color::Black);
                    }
                }  // Закрывающая скобка для if (event.type == sf::Event::MouseButtonReleased)
            }  // Закрывающая скобка для while (window.pollEvent(event))

            // Обновление таймера
            elapsedTime += clock.restart().asSeconds();

            // Если прошло достаточно времени, обновляем экран
            if (elapsedTime >= updateInterval) {
                elapsedTime = 0.0f;

                // Обновление состояния лампочки индикатора приема сигнала
                if (signal_received) {
                    lamps[1].changeColor(sf::Color::Yellow);  // Лампочка загорается желтым при приеме сигнала
                } else {
                    lamps[1].changeColor(sf::Color::Black);  // Лампочка выключается при отсутствии сигнала
                }

                // Очистка и отрисовка
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
            } else {
                // Если обновление не требуется, добавляем небольшую задержку
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }  // Закрывающая скобка для while (window.isOpen())

        // Завершение потоков корректно
        audio_receive = false;
        try {
            if (!audio_receive) {
                if (audioRxThread.joinable())
                    audioRxThread.join();
            }
        }
        catch (const std::exception &e) {
            std::cerr << "Exception while joining audioRxThread: " << e.what() << std::endl;
        }

        try {
            if (!audio_transmit) {
                if (audioTxThread.joinable())
                    audioTxThread.join();
            }
        }
        catch (const std::exception &e) {
            std::cerr << "Exception while joining audioTxThread: " << e.what() << std::endl;
        }

        for (auto &button : buttons)
            delete button;

        // FIXME удаление лампочек
    }
    catch (const std::exception &e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown exception caught in main!" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}

