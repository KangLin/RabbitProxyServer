# Author: Kang Lin <kl222@126.com>

cmake_minimum_required(VERSION 2.8.12)

project(RabbitProxyServerConfigure)

SET(QT_COMPONENTS Gui Widgets)
if(ANDROID)
    SET(QT_COMPONENTS ${QT_COMPONENTS} AndroidExtras)
endif()
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_COMPONENTS})
message("QT_VERSION:${Qt${QT_VERSION_MAJOR}_VERSION}")
if(Qt${QT_VERSION_MAJOR}_VERSION VERSION_LESS 5.10.0 AND ANDROID)
    message(FATAL_ERROR "Qt must great 5.10.0")
endif()
if(Qt${QT_VERSION_MAJOR}_FOUND)
    FOREACH(_COMPONENT ${QT_COMPONENTS})
        SET(QT_LIBRARIES ${QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${_COMPONENT})
    ENDFOREACH()
endif()

SET(SOURCE_FILES
    mainwindow.cpp
    FrmSocket.cpp)

set(HEADER_FILES
    mainwindow.h
    FrmSocket.h)

set(SOURCE_UI_FILES
    mainwindow.ui
    FrmSocket.ui
    )

#翻译
#GENERATED_QT_TRANSLATIONS(
#    NAME ${PROJECT_NAME}Configure
#    SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${SOURCE_UI_FILES} main_config.cpp main.cpp
#    OUT_QRC TRANSLATIONS_QRC_FILES
#    )
#string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
#if(BUILD_TYPE STREQUAL "debug")
#    LIST(APPEND PRIVATE_DEFINITIONS DEBUG)
#    LIST(APPEND RCC_FILES
#        ${TRANSLATIONS_QRC_FILES}
#        )
#endif()
LIST(APPEND RCC_FILES Resource/Resource.qrc)
if(WIN32)
    LIST(APPEND RCC_FILES Resource/App.rc)
endif()

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

list(APPEND SERVICE_LIBS RabbitProxy ${QT_LIBRARIES})

ADD_TARGET(NAME ${PROJECT_NAME}
    ISEXE
    ISWINDOWS
    VERSION ${BUILD_VERSION}
    PRIVATE_DEFINITIONS BUILD_VERSION="${BUILD_VERSION}" ${SERVICE_DEFINITIONS}
    SOURCE_FILES main_config.cpp ${SOURCE_FILES} ${HEADER_FILES} ${SOURCE_UI_FILES} ${RCC_FILES}
    PRIVATE_LIBS ${SERVICE_LIBS})

find_package(QtService)
if(QtService_FOUND)
    SET(SOURCE_FILES Service.cpp)
    SET(HEADER_FILES Service.h)
    list(APPEND SERVICE_LIBS QtService)
    INSTALL_TARGETS(TARGETS QtService)

    ADD_TARGET(NAME RabbitProxyServer
        ISEXE
        ISWINDOWS
        NO_TRANSLATION
        VERSION ${BUILD_VERSION}
        PRIVATE_DEFINITIONS BUILD_VERSION="${BUILD_VERSION}" ${SERVICE_DEFINITIONS}
        SOURCE_FILES main.cpp ${SOURCE_FILES} ${HEADER_FILES} ${SOURCE_UI_FILES} ${RCC_FILES}
        PRIVATE_LIBS ${SERVICE_LIBS})
endif()

if(WIN32)
    install(FILES Resource/Image/App.ico
        DESTINATION .
            COMPONENT Runtime)
endif()
if(UNIX AND NOT ANDROID)
    INSTALL(FILES ${CMAKE_SOURCE_DIR}/debian/RabbitProxyServerConfig.desktop
        DESTINATION share/applications
            COMPONENT Runtime)
    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resource/Image/App.png
        DESTINATION share/pixmaps
            COMPONENT Runtime
        RENAME RabbitProxyServer.png)
endif()

# Install other files
set(OTHER_FILES
    ${CMAKE_SOURCE_DIR}/License.md
    ${CMAKE_SOURCE_DIR}/Authors.md
    ${CMAKE_SOURCE_DIR}/ChangeLog.md
    ${CMAKE_SOURCE_DIR}/Authors_zh_CN.md
    ${CMAKE_SOURCE_DIR}/ChangeLog_zh_CN.md)
INSTALL_FILE(SOURCES ${OTHER_FILES}
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT Runtime)
