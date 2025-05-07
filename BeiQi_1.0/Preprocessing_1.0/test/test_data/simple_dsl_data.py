"""
This code is a part of BJEV Test ToolChain
Created by ZhaoYi in 2022-02.
Copyright (c) 2022 BJEV. All rights reserved.
"""

simple_dsl_data = '''
def case0():
    前置条件(state == 12)
    执行(发送(BMS自检计数器 == 0, MCU初始化及自检标志位 == 1), 保持(state == 12), timeout=5.5, subject="")
    执行(发送(BMS自检计数器 == 1, MCU初始化及自检标志位 == 0), 保持(state == 12), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 2, MCU初始化及自检标志位 == 1), 保持(state == 12), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 1, MCU初始化及自检标志位 == 1), 预期(state == 14), timeout=5, subject="")
    执行(预期(state == 17), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 1), 保持(state == 17), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 3), 保持(state == 17), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 2), 预期(state == 20), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 2), 保持(state == 20), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 4), 保持(state == 20), timeout=5, subject="")
    执行(发送(BMS自检计数器 == 3), 预期(state == 23), timeout=5, subject="")
    执行(发送(MCU高压检测完成标志 == 0, 整车模式变量 == 2), 保持(state == 23), timeout=5, subject="")
    执行(发送(MCU高压检测完成标志 == 1, 整车模式变量 == 1), 保持(state == 23), timeout=5, subject="")
    执行(发送(MCU高压检测完成标志 == 1, 整车模式变量 == 3), 保持(state == 23), timeout=5, subject="")
    执行(发送(MCU高压检测完成标志 == 1, 整车模式变量 == 2), 预期(state == 30), timeout=5, subject="")
'''