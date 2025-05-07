"""
This code is a part of BJEV Test ToolChain
Created by Bolow in 2022-02.
Copyright (c) 2022 BJEV. All rights reserved.
"""
from typing import List

from fsm.env.type.signal_type import SignalTypeConfig

simple_signal_define: List[SignalTypeConfig] = [
    {
        'name': 'value1',
        'desc': '',
        'expr': 'value2/2+1',
        'data_type': 'int',
        'default_value': 0,
        'factor': 1,
        'ranges': [{'max_value': 255, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}]
    },
    {
        'name': 'value2',
        'desc': '',
        'expr': '10',
        'data_type': 'int',
        'default_value': 0,
        'factor': 1,
        'ranges': [{'max_value': 255, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}]
    }
]

simple_signal_define_cycle: List[SignalTypeConfig] = [
    {
        'name': 'value1',
        'desc': '',
        'expr': 'value2/2+1',
        'default_value': 0,
        'factor': 1,
    },
    {
        'name': 'value2',
        'expr': '10+value3',
    },
    {
        'name': 'value3',
        'expr': '10+value1',
    }
]
