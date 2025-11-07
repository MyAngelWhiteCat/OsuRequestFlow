#pragma once 

#include "../src/message_processor.h"
#include "../src/message.h"
#include "utils.h"


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
            utilities::PrintTwoVectorsInRow(std::cout, expected_output, output, "Expected:", "Fact:");
        }
    }

    void TestPing(Processor& proc) {
        std::string ping = "PING";
        std::string ping_original = " :tmi.twitch.tv";
        std::string ping_weird = " 1 qw 32-=0 ,.,. === iiii 79qvhl8";
        std::string message = ping + ping_original + "\r\n" + ping + ping_weird + "\r\n";
        Message ping0(irc::domain::MessageType::PING, std::move(ping_original));
        Message ping1(irc::domain::MessageType::PING, std::move(ping_weird));
        std::vector<Message> expected = { ping0, ping1};
        TestExpected("Ping", proc, message, expected);
    }

    void RunTests() {
        Processor processor;
        TestPing(processor);
    }

}