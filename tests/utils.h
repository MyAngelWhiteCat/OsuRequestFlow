#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

// unreadable quik code on
// no time for test or hight perfomance readable code. Let it just work. AI will refact this later
namespace test_message_processor {

    namespace utilities {

        const std::string RESET = "\033[0m";
        const std::string RED = "\033[31m";
        const std::string GREEN = "\033[32m";

        std::string GetStringWithOffset(std::string_view lhs, std::string_view rhs, size_t max_elem_size) {
            size_t need_spaces = (max_elem_size - lhs.size()) + 1;
            std::string offset(need_spaces, ' ');
            std::string result = std::string(lhs).append(offset).append(std::string(rhs));
            return result;
        }

        std::string AddSpacesOffset(std::string_view str, size_t subtruct, size_t max_elem_size) {
            size_t need_spaces = (max_elem_size - subtruct) + 1;
            std::string offset(need_spaces, ' ');
            std::string result = offset.append(std::string(str));
            return result;
        }

        std::string ConcatinateStringVectors(const std::vector<std::string>& lhs
            , const std::vector<std::string>& rhs, size_t max_elem_size
            , std::string_view left_name = "", std::string_view right_name = "") {

            max_elem_size = std::max({ max_elem_size, left_name.size(), right_name.size() });

            size_t max_size = std::max(lhs.size(), rhs.size());
            std::string result = GetStringWithOffset(left_name, right_name, max_elem_size).append("\n");
            for (int i = 0; i < max_size; ++i) {
                if (i < lhs.size()) {
                    result += lhs[i];
                }
                if (i < rhs.size()) {
                    result += AddSpacesOffset(rhs[i], i >= lhs.size() ? 0 : lhs[i].size(), max_elem_size);
                }
                result += '\n';
            }
            return result;
        }

        template <typename T>
        std::string GetElements(const std::vector<T>& lhs, const std::vector<T>& rhs
            , std::string_view left_name = "", std::string_view right_name = "") {
            size_t max_size = std::max(lhs.size(), rhs.size());
            std::vector<std::string> left;
            std::vector<std::string> right;

            size_t max_elem_size = 0;

            for (int i = 0; i < max_size; ++i) {
                std::ostringstream lhs_strm{};
                std::ostringstream rhs_strm{};
                std::string lhs_str;
                std::string rhs_str;
                std::string edited_left_str;

                if (i < lhs.size()) {
                    lhs_strm << lhs[i];
                    lhs_str = lhs_strm.str();
                    std::string edited_str;
                    for (char& ch : lhs_str) {
                        if (ch == '\n') {
                            edited_str += "\\n";
                        }
                        else if (ch == '\r') {
                            edited_str += "\\r";
                        }
                        else {
                            edited_str += ch;
                        }
                    }
                    edited_left_str = edited_str;
                    max_elem_size = std::max(lhs_str.size(), max_elem_size);
                }

                std::string colored_str;

                bool red_setted = false;
                bool green_setted = false;
                if (i < rhs.size()) {
                    rhs_strm << rhs[i];
                    rhs_str = rhs_strm.str();
                    for (int j = 0; j < rhs_str.size(); ++j) {
                        if (j < lhs_str.size()) {
                            if (j < rhs_str.size()) {
                                if (rhs_str[j] != lhs_str[j]) {
                                    if (!red_setted) {
                                        colored_str += RED;
                                        red_setted = true;
                                        green_setted = false;
                                    }
                                    if (rhs_str[j] == ' ') {
                                        colored_str += "[SPACE]";
                                    }
                                    else if (rhs_str[j] == '\r') {
                                        colored_str += "\\r";
                                    }
                                    else if (rhs_str[j] == '\n') {
                                        colored_str += "\\n";
                                    }
                                    else if (lhs_str[j] == '\r') {
                                        colored_str += "[\\r]";
                                    }
                                    else if (lhs_str[j] == '\n') {
                                        colored_str += "[\\n]";
                                    }
                                    else if (lhs_str[j] == ' ') {
                                        colored_str += "[SPACE_MISS]";
                                    }
                                    else {
                                        colored_str += rhs_str[j];
                                    }
                                }
                                else {
                                    if (!green_setted) {
                                        colored_str += GREEN;
                                        green_setted = true;
                                        red_setted = false;
                                    }
                                    colored_str += rhs_str[j] ;
                                }
                            }
                            else {
                                if (!red_setted) {
                                    colored_str += RED;
                                    red_setted = true;
                                    green_setted = false;
                                }
                                colored_str += lhs_str[j];
                            }
                        }
                        else {
                            if (!red_setted) {
                                colored_str += RED;
                                red_setted = true;
                                green_setted = false;
                            }
                            colored_str += rhs_str[j];
                        }
                    }
                }
                
                colored_str += RESET;
                left.push_back(edited_left_str);
                if (colored_str.empty() && !edited_left_str.empty()) {
                    colored_str += RED + "[EMPTY]" + RESET;
                }
                right.push_back(colored_str);
                max_elem_size = std::max(rhs_str.size(), max_elem_size);
            }
            return ConcatinateStringVectors(left, right, max_elem_size, left_name, right_name);
        }

        template <typename T>
        void PrintTwoVectorsInRow(std::ostream& out, const std::vector<T>& lhs, const std::vector<T>& rhs
            , std::string_view left_colomn_name, std::string_view right_colomn_name) {
            out << GetElements(lhs, rhs, left_colomn_name, right_colomn_name);
        }

        template <typename T>
        void PrintTwoVectorsInRow(std::ostream& out, const std::vector<T>& lhs, const std::vector<T>& rhs) {
            out << GetElements(lhs, rhs);
        }

    }

}
// quik code off