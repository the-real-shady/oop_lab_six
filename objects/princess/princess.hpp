#pragma once

#include "../npc/npc.hpp"
#include <memory>

struct Princess : public NPC {
    Princess(const std::string& name, int x, int y);
    Princess(std::istream& is);

    void print(std::ostream& os) override;
    void save(std::ostream& os) override;

    bool visit(std::shared_ptr<Dragon> other) override;
    bool visit(std::shared_ptr<Princess> other) override;
    bool visit(std::shared_ptr<Knight> other) override;

    bool accept(std::shared_ptr<NPC> attacker) override;

    friend std::ostream& operator<<(std::ostream& os, Princess& princess);
};

inline Princess::Princess(const std::string& name, int x, int y)
    : NPC(PrincessType, name, x, y) {}

inline Princess::Princess(std::istream& is) : NPC(PrincessType, is) {}

inline void Princess::print(std::ostream& os) {
    os << *this;
}

inline void Princess::save(std::ostream& os) {
    os << PrincessType << std::endl;
    NPC::save(os);
}

inline bool Princess::visit(std::shared_ptr<Dragon> other) {
    (void)other;
    return false;
}

inline bool Princess::visit(std::shared_ptr<Princess> other) {
    (void)other;
    return false;
}

inline bool Princess::visit(std::shared_ptr<Knight> other) {
    (void)other;
    return false;
}

inline bool Princess::accept(std::shared_ptr<NPC> attacker) {
    return attacker->visit(std::dynamic_pointer_cast<Princess>(shared_from_this()));
}

inline std::ostream& operator<<(std::ostream& os, Princess& princess) {
    os << "Princess: " << princess.name << " " << *static_cast<NPC*>(&princess) << std::endl;
    return os;
}
