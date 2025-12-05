#pragma once

#include "../npc/npc.hpp"
#include <memory>

struct Dragon : public NPC {
    Dragon(const std::string& name, int x, int y);
    Dragon(std::istream& is);

    void print(std::ostream& os) override;
    void save(std::ostream& os) override;

    bool visit(std::shared_ptr<Dragon> other) override;
    bool visit(std::shared_ptr<Princess> other) override;
    bool visit(std::shared_ptr<Knight> other) override;

    bool accept(std::shared_ptr<NPC> attacker) override;

    friend std::ostream& operator<<(std::ostream& os, Dragon& dragon);
};

inline Dragon::Dragon(const std::string& name, int x, int y)
    : NPC(DragonType, name, x, y) {}

inline Dragon::Dragon(std::istream& is) : NPC(DragonType, is) {}

inline void Dragon::print(std::ostream& os) {
    os << *this;
}

inline void Dragon::save(std::ostream& os) {
    os << DragonType << std::endl;
    NPC::save(os);
}

inline bool Dragon::visit(std::shared_ptr<Dragon> other) {
    (void)other;
    return false;
}

inline bool Dragon::visit(std::shared_ptr<Princess> other) {
    fight_notify(std::static_pointer_cast<NPC>(other), true);
    return true;
}

inline bool Dragon::visit(std::shared_ptr<Knight> other) {
    (void)other;
    return false;
}

inline bool Dragon::accept(std::shared_ptr<NPC> attacker) {
    return attacker->visit(std::dynamic_pointer_cast<Dragon>(shared_from_this()));
}

inline std::ostream& operator<<(std::ostream& os, Dragon& dragon) {
    os << "Dragon: " << dragon.name << " " << *static_cast<NPC*>(&dragon) << std::endl;
    return os;
}
