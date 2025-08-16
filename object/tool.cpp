#include "tool.h"

Tool::Tool(QObject *parent)
    : RobotInstance(parent)  //调用父类的构造函数
{


}

Tool :: ~Tool()
{

}

QString Tool::getBaseName() const
{
     // 如果工具的配置文件被加载了，并且里面有名字，就用那个名字
    if(m_config && m_config->isLoad()){
        return m_config->name();
    }

    // 否则，返回一个通用的 "Tool" 作为基础名称
    return "tool";

}
