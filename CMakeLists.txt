# to compile the plugin out of
# kopete source tree
if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
  find_package(KDE4 REQUIRED)
  find_package(Qt4 REQUIRED)
  INCLUDE_DIRECTORIES(${QT4_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${QT_QTCORE_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${QT_QTGUI_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${QT_QTNETWORK_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${KDE4_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${KDE4_INCLUDES})

  include (KDE4Defaults)
  include (MacroLibrary)

  find_path(KOPETE_INCLUDE_DIR
    NAME kopeteversion.h
    PATH_SUFFIXES kopete
    HINTS
    ${INCLUDE_INSTALL_DIR}
    ${KDE4_INCLUDE_DIR}
  )

  if(KOPETE_INCLUDE_DIR)
    MESSAGE("Found Kopete: includes - ${KOPETE_INCLUDE_DIR}")
  else(KOPETE_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "Kopete headers not found")
  endif(KOPETE_INCLUDE_DIR)

  mark_as_advanced(KOPETE_INCLUDE_DIR)
endif(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(QJson REQUIRED)

add_subdirectory( icons )

include_directories( 
${KOPETE_INCLUDE_DIR}
${KOPETE_INCLUDE_DIR}/ui 
${CMAKE_CURRENT_SOURCE_DIR}/ui/
${QJSON_INCLUDE_DIR}
)

set(facebook_SRCS
   facebook/chatservice.cpp
   facebook/buddyinfo.cpp
   facebook/chatmessage.cpp
)

set(kopete_facebook_PART_SRCS 
   facebookprotocol.cpp 
   facebookcontact.cpp 
   facebookaccount.cpp 
   facebookchatsession.cpp 
   facebookaddcontactpage.cpp 
   facebookeditaccountwidget.cpp 
   ${facebook_SRCS}
)

kde4_add_ui_files(kopete_facebook_PART_SRCS
  facebookaddui.ui
)

kde4_add_ui_files(kopete_facebook_PART_SRCS
  facebookaccountpreferences.ui
)

kde4_add_plugin(kopete_facebook ${kopete_facebook_PART_SRCS})

#QT4_AUTOMOC(${facebook_SRCS})
kde4_add_executable(facebooktest test.cpp ${facebook_SRCS})
target_link_libraries(facebooktest kopete ${QJSON_LIBRARIES})

if(WIN32)
  target_link_libraries(kopete_facebook  ${KDE4_KIO_LIBS} kopete ${QJSON_LIBRARIES})
else(WIN32)
  target_link_libraries(kopete_facebook  ${KDE4_KIO_LIBS} kopete kopete_videodevice ${QJSON_LIBRARIES})
endif(WIN32)

install(TARGETS kopete_facebook  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_facebook.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
