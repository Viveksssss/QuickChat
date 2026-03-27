cmake_minimum_required(VERSION 3.19)
project(CppChat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)
find_package(Qt6 REQUIRED COMPONENTS MultimediaWidgets)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Protobuf REQUIRED)
find_package(absl REQUIRED CONFIG)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${PROTO_SRC_DIR})          # 生成文件也放这里

# 编译proto
add_custom_command(
    OUTPUT
        ${PROTO_OUT_DIR}/im.pb.cc
        ${PROTO_OUT_DIR}/im.pb.h

    COMMAND
        protobuf::protoc

    ARGS
        --cpp_out=${PROTO_OUT_DIR}
        -I${PROTO_SRC_DIR}
        ${PROTO_SRC_DIR}/im.proto

    DEPENDS
        ${PROTO_SRC_DIR}/im.proto
    COMMENT
        "Running protoc on im.proto"
    VERBATIM
)


qt_standard_project_setup()

# 这里资源文件qrc需要手动的显示添加:qt_add_resources,否则默认找不到
qt_add_resources(QT_RESOURCES "res.qrc")

qt_add_executable(CppChat
    WIN32 MACOSX_BUNDLE
    main.cpp
    ${PROTO_OUT_DIR}/im.pb.cc
    ${QT_RESOURCES}
    mainwindow.h mainwindow.cpp
    stylemanager.h stylemanager.cpp
    LoginInterface/forgotscreen.cpp LoginInterface/forgotscreen.h LoginInterface/loginscreen.cpp LoginInterface/loginscreen.h LoginInterface/registerscreen.cpp LoginInterface/registerscreen.h
    Properties/singleton.h
    Properties/global.h
    Properties/global.cpp
    Properties/sourcemanager.h Properties/sourcemanager.cpp
    httpmanager.h httpmanager.cpp
    tcpmanager.h tcpmanager.cpp
    config.ini
    usermanager.h usermanager.cpp
    MainInterface/mainscreen.h MainInterface/mainscreen.cpp
    MainInterface/toptitlepart.h MainInterface/toptitlepart.cpp
    MainInterface/sidebarpart.h MainInterface/sidebarpart.cpp
    MainInterface/Chat/chatpart.h MainInterface/Chat/chatpart.cpp
    MainInterface/Chat/ChatTopArea/chattoparea.h MainInterface/Chat/ChatTopArea/chattoparea.cpp
    MainInterface/Chat/ChatArea/chatarea.h MainInterface/Chat/ChatArea/chatarea.cpp
    MainInterface/Chat/ChatArea/InputArea/inputarea.h MainInterface/Chat/ChatArea/InputArea/inputarea.cpp
    MainInterface/Chat/ChatArea/MessageArea//messagearea.h MainInterface/Chat/ChatArea/MessageArea//messagearea.cpp
    MainInterface/Chat/ChatArea/MessageArea/messagetypes.h
    MainInterface/Chat/ChatArea/MessageArea/messagemodel.h MainInterface/Chat/ChatArea/MessageArea/messagemodel.cpp
    MainInterface/Chat/ChatArea/MessageArea/messagedelegate.h MainInterface/Chat/ChatArea/MessageArea/messagedelegate.cpp
    MainInterface/Chat/ChatTopArea/SideNews/friendsnewsitem.h MainInterface/Chat/ChatTopArea/SideNews/friendsnewsitem.cpp
    MainInterface/Chat/ChatTopArea/SideNews/systemnewsitem.h MainInterface/Chat/ChatTopArea/SideNews/systemnewsitem.cpp
    MainInterface/Chat/ChatTopArea/SideNews/notificationpanel.h MainInterface/Chat/ChatTopArea/SideNews/notificationpanel.cpp
    Properties/signalrouter.h Properties/signalrouter.cpp
    proto/im.proto
    proto/im.pb.cc
    proto/im.pb.h
    proto/im.pb.cc proto/im.pb.h
    MainInterface/List/FriendsList/friendsmodel.h MainInterface/List/FriendsList/friendsmodel.cpp MainInterface/List/FriendsList/friendslistpart.h MainInterface/List/FriendsList/friendslistpart.cpp MainInterface/List/FriendsList/frienditemdelegate.h MainInterface/List/FriendsList/frienditemdelegate.cpp MainInterface/List/FriendsList/frienditem.h
    MainInterface/List/MessagesList/messageitemdelegate.cpp MainInterface/List/MessagesList/messageitemdelegate.h MainInterface/List/MessagesList/messageslistpart.cpp MainInterface/List/MessagesList/messageslistpart.h MainInterface/List/MessagesList/messagesmodel.cpp MainInterface/List/MessagesList/messagesmodel.h
    MainInterface/List/listpart.h MainInterface/List/listpart.cpp
    database.h database.cpp
    Properties/imagecropperdialog.h Properties/imagecropperdialog.cpp
)


configure_file(${CMAKE_SOURCE_DIR}/config.ini
               ${CMAKE_BINARY_DIR}/config.ini COPYONLY)


# 将静态资源文件复制到运行时的路径下
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Statics
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)


# set(RESOURCE_FILES
#     Statics/images/icon.png
#     Statics/styles/style.qss
#     Statics/translations/app_zh_CN.qm
# )

# # 复制资源到构建目录
# foreach(resource_file ${RESOURCE_FILES})
#     configure_file(
#         ${resource_file}
#         ${CMAKE_CURRENT_BINARY_DIR}/${resource_file}
#         COPYONLY
#     )
# endforeach()

target_link_libraries(CppChat
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        protobuf::libprotobuf
        absl::log_internal_check_op
        absl::strings
        absl::base
        absl::synchronization
        absl::time
)
target_link_libraries(CppChat PRIVATE Qt6::Widgets Qt6::Sql)
target_link_libraries(CppChat PRIVATE Qt6::Core)

target_include_directories(CppChat PRIVATE ${ASIO_INCLUDE_DIR})
target_include_directories(CppChat PRIVATE ${PROTO_OUT_DIR})
include(GNUInstallDirs)

install(TARGETS CppChat
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET CppChat
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
