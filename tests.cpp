#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include "objects/npc/npc.hpp"
#include "objects/dragon/dragon.hpp"
#include "objects/princess/princess.hpp"
#include "objects/knight/knight.hpp"

TEST(NPCCreation, CreateDragon) {
    Dragon d("D", 10, 20);
    EXPECT_EQ(d.type, DragonType);
    EXPECT_EQ(d.name, "D");
    EXPECT_EQ(d.x, 10);
    EXPECT_EQ(d.y, 20);
}

TEST(NPCCreation, CreatePrincess) {
    Princess p("P", 5, 6);
    EXPECT_EQ(p.type, PrincessType);
    EXPECT_EQ(p.name, "P");
    EXPECT_EQ(p.x, 5);
    EXPECT_EQ(p.y, 6);
}

TEST(NPCCreation, CreateKnight) {
    Knight k("K", 7, 8);
    EXPECT_EQ(k.type, KnightType);
    EXPECT_EQ(k.name, "K");
    EXPECT_EQ(k.x, 7);
    EXPECT_EQ(k.y, 8);
}

TEST(Serialization, SaveAndLoadDragon) {
    auto d = std::make_shared<Dragon>("SavedD", 1, 2);
    std::stringstream ss;
    d->save(ss);
    int type;
    ss >> type;
    EXPECT_EQ(type, DragonType);
    Dragon loaded(ss);
    EXPECT_EQ(loaded.name, "SavedD");
    EXPECT_EQ(loaded.x, 1);
    EXPECT_EQ(loaded.y, 2);
}

TEST(Serialization, SaveAndLoadPrincess) {
    auto p = std::make_shared<Princess>("SavedP", 3, 4);
    std::stringstream ss;
    p->save(ss);
    int type;
    ss >> type;
    EXPECT_EQ(type, PrincessType);
    Princess loaded(ss);
    EXPECT_EQ(loaded.name, "SavedP");
    EXPECT_EQ(loaded.x, 3);
    EXPECT_EQ(loaded.y, 4);
}

TEST(Serialization, SaveAndLoadKnight) {
    auto k = std::make_shared<Knight>("SavedK", 5, 6);
    std::stringstream ss;
    k->save(ss);
    int type;
    ss >> type;
    EXPECT_EQ(type, KnightType);
    Knight loaded(ss);
    EXPECT_EQ(loaded.name, "SavedK");
    EXPECT_EQ(loaded.x, 5);
    EXPECT_EQ(loaded.y, 6);
}

TEST(Distance, CloseNPCs) {
    auto d = std::make_shared<Dragon>("D1", 0, 0);
    auto p = std::make_shared<Princess>("P1", 3, 4);
    EXPECT_TRUE(d->is_close(p, 10));
    EXPECT_TRUE(d->is_close(p, 5));
    EXPECT_FALSE(d->is_close(p, 4));
}

TEST(Distance, FarNPCs) {
    auto d = std::make_shared<Dragon>("D1", 0, 0);
    auto p = std::make_shared<Princess>("P1", 100, 100);
    EXPECT_FALSE(d->is_close(p, 100));
    EXPECT_TRUE(d->is_close(p, 150));
}

TEST(Distance, SamePosition) {
    auto d = std::make_shared<Dragon>("D1", 50, 50);
    auto p = std::make_shared<Princess>("P1", 50, 50);
    EXPECT_TRUE(d->is_close(p, 0));
    EXPECT_TRUE(d->is_close(p, 1));
}

TEST(FightRules, DragonKillsPrincess) {
    auto dragon = std::make_shared<Dragon>("D1", 0, 0);
    auto princess = std::make_shared<Princess>("P1", 0, 0);
    bool result = princess->accept(dragon);
    EXPECT_TRUE(result);
}

TEST(FightRules, PrincessDoesNotKillDragon) {
    auto dragon = std::make_shared<Dragon>("D1", 0, 0);
    auto princess = std::make_shared<Princess>("P1", 0, 0);
    bool result = dragon->accept(princess);
    EXPECT_FALSE(result);
}

TEST(FightRules, PrincessDoesNotKillKnight) {
    auto knight = std::make_shared<Knight>("K1", 0, 0);
    auto princess = std::make_shared<Princess>("P1", 0, 0);
    bool result = knight->accept(princess);
    EXPECT_FALSE(result);
}

TEST(FightRules, KnightKillsDragon) {
    auto knight = std::make_shared<Knight>("K1", 0, 0);
    auto dragon = std::make_shared<Dragon>("D1", 0, 0);
    bool result = dragon->accept(knight);
    EXPECT_TRUE(result);
}

TEST(FightRules, KnightDoesNotKillPrincess) {
    auto knight = std::make_shared<Knight>("K1", 0, 0);
    auto princess = std::make_shared<Princess>("P1", 0, 0);
    bool result = princess->accept(knight);
    EXPECT_FALSE(result);
}

TEST(FightRules, DragonDoesNotKillKnight) {
    auto dragon = std::make_shared<Dragon>("D1", 0, 0);
    auto knight = std::make_shared<Knight>("K1", 0, 0);
    bool result = knight->accept(dragon);
    EXPECT_FALSE(result);
}

TEST(FightRules, DragonDoesNotKillDragon) {
    auto d1 = std::make_shared<Dragon>("D1", 0, 0);
    auto d2 = std::make_shared<Dragon>("D2", 0, 0);
    bool result = d2->accept(d1);
    EXPECT_FALSE(result);
}

class TestObserver : public IFightObserver {
public:
    int fight_count = 0;
    int win_count = 0;
    int loss_count = 0;
    void on_fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender, bool win) override {
        (void)attacker;
        (void)defender;
        fight_count++;
        if (win) win_count++; else loss_count++;
    }
};

TEST(Observer, NotificationOnWin) {
    auto observer = std::make_shared<TestObserver>();
    auto dragon = std::make_shared<Dragon>("D1", 0, 0);
    auto princess = std::make_shared<Princess>("P1", 0, 0);
    dragon->subscribe(observer);
    princess->accept(dragon);
    EXPECT_EQ(observer->fight_count, 1);
    EXPECT_EQ(observer->win_count, 1);
    EXPECT_EQ(observer->loss_count, 0);
}

TEST(Observer, NoNotificationWhenNoKill) {
    auto observer = std::make_shared<TestObserver>();
    auto princess = std::make_shared<Princess>("P1", 0, 0);
    auto dragon = std::make_shared<Dragon>("D1", 0, 0);
    princess->subscribe(observer);
    dragon->accept(princess);
    EXPECT_EQ(observer->fight_count, 0);
    EXPECT_EQ(observer->win_count, 0);
    EXPECT_EQ(observer->loss_count, 0);
}

TEST(Print, DragonOutput) {
    Dragon d("Dr", 1, 2);
    std::stringstream ss;
    d.print(ss);
    std::string out = ss.str();
    EXPECT_TRUE(out.find("Dragon") != std::string::npos);
    EXPECT_TRUE(out.find("Dr") != std::string::npos);
}

TEST(Print, PrincessOutput) {
    Princess p("Pg", 3, 4);
    std::stringstream ss;
    p.print(ss);
    std::string out = ss.str();
    EXPECT_TRUE(out.find("Princess") != std::string::npos);
    EXPECT_TRUE(out.find("Pg") != std::string::npos);
}

TEST(Print, KnightOutput) {
    Knight k("Kt", 5, 6);
    std::stringstream ss;
    k.print(ss);
    std::string out = ss.str();
    EXPECT_TRUE(out.find("Knight") != std::string::npos);
    EXPECT_TRUE(out.find("Kt") != std::string::npos);
}

TEST(Integration, BattleScenario) {
    set_t npcs;
    npcs.insert(std::make_shared<Dragon>("D1", 0, 0));
    npcs.insert(std::make_shared<Princess>("P1", 10, 0));
    npcs.insert(std::make_shared<Knight>("K1", 20, 0));

    EXPECT_EQ(npcs.size(), static_cast<size_t>(3));

    set_t dead_list;
    for (const auto& attacker : npcs) {
        for (const auto& defender : npcs) {
            if ((attacker != defender) && (attacker->is_close(defender, 15))) {
                bool success = defender->accept(attacker);
                if (success) dead_list.insert(defender);
            }
        }
    }

    EXPECT_GE(dead_list.size(), static_cast<size_t>(1));
}

TEST(EdgeCases, EmptyName) {
    Princess p("", 0, 0);
    EXPECT_EQ(p.name, "");
}

TEST(EdgeCases, NegativeCoordinates) {
    Knight k("K", -10, -20);
    EXPECT_EQ(k.x, -10);
    EXPECT_EQ(k.y, -20);
}

TEST(EdgeCases, LargeCoordinates) {
    Dragon d("Big", 100000, 200000);
    EXPECT_EQ(d.x, 100000);
    EXPECT_EQ(d.y, 200000);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
