cmake_minimum_required(VERSION 3.22)
project(persistent_touch_id_sudo C)

set(CMAKE_C_STANDARD 17)

add_executable(persistent-touch-id-sudo src/main.c)

install(TARGETS persistent-touch-id-sudo DESTINATION /usr/local/bin)
install(FILES assets/com.yuriyguts.persistent-touch-id-sudo.plist DESTINATION /Library/LaunchDaemons)
