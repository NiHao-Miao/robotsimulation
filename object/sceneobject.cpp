#include "sceneobject.h"
#include <AIS_InteractiveContext.hxx>
#include "robotinstance.h"

SceneObject::SceneObject(QObject* parent)
    : QObject(parent)
{
}

// 虚析构函数的实现体可以是空的
SceneObject::~SceneObject()
{
}

void SceneObject::removeFromScene(const Handle(AIS_InteractiveContext)& context)
{
    if (!context.IsNull() && !m_aisObject.IsNull() && context->IsDisplayed(m_aisObject)) {
        context->Remove(m_aisObject, Standard_True);
    }
}
