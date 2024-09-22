#include <iostream>
#include <tuple>
#include <format>
#include <optional>



class CommandAPI {
public:
    virtual bool execute(std::string_view arg_str) const = 0;
    virtual const CommandAPI *nameMatch(std::string_view arg_str) const = 0;
};

template<class ...C>
class Command : public CommandAPI {
    std::tuple<C...> arguments{};
    std::string_view name = "dummy";
    bool (*cmd_logic)(C...);

public:
    template<class Func>
    consteval
    Command(std::string_view cmd_name, Func logic) : name { cmd_name }, cmd_logic { logic } {}

    bool execute(std::string_view arg_str) const override {
        std::cout << std::format("executing cmd '{}' with raw argument string '{}'\n", name, arg_str);
        std::cout << std::format("TODO: process text string and tokenize based on argument information\n");
        return std::apply(cmd_logic, arguments);
    }
    const CommandAPI *nameMatch(std::string_view cmd_name) const override {
        std::cout << std::format("my name '{}' checked against '{}'\n", name, cmd_name);
        return name == cmd_name ? this : nullptr;
    }
    std::string_view getName() const {
        return name;
    }
};

class GroupAPI {
public:
    virtual const CommandAPI* findCommand(std::string_view cmd_name) const = 0;
};

template<class ...C>
class CommandGroup : public GroupAPI {
    std::tuple<C...> command_list{};
    std::string_view name = "group";

public:
    consteval CommandGroup(std::string_view group_name, const C &... cmd) : command_list { cmd... },
                                                                      name { group_name } {}
    consteval size_t cmdCount() const {
        return std::tuple_size<decltype(command_list)>();
    }

    virtual const CommandAPI* findCommand(std::string_view cmd_name) const {
        std::cout << std::format("searching for command with name '{}'\n", cmd_name);
        return std::apply([&](auto&&... cmd_list) {
            const CommandAPI *cmd = nullptr;
            ((!cmd && (cmd = cmd_list.nameMatch(cmd_name))), ...);
            return cmd;
            }, command_list);
    }
};



// USAGE EXAMPLES:

constexpr Command<int, bool, float> cmd1{"cmd1", [](int a, bool b, float c) {
    std::cout << std::format("cmd:1 logic executed: {}, {}, {}\n", a, b, c);
    return true;
}};

constexpr Command<int> cmd2{"cmd2", [](int a) {
    std::cout << std::format("cmd:2 logic executed: {}\n", a);
    if (a > 10) {
        return false;
    }
    return true;
}};

constexpr CommandGroup main_cmd_group("main", cmd1, cmd2, Command<int>{"cmd3", [](int a){
    std::cout << std::format("cmd:3 logic executed: {}\n", a);
    return true;
}});

int main(int argc, char **args) {
    std::string_view test_cmd = "cmd1";
    std::string_view test_args = "1,2,3";

    if (argc >= 2) {
        test_cmd = args[1];
    }
    if (argc >= 3) {
        test_args = args[2];
    }

    std::cout << std::format("experimental...\n");
    std::cout << std::format("cmd count: {}\n", main_cmd_group.cmdCount());

    const GroupAPI &gr = main_cmd_group;

    const auto matching = gr.findCommand(test_cmd);
    if (matching) {
        auto result = matching->execute(test_args);
        std::cout << std::format("command returned {}\n", result);
    }
    else {
        std::cout << std::format("command not found\n");
    }

    return 0;
}
