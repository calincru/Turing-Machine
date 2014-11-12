#include <iostream>
#include <unordered_map>
#include <tuple>
#include <exception>
#include <vector>


namespace TM
{

enum SHIFT {
    LEFT = -1,
    RIGHT = 1,
    HOLD = 0
};

class MultipleValuesMappedToKey : public std::exception
{
private:
    virtual const char *what() const throw()
    {
        return "Cannot map multiple values to a key in an unordered_map";
    }
};

class KeyNotFound : public std::exception
{
private:
    virtual const char *what() const throw()
    {
        return "Key not found. Maybe first test using contains?";
    }
};

bool operator==(const std::tuple<int, char> &a,
                const std::tuple<int, char> &b)
{
    return std::get<0>(a) == std::get<0>(b) &&
           std::get<1>(a) == std::get<1>(b);
}

struct tuple_hash : public std::unary_function<std::tuple<int, char>,
                                               std::size_t>
{
    const int MOD = 666013;

    std::size_t operator()(const std::tuple<int, char> &k) const
    {
        return (std::get<0>(k) + std::get<1>(k)) % MOD;
    }
};

class TMTransition
{
public:
    typedef std::tuple<int, char> key_type;
    typedef std::tuple<int, char, SHIFT> value_type;


    void emplace(const key_type &k, const value_type &v) throw()
    {
        if (delta_.count(k))
            throw new MultipleValuesMappedToKey();

        delta_.emplace(k, v);
    }

    void emplace(key_type &&k, value_type &&v) throw()
    {
        if (delta_.count(k))
            throw new MultipleValuesMappedToKey();

        delta_.emplace(std::move(k), std::move(v));
    }

    value_type get(const key_type &k) const
    {
        if (!delta_.count(k))
            throw new KeyNotFound();

        return delta_.find(k)->second;
    }

    bool contains(const key_type &k) const
    {
        return delta_.count(k);
    }

private:
    std::unordered_map<key_type, value_type, tuple_hash> delta_;
};

class TMConfiguration
{
public:
    TMConfiguration()
        : statesNr_(0)
    {
    }

    void addTransition(int state_in, char sym_in, int state_af,
                       char sym_af, SHIFT shift ) throw()
    {
        addTransition(std::make_tuple(state_in, sym_in),
                      std::make_tuple(state_af, sym_af, shift));

        // Educated guess. May change in the future - machines with multiple
        // final states.
        if (statesNr_ < state_in || statesNr_ < state_af)
            statesNr_ = std::max(state_af, state_in);
    }

    bool is_final(int state) const
    {
        return state >= statesNr_;
    }

    bool is_undefined(int state, char sym) const
    {
        return !delta_.contains(std::make_tuple(state, sym));
    }

    TMTransition::value_type getValue(int state, char sym) const
    {
        return delta_.get(std::make_tuple(state, sym));
    }

private:
    void addTransition(const TMTransition::key_type &k,
                       const TMTransition::value_type &v)
    {
        delta_.emplace(k, v);
    }

    void addTransition(TMTransition::key_type &&k,
                       TMTransition::value_type &&v)
    {
        delta_.emplace(std::move(k), std::move(v));
    }


    TMTransition delta_;
    int statesNr_;
};

class TMRuntime
{
public:
    TMRuntime(const TMConfiguration &conf)
        : conf_(conf)
    {
    }

    std::string run(std::string tape) throw()
    {
        int tape_head = 1;
        int current_state = 0;

        while (!conf_.is_final(current_state)) {
#ifdef VERBOSE
            print(current_state, tape_head, tape);
#endif

            int curr_sym = static_cast<int>(tape[tape_head]);
            TMTransition::value_type val = conf_.getValue(current_state,
                                                          curr_sym);

            tape[tape_head] = std::get<1>(val);
            tape_head += std::get<2>(val);
            current_state = std::get<0>(val);
        }

#ifdef VERBOSE
        print(current_state, tape_head, tape);
#endif
        return tape;
    }

private:
#ifdef VERBOSE
    void print(int current_state, int tape_head,
               const std::string &current_tape)
    {
        std::string head_str(current_tape.size(), ' ');
        head_str[tape_head] = '^';

        if (conf_.is_final(current_state))
            std::cout << "Final state: ";
        else
            std::cout << "Current State: ";

        std::cout << current_state << std::endl;
        std::cout << current_tape << std::endl;
        std::cout << head_str;

        std::cout << std::endl;
    }
#endif

    TMConfiguration conf_;
};

namespace unittest
{

#define TEST_OUTPUT(testNr, expected, actual)\
    do {\
        if (expected == actual)\
            std::cout << "Test " << testNr << " succeded" << std::endl;\
        else\
            std::cout << "Test " << testNr << " failed: "\
                      << "Expected: " << expected << "; "\
                      << "Actual: " << actual << std::endl;\
    } while (false)


class UnitTest
{
public:
    void runTest()
    {
        std::cout << "Running " << name() << std::endl;

        init();
        addUnitTests();
        TMRuntime *tm_runtime = new TMRuntime(tmConf_);

        for (auto it = testCases_.cbegin(); it != testCases_.cend(); ++it)
            TEST_OUTPUT(it - testCases_.cbegin() + 1, it->second,
                        tm_runtime->run(it->first));

        std::cout << std::endl;
        delete tm_runtime;
    }

protected:
    TMConfiguration tmConf_;
    std::vector<std::pair<std::string, std::string>> testCases_;

private:
    virtual const char *name() = 0;
    virtual void init() = 0;
    virtual void addUnitTests() = 0;
};

#define ADD_TRANSITION(state_in, sym_in, state_after, sym_after, shift)\
    do {\
        tmConf_.addTransition(state_in, sym_in, state_after, sym_after, shift);\
    } while (false)

#define ADD_UNIT_TEST(input, expected_output)\
    do {\
        testCases_.emplace_back(input, expected_output);\
    } while (false)

} // namespace TM::unittest

} // namespace TM




using namespace TM;

class MatrixTest : public ::unittest::UnitTest
{
    const char *name() override
    {
        return "MatrixTest";
    }

    void init() override
    {
        // TODO
    }

    void addUnitTests() override
    {
        ADD_UNIT_TEST(">1100101001110010#", "1#################");
        ADD_UNIT_TEST(">0000000000000000#", "0#################");
        ADD_UNIT_TEST(">1100101001110010#", "1#################");
        ADD_UNIT_TEST(">0000000000000000#", "0#################");
        ADD_UNIT_TEST(">0000000000010000#", "1#################");
        ADD_UNIT_TEST(">1100101001100010#", "0#################");
    }
};

class AnagramsTest : public ::unittest::UnitTest
{
    const char *name() override
    {
        return "AnagramsTest";
    }

    void init() override
    {
        // TODO
    }

    void addUnitTests() override
    {
        ADD_UNIT_TEST(">10101_10011#", "1############");
        ADD_UNIT_TEST(">1101_10011#", "0###########");
        ADD_UNIT_TEST(">10101_1001100#", "0##############");
        ADD_UNIT_TEST(">_1001100#", "0#########");
        ADD_UNIT_TEST(">10101_#", "0#######");
        ADD_UNIT_TEST(">11111_11111#", "1############");
        ADD_UNIT_TEST(">00000_00000#", "1############");
        ADD_UNIT_TEST(">_#", "1##");
    }
};

class CountZerosTest : public ::unittest::UnitTest
{
    const char *name() override
    {
        return "CountZerosTest";
    }

    void init() override
    {
        // TODO
    }

    void addUnitTests() override
    {
        ADD_UNIT_TEST(">10010#", "####11#");
        ADD_UNIT_TEST(">1001011#", "######11#");
        ADD_UNIT_TEST(">1111111#", "########0");
    }
};


int main()
{
    unittest::UnitTest *test1 = new MatrixTest();
    unittest::UnitTest *test2 = new AnagramsTest();
    unittest::UnitTest *test3 = new CountZerosTest();

    test1->runTest();
    test2->runTest();
    test3->runTest();

    return 0;
}
