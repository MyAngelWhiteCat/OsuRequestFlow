#include <iostream>

#include "test_message_processor.h"
#include "../../src/logging.h"



int main() {
    logging::Logger::Init();
    std::cout << "Run test..." << std::endl;
    test_message_processor::RunTests();
}