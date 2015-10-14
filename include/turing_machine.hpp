// This program is free software licenced under MIT Licence. You can
// find a copy of this licence in LICENCE.txt in the top directory of
// source code.
//

#ifndef INCLUDED_TURING_MACHINE
#define INCLUDED_TURING_MACHINE

// C++ headers
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

// C headers
#include <cassert>


#define TEST_OUTPUT(testNr, expected, actual) \
    do { \
        if (expected == actual) \
            std::cout << "Test " << testNr << " succeded" << std::endl; \
        else \
            std::cout << "Test " << testNr << " failed: " \
                      << "Expected: " << expected << "; " \
                      << "Actual: " << actual << std::endl; \
    } while (false)

#define ADD_TRANSITION(state_in, sym_in, state_out, sym_out, shift) \
    do { \
        d_config.add_transition( \
            turingmachine::transition_in{state_in, sym_in}, \
            turingmachine::transition_out{state_out, sym_out, shift} \
        ); \
    } while (false)

#define EXPECTED_TAPE_ON(input, expected_output) \
    do { \
        d_tests.emplace_back(input, expected_output); \
    } while (false)

namespace turingmachine {
    struct transition_in {
        int state_in;
        char sym_in;
    };

    bool operator==(const transition_in &lhs, const transition_in &rhs) {
        return lhs.state_in == rhs.state_in
            && lhs.sym_in == rhs.sym_in;
    }

    struct transition_hash {
        static constexpr auto PRIME1 = 2879;
        static constexpr auto PRIME2 = 709;

        std::size_t operator()(const transition_in &trans) const noexcept {
            return trans.state_in * PRIME1
                 + trans.sym_in * PRIME2;
        }
    };

    enum MOVE_DIRECTION {
        GO_LEFT = -1,
        GO_RIGHT = 1,
        HOLD = 0
    };

    struct transition_out {
        int state_out;
        char sym_out;
        int direction;
    };

    class tm_configuration final {
        // DATA
        std::unordered_map<transition_in,
                           transition_out,
                           transition_hash> d_delta;
        int d_states;

      public:
        tm_configuration()
            : d_states{0} {
            // Nothing to do
        }

        void add_transition(const transition_in &trans_in,
                            const transition_out &trans_out) {
            d_delta.emplace(trans_in, trans_out);

            // Educated guess. May change in the future - machines with
            // multiple final states.
            d_states
                = std::max({d_states, trans_in.state_in, trans_out.state_out});
        }

        bool is_final_state(int state) const {
            return !(state < d_states);
        }

        bool is_undefined_state(int state, char sym) const {
            return d_delta.find(transition_in{state, sym}) == d_delta.end();
        }

        transition_out get_trans_out(int state, char sym) const {
            return d_delta.at(transition_in{state, sym});
        }
    };

    class tm_abstract_problem {
      protected:
        // DATA
        std::vector<std::pair<std::string, std::string>> d_tests;
        tm_configuration d_config;

      public:
        void run() {
            std::cout << "Running " << name() << std::endl;

            configure();
            add_units();

            for (auto it = d_tests.cbegin(); it != d_tests.cend(); ++it) {
                auto tape = it->first;
                auto tape_head = 1;
                auto current_state = 0;

                while (!d_config.is_final_state(current_state)) {
                    assert(!d_config.is_undefined_state(current_state,
                                                        tape[tape_head]));

                    auto trans_out = d_config.get_trans_out(current_state,
                                                            tape[tape_head]);

                    tape[tape_head] = trans_out.sym_out;
                    tape_head += trans_out.direction;
                    current_state = trans_out.state_out;
                }

                TEST_OUTPUT(it - d_tests.cbegin() + 1, it->second, tape);
            }

            std::cout << std::endl;
        }

        virtual const char *name() const = 0;
        virtual void configure() = 0;
        virtual void add_units() = 0;
    };
} // namespace turingmachine

#endif // INCLUDED_TURING_MACHINE
