from typing import List

from fsm.env.type.signal_type import SignalTypeConfig

simple_signal_define: List[SignalTypeConfig] = [
    {
        'name': 'BMS自检计数器',
        'desc': '',
        'expr': 'MCU初始化及自检标志位/2+1',
        'data_type': 'int',
        'default_value': 0,
        'factor': 1,
        'ranges': [{'max_value': 4, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}],
        'intervals': ['[0,4]'],
        'values': []
    },
    {
        'name': 'MCU初始化及自检标志位',
        'desc': '',
        'expr': '10',
        'data_type': 'int',
        'default_value': 0,
        'factor': 1,
        'ranges': [{'max_value': 1, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}],
        'intervals': ['[0,1]'],
        'values': []
    },
    {
        'name': 'MCU高压检测完成标志',
        'desc': '',
        'expr': '1',
        'data_type': 'int',
        'default_value': 0,
        'factor': 1,
        'ranges': [{'max_value': 1, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}],
        'intervals': ['[0,1]'],
        'values': []
    },
    {
        'name': '整车模式变量',
        'desc': '',
        'expr': '1',
        'data_type': 'int',
        'default_value': 0,
        'factor': 1,
        'ranges': [{'max_value': 10, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}],
        'intervals': ['[0,10]'],
        'values': []
    },
    {
        'name': 'period',
        'desc': '',
        'expr': '0',
        'data_type': 'float',
        'default_value': 0,
        'factor': 0.2,
        'ranges': [{'max_value': 10, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}],
        'intervals': ['[0,inf]'],
        'values': []
    },
    {
        'name': 'timestamp',
        'desc': '',
        'expr': '0',
        'data_type': 'float',
        'default_value': 0,
        'factor': 0.2,
        'ranges': [{'max_value': 10, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}],
        'intervals': ['[0,inf]'],
        'values': []
    }
]
