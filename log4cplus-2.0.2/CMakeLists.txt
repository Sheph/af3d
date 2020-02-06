set(SOURCES
  appenderattachableimpl.cxx
  appender.cxx
  asyncappender.cxx
  callbackappender.cxx
  configurator.cxx
  connectorthread.cxx
  consoleappender.cxx
  cygwin-win32.cxx
  env.cxx
  factory.cxx
  fileappender.cxx
  fileinfo.cxx
  filter.cxx
  global-init.cxx
  hierarchy.cxx
  hierarchylocker.cxx
  layout.cxx
  log4judpappender.cxx
  lockfile.cxx
  logger.cxx
  loggerimpl.cxx
  loggingevent.cxx
  loggingmacros.cxx
  loglevel.cxx
  loglog.cxx
  mdc.cxx
  ndc.cxx
  nullappender.cxx
  objectregistry.cxx
  patternlayout.cxx
  pointer.cxx
  property.cxx
  queue.cxx
  rootlogger.cxx
  snprintf.cxx
  socketappender.cxx
  socketbuffer.cxx
  socket.cxx
  stringhelper.cxx
  stringhelper-clocale.cxx
  stringhelper-cxxlocale.cxx
  stringhelper-iconv.cxx
  syncprims.cxx
  syslogappender.cxx
  threads.cxx
  timehelper.cxx
  tls.cxx
  version.cxx
)

set(SOURCES ${SOURCES}
    socket-unix.cxx
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DINSIDE_LOG4CPLUS)

add_library(log4cplus STATIC ${SOURCES})

target_link_libraries(log4cplus ${CMAKE_THREAD_LIBS_INIT})
