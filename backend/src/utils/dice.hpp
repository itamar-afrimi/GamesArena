#pragma once
#include <random>

class Dice {
private:
    std::mt19937 gen;
    std::uniform_int_distribution<> dist;

public:
    Dice(int sides = 6)
        : gen(std::random_device{}()), dist(1, sides) {}

    int roll() {
        return dist(gen);
    }
};
