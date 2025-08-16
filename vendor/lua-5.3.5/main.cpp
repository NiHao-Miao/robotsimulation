#include <QCoreApplication>
#include <iostream>
#include <stdio.h>
#include <lua.hpp>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printf("I am QT5.7\n");
    lua_State *L;
    L = luaL_newstate();
    if(L == NULL){
        printf("luaL_newstate failed\n");
        return a.exec();
    }
    /*load Lua base libraries*/
    luaL_openlibs(L);

    if(luaL_dofile(L,"bin/cal.lua")){
            const char *error = lua_tostring(L, -1);
            std::cout << error <<std::endl;
            return false;
       }

    lua_close(L);
    printf("luaL_newstate OK\n");
    return a.exec();
}
