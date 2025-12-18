#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include "objects/npc/npc.hpp"
#include "objects/dragon/dragon.hpp"
#include "objects/princess/princess.hpp"
#include "objects/knight/knight.hpp"

namespace {
constexpr int kMapWidth = 40;
constexpr int kMapHeight = 20;
constexpr size_t kInitialNpcCount = 50;
constexpr std::chrono::seconds kSimulationDuration{30};
constexpr std::chrono::milliseconds kMovementTick{200};
constexpr std::chrono::milliseconds kPrintInterval{1000};
constexpr double kTwoPi = 6.28318530717958647692;
}

namespace detail {
inline std::mutex console_mutex;
}

class TextObserver : public IFightObserver {
private:
    TextObserver() = default;

public:
    static std::shared_ptr<IFightObserver> get() {
        static TextObserver instance;
        return std::shared_ptr<IFightObserver>(&instance, [](IFightObserver*) {});
    }

    void on_fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender, bool win) override {
        if (!win)
            return;
        std::lock_guard<std::mutex> lock(detail::console_mutex);
        std::cout << std::endl << "Murder --------" << std::endl;
        attacker->print(std::cout);
        defender->print(std::cout);
    }
};

class FileObserver : public IFightObserver {
private:
    std::ofstream fs;
    FileObserver() { fs.open("log.txt"); }

public:
    ~FileObserver() { fs.close(); }
    static std::shared_ptr<IFightObserver> get() {
        static FileObserver instance;
        return std::shared_ptr<IFightObserver>(&instance, [](IFightObserver*) {});
    }

    void on_fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender, bool win) override {
        if (!win)
            return;
        fs << std::endl << "Murder --------" << std::endl;
        attacker->print(fs);
        defender->print(fs);
    }
};

std::shared_ptr<NPC> factory(NpcType type, const std::string& name, int x, int y) {
    std::shared_ptr<NPC> result;
    switch (type) {
    case DragonType:
        result = std::make_shared<Dragon>(name, x, y);
        break;
    case PrincessType:
        result = std::make_shared<Princess>(name, x, y);
        break;
    case KnightType:
        result = std::make_shared<Knight>(name, x, y);
        break;
    default:
        break;
    }
    if (result) {
        result->subscribe(TextObserver::get());
        result->subscribe(FileObserver::get());
    }
    return result;
}

const char* type_label(NpcType type) {
    switch (type) {
    case DragonType: return "Dragon";
    case PrincessType: return "Princess";
    case KnightType: return "Knight";
    default: return "Unknown";
    }
}

char symbol_for_type(NpcType type) {
    switch (type) {
    case DragonType: return 'D';
    case PrincessType: return 'P';
    case KnightType: return 'K';
    default: return '?';
    }
}

struct MovementAttributes {
    double step;
    size_t kill_distance;
};

MovementAttributes get_attributes(NpcType type) {
    switch (type) {
    case DragonType: return {50.0, 30};
    case KnightType: return {30.0, 10};
    case PrincessType: return {1.0, 1};
    default: return {0.0, 0};
    }
}

bool can_kill(NpcType attacker, NpcType defender) {
    return (attacker == DragonType && defender == PrincessType) ||
           (attacker == KnightType && defender == DragonType);
}

struct NPCState {
    std::shared_ptr<NPC> npc;
    bool alive{true};
};

struct FightTask {
    std::shared_ptr<NPC> attacker;
    std::shared_ptr<NPC> defender;
};

NPCState* find_state(std::vector<NPCState>& world, const std::shared_ptr<NPC>& target) {
    for (auto& state : world)
        if (state.npc == target)
            return &state;
    return nullptr;
}

const NPCState* find_state(const std::vector<NPCState>& world,
                           const std::shared_ptr<NPC>& target) {
    for (const auto& state : world)
        if (state.npc == target)
            return &state;
    return nullptr;
}

void print_map(const std::vector<NPCState>& world, std::shared_mutex& world_mutex) {
    std::vector<std::string> grid(kMapHeight, std::string(kMapWidth, '.'));
    {
        std::shared_lock<std::shared_mutex> lock(world_mutex);
        for (const auto& state : world) {
            if (!state.alive)
                continue;
            const auto& npc = state.npc;
            if (npc->x < 0 || npc->x >= kMapWidth || npc->y < 0 || npc->y >= kMapHeight)
                continue;
            grid[npc->y][npc->x] = symbol_for_type(npc->type);
        }
    }
    std::lock_guard<std::mutex> lock(detail::console_mutex);
    std::cout << "Map snapshot:" << std::endl;
    for (const auto& row : grid)
        std::cout << row << std::endl;
    std::cout << std::endl;
}

int main() {
    std::vector<NPCState> world;
    world.reserve(kInitialNpcCount);

    std::random_device rd;
    std::mt19937 type_rng(rd());
    std::uniform_int_distribution<int> type_dist(1, 3);
    std::uniform_int_distribution<int> x_dist(0, kMapWidth - 1);
    std::uniform_int_distribution<int> y_dist(0, kMapHeight - 1);

    for (size_t i = 0; i < kInitialNpcCount; ++i) {
        NpcType type = static_cast<NpcType>(type_dist(type_rng));
        std::string name = std::string(type_label(type)) + "_" + std::to_string(i);
        auto npc = factory(type, name, x_dist(type_rng), y_dist(type_rng));
        if (npc)
            world.push_back({std::move(npc), true});
    }

    std::shared_mutex world_mutex;
    std::deque<FightTask> fight_queue;
    std::mutex fight_mutex;
    std::condition_variable fight_cv;
    std::atomic<bool> running{true};

    auto movement_thread = std::thread([&]() {
        std::mt19937 rng(rd() + 1);
        std::uniform_real_distribution<double> angle_dist(0.0, kTwoPi);
        std::uniform_real_distribution<double> length_dist(0.0, 1.0);
        while (running.load()) {
            {
                std::lock_guard<std::shared_mutex> lock(world_mutex);
                for (auto& state : world) {
                    if (!state.alive)
                        continue;
                    const auto attr = get_attributes(state.npc->type);
                    double angle = angle_dist(rng);
                    double length = length_dist(rng) * attr.step;
                    int dx = static_cast<int>(std::round(std::cos(angle) * length));
                    int dy = static_cast<int>(std::round(std::sin(angle) * length));
                    state.npc->x =
                        std::clamp(state.npc->x + dx, 0, kMapWidth - 1);
                    state.npc->y =
                        std::clamp(state.npc->y + dy, 0, kMapHeight - 1);
                }
            }

            std::vector<FightTask> candidates;
            {
                std::shared_lock<std::shared_mutex> lock(world_mutex);
                const size_t count = world.size();
                for (size_t i = 0; i < count; ++i) {
                    const auto& attacker_state = world[i];
                    if (!attacker_state.alive)
                        continue;
                    const auto attr = get_attributes(attacker_state.npc->type);
                    if (attr.kill_distance == 0)
                        continue;
                    for (size_t j = 0; j < count; ++j) {
                        if (i == j)
                            continue;
                        const auto& defender_state = world[j];
                        if (!defender_state.alive)
                            continue;
                        if (!can_kill(attacker_state.npc->type, defender_state.npc->type))
                            continue;
                        if (attacker_state.npc->is_close(defender_state.npc, attr.kill_distance))
                            candidates.push_back(
                                {attacker_state.npc, defender_state.npc});
                    }
                }
            }

            if (!candidates.empty()) {
                {
                    std::lock_guard<std::mutex> queue_lock(fight_mutex);
                    fight_queue.insert(fight_queue.end(),
                                       candidates.begin(), candidates.end());
                }
                fight_cv.notify_one();
            }

            std::this_thread::sleep_for(kMovementTick);
        }
        fight_cv.notify_all();
    });

    auto fight_thread = std::thread([&]() {
        std::mt19937 dice_rng(rd() + 2);
        std::uniform_int_distribution<int> dice(1, 6);
        while (running.load() || !fight_queue.empty()) {
            FightTask task;
            {
                std::unique_lock<std::mutex> queue_lock(fight_mutex);
                fight_cv.wait(queue_lock, [&]() {
                    return !fight_queue.empty() || !running.load();
                });
                if (fight_queue.empty()) {
                    if (!running.load())
                        break;
                    continue;
                }
                task = fight_queue.front();
                fight_queue.pop_front();
            }

            {
                std::shared_lock<std::shared_mutex> read_lock(world_mutex);
                const auto* attacker_state = find_state(world, task.attacker);
                const auto* defender_state = find_state(world, task.defender);
                if (!attacker_state || !defender_state || !attacker_state->alive ||
                    !defender_state->alive) {
                    continue;
                }
            }

            int attack = dice(dice_rng);
            int defense = dice(dice_rng);
            if (attack <= defense)
                continue;

            bool killed = false;
            {
                std::lock_guard<std::shared_mutex> write_lock(world_mutex);
                auto* attacker_state = find_state(world, task.attacker);
                auto* defender_state = find_state(world, task.defender);
                if (attacker_state && defender_state && attacker_state->alive &&
                    defender_state->alive) {
                    defender_state->alive = false;
                    killed = true;
                }
            }

            if (killed)
                task.attacker->fight_notify(task.defender, true);
        }
    });

    auto start_time = std::chrono::steady_clock::now();
    auto next_print = start_time;
    const auto end_time = start_time + kSimulationDuration;
    while (std::chrono::steady_clock::now() < end_time) {
        print_map(world, world_mutex);
        next_print += kPrintInterval;
        std::this_thread::sleep_until(next_print);
    }

    running = false;
    fight_cv.notify_all();
    movement_thread.join();
    fight_thread.join();

    std::vector<std::shared_ptr<NPC>> survivors;
    {
        std::shared_lock<std::shared_mutex> lock(world_mutex);
        for (const auto& state : world)
            if (state.alive)
                survivors.push_back(state.npc);
    }

    {
        std::lock_guard<std::mutex> lock(detail::console_mutex);
        std::cout << "Simulation finished. Survivors: " << survivors.size()
                  << std::endl;
        for (const auto& npc : survivors)
            std::cout << type_label(npc->type) << ": " << npc->name << " ("
                      << npc->x << ", " << npc->y << ")" << std::endl;
    }

    return 0;
}
