#pragma once 

#include "../src/message_processor.h"
#include "../src/message.h"


namespace test_message_processor {

    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";

    using Processor = irc::message_processor::MessageProcessor;
    using Message = irc::domain::Message;

    std::vector<char> MakeVectorFromString(std::string_view input) {
        std::vector<char> res;
        res.reserve(input.size());
        for (const char ch : input) {
            res.push_back(ch);
        }
        return res;
    }

    void TestExpected(std::string_view test_name, Processor& processor
        , std::string_view test_case, std::vector<irc::domain::Message> expected_output) {
        auto input = MakeVectorFromString(test_case);
        auto output = processor.ProcessMessage(input);

        std::cout << test_name;
        if (output == expected_output) {
            std::cout << GREEN << " PASS" << RESET << std::endl;
        }
        else {
            std::cout << RED << " FAIL." << RESET << std::endl;
        }
    }

    void TestPing(Processor& proc) {
        Message msg1(irc::domain::MessageType::PING, "PING");
        Message msg2(irc::domain::MessageType::PING, ":tmi.twitch.tv");
        std::vector<Message> expected{ msg1, msg2 };
        TestExpected("Ping", proc, "PING :tmi.twitch.tv", expected);
    }

    void RunTests() {
        Processor processor;
        TestPing(processor);
    }

}