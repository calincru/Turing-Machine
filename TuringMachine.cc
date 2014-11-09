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

    TMConfiguration conf_;
};


#define TEST_OUTPUT(testNr, expected, actual)\
{\
    if (expected == actual)\
        std::cout << "Test " << testNr << " succeded" << std::endl;\
    else\
        std::cout << "Test " << testNr << " failed: "\
                  << "Expected: " << expected << "; "\
                  << "Actual: " << actual << std::endl;\
}

namespace unittest
{

class UnitTest
{
public:
    void runTest()
    {
        std::cout << "Running " << name() << std::endl;

        init();
        TMRuntime *tm_runtime = new TMRuntime(tmConf_);

        auto inouts = getInOuts();
        for (auto it = inouts.cbegin(); it != inouts.cend(); ++it)
            TEST_OUTPUT(it - inouts.cbegin() + 1, it->second,
                        tm_runtime->run(it->first))

        std::cout << std::endl;
        delete tm_runtime;
    }

protected:
    TMConfiguration tmConf_;

private:
    virtual const char *name() = 0;
    virtual void init() = 0;
    virtual std::vector<std::pair<std::string, std::string>> getInOuts() = 0;
};

}

}




using namespace TM;

class IncrementTest : public ::unittest::UnitTest
{
private:
    const char *name() override
    {
        return "IncrementTest";
    }

    void init() override
    {
        tmConf_.addTransition(0, '0', 0, '0', RIGHT);
        tmConf_.addTransition(0, '1', 0, '1', RIGHT);
        tmConf_.addTransition(0, '#', 1, '#', LEFT);
        tmConf_.addTransition(1, '0', 2, '1', LEFT);
        tmConf_.addTransition(1, '1', 1, '0', LEFT);
        tmConf_.addTransition(2, '0', 2, '0', LEFT);
        tmConf_.addTransition(2, '1', 2, '1', LEFT);
        tmConf_.addTransition(2, '>', 3, '>', HOLD);
    }

    std::vector<std::pair<std::string, std::string>> getInOuts() override
    {
        std::vector<std::pair<std::string, std::string>> ret;
        ret.emplace_back(">0001#", ">0010#");
        ret.emplace_back(">00010#", ">00011#");

        return ret;
    }
};

class PalindromeTest : public ::unittest::UnitTest
{
private:
    const char *name() override
    {
        return "PalindromeTest";
    }

    void init() override
    {
        // TODO
    }

    std::vector<std::pair<std::string, std::string>> getInOuts() override
    {
        std::vector<std::pair<std::string, std::string>> ret;
        // TODO

        return ret;
    }
};

// TODO
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

    std::vector<std::pair<std::string, std::string>> getInOuts() override
    {
        std::vector<std::pair<std::string, std::string>> ret;
        // TODO

        return ret;
    }
};

// TODO
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

    std::vector<std::pair<std::string, std::string>> getInOuts() override
    {
        std::vector<std::pair<std::string, std::string>> ret;
        // TODO

        return ret;
    }
};

// TODO
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

    std::vector<std::pair<std::string, std::string>> getInOuts() override
    {
        std::vector<std::pair<std::string, std::string>> ret;
        // TODO

        return ret;
    }
};


int main()
{
    unittest::UnitTest *test1 = new IncrementTest();
    unittest::UnitTest *test2 = new PalindromeTest();
    unittest::UnitTest *test3 = new MatrixTest();
    unittest::UnitTest *test4 = new AnagramsTest();
    unittest::UnitTest *test5 = new CountZerosTest();

    test1->runTest();
    test2->runTest();
    test3->runTest();
    test4->runTest();
    test5->runTest();

    return 0;
}
