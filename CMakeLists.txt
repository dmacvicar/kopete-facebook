add_subdirectory( icons )

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
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
target_link_libraries(facebooktest kopete qjson)

if(WIN32)
  target_link_libraries(kopete_facebook  ${KDE4_KIO_LIBS} kopete qjson)
else(WIN32)
  target_link_libraries(kopete_facebook  ${KDE4_KIO_LIBS} kopete kopete_videodevice qjson)
endif(WIN32)

install(TARGETS kopete_facebook  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_facebook.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
