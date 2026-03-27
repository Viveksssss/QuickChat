cmake_minimum_required(VERSION 3.10.0)
project(ChatServer VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

cmake_policy(SET CMP0167 NEW)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED)
#
# find_packageckage(Boost 1.70 REQUIRED COMPONENTS filesystem system)

# pkg_check_modules(MYSQLPP REQUIRED mysql++)
pkg_check_modules(MYSQL REQUIRED REQUIRED IMPORTED_TARGET mariadb)
pkg_check_modules(SPEEDLOG REQUIRED REQUIRED IMPORTED_TARGET spdlog)
pkg_check_modules(REDIS REQUIRED REQUIRED IMPORTED_TARGET hiredis)
pkg_check_modules(JSON REQUIRED REQUIRED IMPORTED_TARGET nlohmann_json)
pkg_check_modules(GRPC REQUIRED REQUIRED IMPORTED_TARGET grpc)
pkg_check_modules(GRPCPP REQUIRED REQUIRED IMPORTED_TARGET grpc++)
pkg_check_modules(PROTOBUF REQUIRED IMPORTED_TARGET protobuf)
pkg_check_modules(ABSL REQUIRED IMPORTED_TARGET absl_strings)


add_executable(ChatServer
    main.cpp
    grpc/message.grpc.pb.cc
    grpc/message.pb.cc
    grpc/StatusClient.cpp
    global/ConfigManager.h
    global/const.cpp
    global/UserManager.cpp
    grpc/ChatGrpcClient.cpp
    grpc/ChatGrpcServer.cpp
    server/AsioPool.cpp
    server/Server.cpp
    server/LogicSystem.cpp
    session/Session.cpp
    session/MsgNode.cpp
    data/im.pb.cc
    redis/RedisManager.cpp
    mysql/MysqlManager.cpp
    mysql/MysqlDao.cpp
    redis/DistributedLock.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::headers)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::JSON)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GRPC)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GRPCPP)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::PROTOBUF)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::ABSL)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::REDIS)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::MYSQL)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::SPEEDLOG)
target_link_libraries(${PROJECT_NAME} PRIVATE ${MYSQLPP_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/libmysqlpp.so)
set(NO_WARNING_FILES
    grpc/message.grpc.cc
    grpc/message.grpc.h
    grpc/message.pb.h
    grpc/message.pb.cc
)

foreach(file ${NO_WARNING_FILES})
    set_source_files_properties(${file}
        COMPILE_FLAGS "-Wno-unused-variable -Wno-deprecated"
    )
endforeach()
