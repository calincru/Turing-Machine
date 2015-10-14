// This program is free software licenced under MIT Licence. You can
// find a copy of this licence in LICENCE.txt in the top directory of
// source code.
//

#include "../include/turing_machine.hpp"

using namespace turingmachine;

class do_nothing final: public tm_abstract_problem {
  public:
    const char *name() const override {
        return "Empty";
    }

    void configure() override {
        // Do nothing, tape should remain the same
    }

    void add_units() override {
        ADD_UNIT_TEST(">#01#", ">#01#");
        ADD_UNIT_TEST(">", ">");
        ADD_UNIT_TEST(">9$1#", ">9$1#");
    }
};

class dummy final: public tm_abstract_problem {
  public:
    const char *name() const override {
        return "Dummy";
    }

    void configure() override {
        // Note that the last state is considered the final one.
        ADD_TRANSITION(0, '#', 1, '0', GO_RIGHT);
    }

    void add_units() override {
        ADD_UNIT_TEST(">#01#", ">001#");
    }
};

int main() {
    do_nothing{}.run();
    dummy{}.run();
}
