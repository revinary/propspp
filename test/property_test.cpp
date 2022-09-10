#include <catch2/catch_test_macros.hpp>
#include <property_base.h>
#include <bindable_value.h>

using namespace propspp;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
class Car : public has_properties
{
public:
    PROPERTY(speed, int)
};

TEST_CASE("Bindable values")
{
    bindable_value<int> speed;
    REQUIRE(speed == decltype(speed)::value_type {});
    speed = 10;
    REQUIRE(speed == 10);
}

TEST_CASE("Typed property access")
{
    int a;
    Car car;
    REQUIRE(car.speed == decltype(car.speed)::value_type {});
    car.speed = 200;
    REQUIRE(car.speed == 200);
    car.speed = car.speed + 100;
    REQUIRE(car.speed == 300);
}

TEST_CASE("Generic property access")
{
    using speed_t = decltype(Car::speed)::value_type;
    Car car;
    auto& speed = car.get_property("speed");
    // speed = 100;
    REQUIRE(std::any_cast<speed_t>(speed.as_any()) == speed_t {});
    car.set_property("speed", 200);
    REQUIRE(car.speed == 200);
}

TEST_CASE("Custom setter")
{
    class Train : public has_properties
    {
    public:
        PROPERTY(speed, int, setSpeed);

        void setSpeed(int speed)
        {
            this->speed = std::min(speed, 200);
        }
    } train;

    train.speed = 300;
    REQUIRE(train.speed == 200);
    train.setSpeed(100);
    REQUIRE(train.speed == 100);
}

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// bindable_value | property | reflection | custom setter
// implicit conversions bei operator=(std::any)
// property muss name nicht wissen, redundant in jeder instanz
// coming_from_setter muss RAI tun
// kappt setter-direktaufruf?
// property<std::vector> vs changed-signaling
// --> train.cars.push_back(newCar); kann kein changed auslösen
// --> type-conversion weglassen oder nur const&?
// operator += et al implementieren (als template mit requires{T t; t+= T{};})
// --> intern += auf kopie von _value aufrufen und ergebnis zuweisen (wie C#)
// --> ODER NICHT: += sollte es mit der gleichen Motivation wie push_back() nicht geben

// Was ist statisch pro Klasse?
// - Property-Name
// - getter, setter std::functions
//