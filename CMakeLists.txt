cmake_minimum_required(VERSION 3.29)
project(Badland_ING1_sem2 C)

set(CMAKE_C_STANDARD 99)

add_executable(Badland_ING1_sem2
    main.c
        game.h
        player.h
        menu.h
        levels.h
        save.h
        resources.h)
target_link_libraries(Badland_ING1_sem2 -lmingw32 -lalleg44)
