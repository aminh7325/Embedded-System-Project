CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT("Reciever")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") 


include_directories(/usr/include/mysql) 
link_directories(/usr/lib/x86_64-linux-gnu)
set(APP_SOURCES src/Subscriber.c)
add_executable(${PROJECT_NAME} ${APP_SOURCES})
target_link_libraries(${PROJECT_NAME} -lpaho-mqtt3c -lmysqlclient -lpthread -lz -lm -lrt -latomic -lssl -lcrypto -ldl)

