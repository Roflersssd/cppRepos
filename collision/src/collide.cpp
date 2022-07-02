#include "../headers/geo2d.h"
#include "../headers/game_object.h"

#include "../../test_runner.h"

#include <vector>
#include <memory>

using namespace std;

class Unit : public GameObject {
public:
    explicit Unit(geo2d::Point position) : unitPoint(position) {};
    bool Collide(const GameObject& that) const override {
        return that.Collide(*this);
    }
    bool CollideWith(const Unit& that) const override {
        return geo2d::Collide(unitPoint, that.getPoint());
    }
    bool CollideWith(const Building& that) const {
        return geo2d::Collide(unitPoint, that.getRect());
    }
    bool CollideWith(const Tower& that) const {
        return geo2d::Collide(unitPoint, that.getCircle());
    }
    bool CollideWith(const Fence& that) const {
        return geo2d::Collide(unitPoint, that.getSegment());
    }

    geo2d::Point getPoint() const {
        return unitPoint;
    }
private:
    geo2d::Point unitPoint;
};

class Building : public GameObject{
public:
    explicit Building(geo2d::Rectangle geometry) : buildingRect(geometry) {};
    bool Collide(const GameObject& that) const override {
        return that.Collide(*this);
    }
    bool CollideWith(const Unit& that) const override {
        return geo2d::Collide(buildingRect, that.getPoint());
    }
    bool CollideWith(const Building& that) const {
        return geo2d::Collide(buildingRect, that.getRect());
    }
    bool CollideWith(const Tower& that) const {
        return geo2d::Collide(buildingRect, that.getCircle());
    }
    bool CollideWith(const Fence& that) const {
        return geo2d::Collide(buildingRect, that.getSegment());
    }
    geo2d::Rectangle getRect() const {
        return buildingRect;
    }
private:
    geo2d::Rectangle buildingRect;
};

class Tower : public GameObject {
public:
    explicit Tower(geo2d::Circle geometry): d_towerCircle(geometry){}

    bool Collide(const GameObject& that) const override {
        return that.Collide(*this);
    }

    bool CollideWith(const Unit& that) const {
        return geo2d::Collide(d_towerCircle, that.getPoint());
    }

    bool CollideWith(const Building& that) const {
        return geo2d::Collide(d_towerCircle, that.getRect());
    }

    bool CollideWith(const Tower& that) const {
        return geo2d::Collide(d_towerCircle, that.getCircle());
    }

    bool CollideWith(const Fence& that) const {
        return geo2d::Collide(d_towerCircle, that.getSegment());
    }

    geo2d::Circle getCircle() const {
        return d_towerCircle;
    }
private:
    geo2d::Circle d_towerCircle;
};

class Fence : public GameObject {
public:
    explicit Fence(geo2d::Segment geometry) : fenceSecment(geometry) {};
    bool Collide(const GameObject& that) const override {
        return that.Collide(*this);
    }
    bool CollideWith(const Unit& that) const {
        return geo2d::Collide(fenceSecment, that.getPoint());
    }
    bool CollideWith(const Building& that) const {
        return geo2d::Collide(fenceSecment, that.getRect());
    }
    bool CollideWith(const Tower& that) const {
        return geo2d::Collide(fenceSecment, that.getCircle());
    }
    bool CollideWith(const Fence& that) const {
        return geo2d::Collide(fenceSecment, that.getSegment());
    }

    geo2d::Segment getSegment() const {
        return fenceSecment;
    }
private:
    geo2d::Segment fenceSecment;
};

// � еализуйте функцию Collide из файла GameObject.h
namespace last {
    bool Collide(const GameObject& first, const GameObject& second) {
        return first.Collide(second);
    }
}

void TestAddingNewObjectOnMap() {
    // Юнит-тест моделирует ситуацию, когда на игровой карте уже есть какие-то объекты,
    // и мы хотим добавить на неё новый, например, построить новое здание или башню.
    // Мы можем его добавить, только если он не пересекается ни с одним из существующих.
    using namespace geo2d;

    const vector<shared_ptr<GameObject>> game_map = {
      make_shared<Unit>(Point{3, 3}),
      make_shared<Unit>(Point{5, 5}),
      make_shared<Unit>(Point{3, 7}),
      make_shared<Fence>(Segment{{7, 3}, {9, 8}}),
      make_shared<Tower>(Circle{Point{9, 4}, 1}),
      make_shared<Tower>(Circle{Point{10, 7}, 1}),
      make_shared<Building>(Rectangle{{11, 4}, {14, 6}})
    };

    for (size_t i = 0; i < game_map.size(); ++i) {
        Assert(
            Collide(*game_map[i], *game_map[i]),
            "An object doesn't collide with itself: " + to_string(i)
        );

        for (size_t j = 0; j < i; ++j) {
            Assert(
                !Collide(*game_map[i], *game_map[j]),
                "Unexpected collision found " + to_string(i) + ' ' + to_string(j)
            );
        }
    }

    auto new_warehouse = make_shared<Building>(Rectangle{ {4, 3}, {9, 6} });
    ASSERT(!last::Collide(*new_warehouse, *game_map[0]));
    ASSERT(last::Collide(*new_warehouse, *game_map[1]));
    ASSERT(!last::Collide(*new_warehouse, *game_map[2]));
    ASSERT(last::Collide(*new_warehouse, *game_map[3]));
    ASSERT(last::Collide(*new_warehouse, *game_map[4]));
    ASSERT(!last::Collide(*new_warehouse, *game_map[5]));
    ASSERT(!last::Collide(*new_warehouse, *game_map[6]));

    auto new_defense_tower = make_shared<Tower>(Circle{ {8, 2}, 2 });
    ASSERT(!last::Collide(*new_defense_tower, *game_map[0]));
    ASSERT(!last::Collide(*new_defense_tower, *game_map[1]));
    ASSERT(!last::Collide(*new_defense_tower, *game_map[2]));
    ASSERT(last::Collide(*new_defense_tower, *game_map[3]));
    ASSERT(last::Collide(*new_defense_tower, *game_map[4]));
    ASSERT(!last::Collide(*new_defense_tower, *game_map[5]));
    ASSERT(!last::Collide(*new_defense_tower, *game_map[6]));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestAddingNewObjectOnMap);
    return 0;
}