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
        EXPECTED_TAPE_ON(">#01#", ">#01#");
        EXPECTED_TAPE_ON(">", ">");
        EXPECTED_TAPE_ON(">9$1#", ">9$1#");
    }
};

class dummy final: public tm_abstract_problem {
  public:
    const char *name() const override {
        return "Dummy";
    }

    void configure() override {
        // Read: If we are in state 0 and we come across a '#', then we switch
        // to state 1, we write '0' on the tape and GO to the RIGHT.
        //
        // Note that the last state is considered the final one.
        ADD_TRANSITION(0, '#', 1, '0', GO_RIGHT);
    }

    void add_units() override {
        EXPECTED_TAPE_ON(">#01#", ">001#");
    }
};

int main() {
    do_nothing{}.run();
    dummy{}.run();
}
