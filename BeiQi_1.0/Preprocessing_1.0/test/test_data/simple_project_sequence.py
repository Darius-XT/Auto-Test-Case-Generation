import time
from .simple_new_signal_data import simple_signal_define

# 当图的id与stateid相同时，会优先使用图，这样会导致死循环

state30 = {
    'id': 'state12_state30',
    'desc': '',
    'type_name': 'graph',
    'h_function': 'none',
    'entry_action_list': [],
    'exit_action_list': [],
    'nodes': [
        {
            'type_name': 'state',
            'id': 'state12',
            'name': '新能源低压自检状态',
            'desc': '新能源低压自检',
            'is_init_node': True,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'state==12', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': [],
            'output_transitions': ['s12_s14']
        },
        {
            'type_name': 'state',
            'id': 'state14',
            'name': '电池高压负闭合状态',
            'desc': '电池高压负闭合',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'state==14', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['s12_s14'],
            'output_transitions': ['s14_s17']
        },
        {
            'type_name': 'state',
            'id': 'state17',
            'name': '电池高压检测状态',
            'desc': '电池高压检测',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'state==17', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['s14_s17'],
            'output_transitions': ['s17_s20']
        },
        {
            'type_name': 'state',
            'id': 'state20',
            'name': '高压系统预充电状态',
            'desc': '高压系统预充电',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'state==20', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['s17_s20'],
            'output_transitions': ['s20_s23']
        },
        {
            'type_name': 'state',
            'id': 'state23',
            'name': '高压系统检测状态',
            'desc': '高压系统检测',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'state==23', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['s20_s23'],
            'output_transitions': ['s23_s30']
        },
        {
            'type_name': 'state',
            'id': 'state30',
            'name': '行车状态',
            'desc': '行车',
            'is_init_node': False,
            'is_finish_node': True,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'state==30', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['s23_s30'],
            'output_transitions': []
        },
    ],
    'transitions': [
        {
            'type_name': 'transition',
            'id': 's12_s14',
            'name': '',
            'desc': '',
            'condition': 'BMS自检计数器==1 and (整车模式变量 == 2 or 整车模式变量 == 7 or 整车模式变量 == 9)',
            'event_list': [],
            'action_list': [],
            'source_node': 'state12',
            'target_node': 'state14',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 's14_s17',
            'name': '',
            'desc': '',
            'condition': '整车模式变量 == 2 or 整车模式变量 == 7 or 整车模式变量 == 9',
            'event_list': [],
            'action_list': [],
            'source_node': 'state14',
            'target_node': 'state17',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 's17_s20',
            'name': '',
            'desc': '',
            'condition': 'BMS自检计数器 == 2 and (整车模式变量 == 2 or 整车模式变量 == 7 or 整车模式变量 == 9)',
            'event_list': [],
            'action_list': [],
            'source_node': 'state17',
            'target_node': 'state20',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 's20_s23',
            'name': '',
            'desc': '',
            'condition': 'BMS自检计数器 == 3 and (整车模式变量 == 2 or 整车模式变量 == 7 or 整车模式变量 == 9)',
            'event_list': [],
            'action_list': [],
            'source_node': 'state20',
            'target_node': 'state23',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 's23_s30',
            'name': '',
            'desc': '',
            'condition': 'MCU高压检测完成标志 == 1 and 整车模式变量 == 2',
            'event_list': [],
            'action_list': [],
            'source_node': 'state23',
            'target_node': 'state30',
            'weights': {'value': 1}
        },
    ]
}

simple_project_demo = {
    'name': 'test project',
    'desc': [],
    'creator': 'zy',
    'create_time': round(time.time() * 1000000),
    'updator': 'zy',
    'update_time': round(time.time() * 1000000),
    'environments': [{'types': simple_signal_define, 'logics': []}],
    'graphs': [state30]
}
