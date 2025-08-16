-- 请在内部完善函数体，不要修改函数名和参数，需要返回一个浮点数组

-- 更新关节坐标
function hc_updateJoints(joints_in)
-- 请在此处写
-- 定义一个数组,输出关节坐标
local joints_out = {}

-- 给数组赋值
joints_out[1] = joints_in[1]
joints_out[2] = joints_in[2]
joints_out[3] = joints_in[3]
joints_out[4] = joints_in[4]
joints_out[5] = joints_in[5]
joints_out[6] = joints_in[6]

return joints_out
end

-- 更新基座坐标
function hc_updateBaseJoints(joints_in)
-- 请在此处写
-- 定义一个数组,输出基座坐标 长度为6
local joints_out = {}

-- 给数组赋值
joints_out[1] = joints_in[7]
joints_out[2] = 0
joints_out[3] = 0
joints_out[4] = 0
joints_out[5] = 0
joints_out[6] = 0

return joints_out
end
