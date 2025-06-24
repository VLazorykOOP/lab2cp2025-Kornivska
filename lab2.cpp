#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <chrono>
#include <random>
#include <mutex>

using namespace std;
using namespace std::chrono;

mutex cout_mutex;

struct Position {
    double x, y;
};

void developer_thread(int id, double velocity, int change_interval) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> angle_dist(0, 2 * M_PI);

    Position pos{0, 0};
    double angle = angle_dist(gen);

    auto last_change = steady_clock::now();

    while (true) {
        // Зміна напрямку кожні N секунд
        auto now = steady_clock::now();
        if (duration_cast<seconds>(now - last_change).count() >= change_interval) {
            angle = angle_dist(gen);
            last_change = now;
        }

        // Рух
        pos.x += velocity * cos(angle);
        pos.y += velocity * sin(angle);

        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "[Dev " << id << "] x: " << pos.x << " y: " << pos.y << endl;
        }

        this_thread::sleep_for(1s);
    }
}

void manager_thread(int id, double velocity, double radius) {
    double angle = 0;
    Position center{0, 0};
    const double delta = velocity / radius;

    while (true) {
        // Круговий рух
        angle += delta;
        Position pos;
        pos.x = center.x + radius * cos(angle);
        pos.y = center.y + radius * sin(angle);

        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "[Mng " << id << "] x: " << pos.x << " y: " << pos.y << endl;
        }

        this_thread::sleep_for(1s);
    }
}

int main() {
    int num_developers = 3;
    int num_managers = 2;
    double velocity = 1.0;
    int direction_change_interval = 3;
    double radius = 10.0;

    vector<thread> threads;

    // Запускаємо потоки розробників
    for (int i = 0; i < num_developers; ++i) {
        threads.emplace_back(developer_thread, i + 1, velocity, direction_change_interval);
    }

    // Запускаємо потоки менеджерів
    for (int i = 0; i < num_managers; ++i) {
        threads.emplace_back(manager_thread, i + 1, velocity, radius);
    }

    for (auto& t : threads) t.join();

    return 0;
}
