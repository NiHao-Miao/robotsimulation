#include "luathread.h"
#include <list>
#include <mutex>
#include <string>
#include <thread>

#include "qdebug.h"
#include <QElapsedTimer>

// 注册C函数到lua脚本中
#define REGISTER_C_FUNCTION_TO_LUA(Lua, Fun, Decr, Help)                       \
    do {                                                                       \
        if (Lua) {                                                             \
            lua_pushcfunction(Lua, Fun);                                       \
            lua_setglobal(Lua, #Fun);                                          \
        }                                                                      \
        function_list.insert(#Fun, Decr);                                      \
        function_help_list.insert(#Fun, Help);                                 \
    } while (0);

QString LuaThread::formatHHMMSS(qint32 ms) {
    int secInt = 1000;
    int minInt = secInt * 60;
    int houInt = minInt * 60;
    int dayInt = houInt * 24;

    long day = ms / dayInt;
    long hour = (ms - day * dayInt) / houInt;
    long minute = (ms - day * dayInt - hour * houInt) / minInt;
    long second =
        (ms - day * dayInt - hour * houInt - minute * minInt) / secInt;
    long milliSecond =
        ms - day * dayInt - hour * houInt - minute * minInt - second * secInt;

    QString dayQStr = QString::number(day, 10);
    QString houQStr = QString::number(hour, 10);
    QString minQStr = QString::number(minute, 10);
    QString secQStr = QString::number(second, 10);
    QString msecQStr = QString::number(milliSecond, 10);

    // qDebug() << "minute:" << min << "second" << sec << "ms" << msec <<endl;

    return dayQStr + "天 " + houQStr + "小时 " + minQStr + "分钟 " + secQStr +
           "秒 " + msecQStr;
}


///
///     https://vimsky.com/examples/detail/cpp-ex-----lua_sethook-function.html
///     https://www.wenjiangs.com/doc/lua_sethook
/// \brief lstop
/// \param L
/// \param ar
///
void lstop(lua_State *l, lua_Debug *ar) {
//    qDebug() << "ar->event" << ar->event;
    if (ar->event == LUA_HOOKCOUNT) {
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < 1)
            ;
    } else if (ar->event == LUA_MASKCALL) {
        lua_getinfo(l, "nSlu", ar);
//        qDebug() << "ar name" << ar->name << ar->namewhat << ar->source;
    }
//    if (((DeviceControl *)lua_getuserdata(l))->lua_thread->lua_stop) {
//        lua_sethook(l, NULL, 0, 0);
//        luaL_error(l, "中断退出!");
//    }
}

int setLuaPath(lua_State *L, const char *path) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1,
                 "path"); // get field "path" from table at top of stack (-1)
    std::string cur_path =
        lua_tostring(L, -1); // grab path string from top of stack
    cur_path.append(";");    // do your path magic here
    cur_path.append(path);
    lua_pop(L,
            1); // get rid of the string on the stack we just pushed on line 5
    lua_pushstring(L, cur_path.c_str()); // push the new one
    lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with
                                 // value at top of stack
    lua_pop(L, 1);               // get rid of package table from top of stack
    return 0;                    // all done!
}

void luaPushArray(lua_State *l, int len, uint16_t *msg) {
    lua_newtable(l); //新建一个表，并压入栈顶
    for (int n = 0; n < len; n++) {
        lua_pushinteger(l, msg[n]); //将数组的数据入栈
        lua_rawseti(
            l, -2,
            n + 1); //将刚刚入栈的数据设置为数组的第n+1个数据，同时这个数据会自动从栈里pop
    }
}

void luaPushDoubleArray(lua_State *l, int len, const QVector<double>& data) {
    lua_newtable(l); //新建一个表，并压入栈顶
    for (int n = 0; n < len; n++) {
        lua_pushnumber(l, data[n]); //将数组的数据入栈
        lua_rawseti(
            l, -2,
            n + 1); //将刚刚入栈的数据设置为数组的第n+1个数据，同时这个数据会自动从栈里pop
    }
}


///
/// \brief luaGetArrayParameter
/// \param l
/// \param arrLen 数组长度未知，需要从lua调用时的参数中获得
/// \param luaArr
/// \return
///
int luaGetArrayParameter(lua_State *l, int *arrLen, int *luaArr) {
    if (!lua_istable(l, 1)) { //判断第一个参数是否为表，否则退出
        return -1;
    }
    *arrLen = luaL_checknumber(l, 2); //获取第二个参数，数据长度
    //    uint8_t luaArr[*arrLen];
    for (int i = 1; i <= *arrLen; i++) {
        lua_rawgeti(l, 1, i);
        luaArr[i - 1] = lua_tointeger(l, -1);
        lua_pop(l, 1); //把上一个内容的出栈
    }
    return 0;
}

int luaGetArrayParameter(lua_State *l, int index, int arrLen, int16_t *luaArr) {
    if (!lua_istable(l, index)) { //判断第index个参数是否为表，否则退出
        return -1;
    }
    for (int i = 1; i <= arrLen; i++) {
        lua_rawgeti(l, index, i);
        luaArr[i - 1] = lua_tointeger(l, -1);
        lua_pop(l, 1); //把上一个内容的出栈
    }
    return 0;
}

///
/// \brief luaGetArrayParameter 获取int数组
/// \param l
/// \param index
/// \param arrLen
/// \param luaArr
/// \return
///
int luaGetArrayParameter(lua_State *l, int index, int arrLen, int *luaArr) {
    if (!lua_istable(l, index)) { //判断第index个参数是否为表，否则退出
        return -1;
    }
    for (int i = 1; i <= arrLen; i++) {
        lua_rawgeti(l, index, i);
        luaArr[i - 1] = lua_tointeger(l, -1);
        lua_pop(l, 1); //把上一个内容的出栈
    }
    return 0;
}

///
/// \brief luaGetArrayParameter 获取uint数组
/// \param l
/// \param index
/// \param arrLen
/// \param luaArr
/// \return
///
int luaGetArrayParameter(lua_State *l, int index, int arrLen,
                         uint32_t *luaArr) {
    if (!lua_istable(l, index)) { //判断第index个参数是否为表，否则退出
        return -1;
    }
    for (int i = 1; i <= arrLen; i++) {
        lua_rawgeti(l, index, i);
        luaArr[i - 1] = lua_tointeger(l, -1);
        lua_pop(l, 1); //把上一个内容的出栈
    }
    return 0;
}

///
/// \brief luaGetArrayParameter  获取double数组
/// \param l
/// \param index
/// \param arrLen
/// \param luaArr
/// \return
///
int luaGetArrayParameter(lua_State *l, int index, int arrLen, QVector<double>* luaArr) {
    if (!lua_istable(l, index)) { //判断第index个参数是否为表，否则退出
        return -1;
    }
    for (int i = 1; i <= arrLen; i++) {
        lua_rawgeti(l, index, i);
        luaArr->append(lua_tonumber(l, -1));
        lua_pop(l, 1); //把上一个内容的出栈
    }
    return 0;
}



LuaThread::LuaThread(QObject* parent) {

}

LuaThread::~LuaThread() { destroyLua(); }

QString LuaThread::startLua(const QString &lua_file) {
    if (L != nullptr){
        destroyLua();
    }
    L = luaL_newstate();
    if (L == nullptr) {
        return "创建Lua错误!";
    }
//    lua_setuserdata(L, device_control);

    // 加载相关库文件
    luaL_openlibs(L);

    initLuaFunction();
    int bRet = luaL_dostring(L, lua_file.toUtf8().data());
    if (bRet) {
        std::string str = lua_tostring(L, -1);
        lua_pop(L, 1);
        str += "\n"; //< 错误信息
        qDebug() <<"Lua文件加载失败";
        qDebug() << QString::fromStdString(str);
        emit runError(QString::fromStdString(str));
        destroyLua();
        return "Lua文件加载失败:"+ QString::fromStdString(str);
    }

    setLuaPath(
        L, QString(script_info.absolutePath() + "/?.lua").toStdString().data());
    setLuaPath(L, "./lualib/?.lua");

    // LUA_MASKCALL:调用一个函数时，就调用一次钩子函数。
    // LUA_MASKRET:从一个函数中返回时，就调用一次钩子函数。
    // LUA_MASKLINE:执行一行指令时，就回调一次钩子函数。
    // LUA_MASKCOUNT:执行指定数量的指令时，就回调一次钩子函数。
//    lua_sethook(L, lstop,
//                LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT, 1);
//    start();
    return "";
}



void LuaThread::initLuaFunction() {

}

void LuaThread::destroyLua() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

void LuaThread::run() {
    exec_time = QTime::currentTime();
    int bRet = lua_pcall(L, 0, 0, 0);
    if (bRet) {
        std::string str = lua_tostring(L, -1);
        lua_pop(L, 1);
        destroyLua();
        qDebug("call main.lua file failed \n");
        qDebug() << QString::fromStdString(str);
        emit runError(QString::fromStdString(str));
        return;
    }
    destroyLua();
    msleep(10);
    emit runFinish();
}

int LuaThread::runTime() { return exec_time.msecsTo(QTime::currentTime()); }

bool LuaThread::isRunning() { return L; }

bool LuaThread::updateJoints(const QVector<double> & joints,QVector<double>* joints_out)
{
    if (L == nullptr)
        return false;
    lua_getglobal(L, "hc_updateJoints");
    if (!lua_isfunction(L, -1)) {
        qDebug() << "Lua function not found: ";
        destroyLua();
        return false;
    }
    luaPushDoubleArray(L,joints.size(),joints);

    // 调用Lua函数
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        qDebug() << "Error calling Lua function: " << lua_tostring(L, -1);
        destroyLua();
        return false;
    }
    int ret = luaGetArrayParameter(L,-1,joints.size(),joints_out);
    return true;
}

bool LuaThread::updateBase(const QVector<double> & joints,QVector<double>* joints_out)
{
    if (L == nullptr)
        return false;
    lua_getglobal(L, "hc_updateBaseJoints");
    if (!lua_isfunction(L, -1)) {
        qDebug() << "Lua function not found: ";
        destroyLua();
        return false;
    }
    luaPushDoubleArray(L,joints.size(),joints);

    // 调用Lua函数
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        qDebug() << "Error calling Lua function: " << lua_tostring(L, -1);
        destroyLua();
        return false;
    }
    int ret = luaGetArrayParameter(L,-1,6,joints_out);
    return true;
}
