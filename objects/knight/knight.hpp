#pragma once

#include "../npc/npc.hpp"
#include <memory>

struct Knight : public NPC {
    Knight(const std::string& name, int x, int y);
    Knight(std::istream& is);

    void print(std::ostream& os) override;
    void save(std::ostream& os) override;

    bool visit(std::shared_ptr<Dragon> other) override;
    bool visit(std::shared_ptr<Princess> other) override;
    bool visit(std::shared_ptr<Knight> other) override;

    bool accept(std::shared_ptr<NPC> attacker) override;

    friend std::ostream& operator<<(std::ostream& os, Knight& knight);
};

inline Knight::Knight(const std::string& name, int x, int y)
    : NPC(KnightType, name, x, y) {}

inline Knight::Knight(std::istream& is) : NPC(KnightType, is) {}

inline void Knight::print(std::ostream& os) {
    os << *this;
}

inline void Knight::save(std::ostream& os) {
    os << KnightType << std::endl;
    NPC::save(os);
}

inline bool Knight::visit(std::shared_ptr<Dragon> other) {
    fight_notify(std::static_pointer_cast<NPC>(other), true);
    return true;
}

inline bool Knight::visit(std::shared_ptr<Princess> other) {
    (void)other;
    return false;
}

inline bool Knight::visit(std::shared_ptr<Knight> other) {
    (void)other;
    return false;
}

inline bool Knight::accept(std::shared_ptr<NPC> attacker) {
    return attacker->visit(std::dynamic_pointer_cast<Knight>(shared_from_this()));
}

inline std::ostream& operator<<(std::ostream& os, Knight& knight) {
    os << "Wandering Knight: " << knight.name << " " << *static_cast<NPC*>(&knight) << std::endl;
    return os;
}
