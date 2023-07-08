cmake_minimum_required(VERSION 3.15)

project(ECE141B)

set(CMAKE_CXX_STANDARD 17)

add_executable(main main.cpp Application.cpp Attribute.cpp BlockIO.cpp ChangeStatement.cpp CmdProcessor.cpp Database.cpp
        DbProcessor.cpp DbStatement.cpp Entity.cpp Faked.cpp Filters.cpp Query.cpp Row.cpp Statement.cpp
        SelectStatement.cpp SQLProcessor.cpp SQLStatement.cpp Storage.cpp Tokenizer.cpp)