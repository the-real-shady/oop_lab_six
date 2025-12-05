#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include <fstream>

struct NPC;
struct Dragon;
struct Princess;
struct Knight;

using set_t = std::set<std::shared_ptr<NPC>>;

enum NpcType {
    Unknown = 0,
    DragonType = 1,
    PrincessType = 2,
    KnightType = 3
};

struct IFightObserver {
    virtual void on_fight(const std::shared_ptr<NPC> attacker,
                          const std::shared_ptr<NPC> defender, bool win) = 0;
    virtual ~IFightObserver() = default;
};

struct NPC : public std::enable_shared_from_this<NPC> {
    NpcType type;
    std::string name;
    int x{0};
    int y{0};
    std::vector<std::shared_ptr<IFightObserver>> observers;

    NPC(NpcType t, const std::string& n, int _x, int _y);
    NPC(NpcType t, std::istream& is);

    void subscribe(std::shared_ptr<IFightObserver> observer);
    void fight_notify(const std::shared_ptr<NPC> defender, bool win);
    bool is_close(const std::shared_ptr<NPC>& other, size_t distance) const;

    virtual bool visit(std::shared_ptr<Dragon> other) = 0;
    virtual bool visit(std::shared_ptr<Princess> other) = 0;
    virtual bool visit(std::shared_ptr<Knight> other) = 0;

    virtual bool accept(std::shared_ptr<NPC> attacker) = 0;

    virtual void print(std::ostream& os) = 0;
    virtual void save(std::ostream& os);

    virtual ~NPC() = default;

    friend std::ostream& operator<<(std::ostream& os, NPC& npc);
};

inline NPC::NPC(NpcType t, const std::string& n, int _x, int _y)
    : type(t), name(n), x(_x), y(_y) {}

inline NPC::NPC(NpcType t, std::istream& is) : type(t) {
    is >> name >> x >> y;
}

inline void NPC::subscribe(std::shared_ptr<IFightObserver> observer) {
    observers.push_back(observer);
}

inline void NPC::fight_notify(const std::shared_ptr<NPC> defender, bool win) {
    for (auto& o : observers) {
        o->on_fight(shared_from_this(), defender, win);
    }
}

inline bool NPC::is_close(const std::shared_ptr<NPC>& other, size_t distance) const {
    long dx = x - other->x;
    long dy = y - other->y;
    long lhs = dx * dx + dy * dy;
    long rhs = static_cast<long>(distance) * static_cast<long>(distance);
    return lhs <= rhs;
}

inline void NPC::save(std::ostream& os) {
    os << name << std::endl << x << std::endl << y << std::endl;
}

inline std::ostream& operator<<(std::ostream& os, NPC& npc) {
    os << "{ x:" << npc.x << ", y:" << npc.y << "} ";
    return os;
}
