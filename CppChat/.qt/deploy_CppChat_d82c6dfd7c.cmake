include("/mnt/S/home/vivek/Codes/Qt/CppChat/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/CppChat-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase;qtmultimedia;qtmultimedia")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "/mnt/S/home/vivek/Codes/Qt/CppChat/CppChat"
    GENERATE_QT_CONF
)
