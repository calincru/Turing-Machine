// This program is free software licenced under MIT Licence. You can
// find a copy of this licence in LICENCE.txt in the top directory of
// source code.
//

#include "../include/turing_machine.hpp"

using namespace turingmachine;

class increment final: public tm_abstract_problem {
  public:
    const char *name() const override {
        return "Increment";
    }

    void configure() override {
        ADD_TRANSITION(0, '0', 0, '0', GO_RIGHT);
        ADD_TRANSITION(0, '1', 0, '1', GO_RIGHT);
        ADD_TRANSITION(0, '#', 1, '#', GO_LEFT);

        ADD_TRANSITION(1, '1', 1, '0', GO_LEFT);
        ADD_TRANSITION(1, '0', 2, '1', HOLD);
        ADD_TRANSITION(1, '>', 2, '>', HOLD);
    }

    void add_units() override {
        EXPECTED_TAPE_ON(">0100#", ">0101#");
        EXPECTED_TAPE_ON(">0000#", ">0001#");
        EXPECTED_TAPE_ON(">0001#", ">0010#");
        EXPECTED_TAPE_ON(">0101#", ">0110#");
        EXPECTED_TAPE_ON(">1111#", ">0000#");
    }
};

int main() {
    increment{}.run();
}
