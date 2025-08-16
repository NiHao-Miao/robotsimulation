#ifndef TOOL_H
#define TOOL_H

#include "robotinstance.h"

/**
 * @brief 代表一个可装配的工具或末端执行器。
 *
 * Tool 类继承自 RobotInstance，这意味着一个工具在本质上
 * 就是一个迷你的、通常是被动的机器人。它可以拥有自己的
 * .hcrobot 配置文件、几何模型、关节链和运动学。
 * 这种设计最大化地复用了现有代码和架构。
 */
class Tool : public RobotInstance
{
    Q_OBJECT

public:
    explicit Tool(QObject *parent = nullptr);
    virtual ~Tool();

    // 我们需要重写 getBaseName() 以提供一个不同的默认名称
    virtual QString getBaseName() const override;

private:
    // 可以在这里添加工具专属的成员变量
};



#endif // TOOL_H
