
#include "SQLid_API.h"

#include "vendor/ImGuiTail.h"
#include <iostream>
#include <memory>
#include <thread>

static boost::asio::io_context io_context;

void inserts() {
    SQLid_API con(io_context, "localhost", "daytime");
    con.connect();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    con.sendText("ATTACH TO base1");
    auto jss = con.readJson();
    print_json_as_table(jss);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto start = std::chrono::steady_clock::now();
    for (int i = 7777; i < 10000; ++i) {

        con.sendText("INSERT INTO nums " + std::to_string(i));
        jss = con.readJson();

    }
    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

}

void deletes() {
    SQLid_API con(io_context, "localhost", "13");
    con.connect();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    con.sendText("ATTACH TO base1");
    auto jss = con.readJson();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    for (int i = 7777; i < 10000; ++i) {

        con.sendText("DELETE FROM nums WHERE num == " + std::to_string(i));
        jss = con.readJson();

    }

}


int main() {
    Window wind;

    // Состояние подключения
    static char ip_buffer[64] = "localhost";
    static char port_buffer[16] = "8080";
    static bool connected = false;
    static std::unique_ptr<SQLid_API> api;
    static std::string console_input;
    static std::vector<std::string> console_history;
    

    while (wind.isValid) {
        wind.NewFrame();

        if (!connected) {
            // Окно подключения
            ImGui::Begin("Database Connection", &wind.isValid, ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::InputText("IP", ip_buffer, sizeof(ip_buffer));
            ImGui::InputText("Port", port_buffer, sizeof(port_buffer));

            if (ImGui::Button("Connect")) {
                try {
                    api = std::make_unique<SQLid_API>(io_context, ip_buffer, port_buffer);
                    api->connect();

                    connected = true;
                    std::cout << "Connected" << std::endl;
                }
                catch (const std::exception& e) {
                    std::cout << std::string("Connection error: ") + e.what() << std::endl;
                }
            }
            if (ImGui::Button("Crash test")) {
                try {
                    std::cout << "Testing" << std::endl;
                    std::jthread ins1(inserts), ins2(inserts), del(deletes);

                    ins1.join();
                    ins2.join();
                    del.join();
                    std::cout << "Test finished" << std::endl;
                }
                catch (...) {
                    std::cout << "Error" << std::endl;
                }
            }
            ImGui::End();
        }
        else {
            std::string buf;
            std::getline(std::cin, buf);
            if (buf == "DISCONNECT" or buf == "0") {
                api.reset(nullptr);
                connected = false;
                std::cout << "Disconnected" << std::endl;
            }
            else {
                auto res = api->request(buf);
                print_json_as_table(res);
            }
        }

        wind.EndFrame();
    }

    

    return 0;
}