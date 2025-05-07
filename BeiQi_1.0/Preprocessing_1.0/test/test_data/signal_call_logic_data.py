"""
This code is a part of BJEV Test ToolChain
Created by Bolow in 2022-02.
Copyright (c) 2022 BJEV. All rights reserved.
"""
signal_call_logic_define = {
    'logics': [
        {'name': 'do_double',
         'desc': '',
         'type': 'logic',
         'expr': 'return x*2+1',
         'return_type': 'void',
         'param_list': [{'name': 'x', 'type': 'int'}],
         'return_ranges': [{'max_value': 255, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}]
         }
    ],
    'types': [
        {
            'name': 'value1',
            'desc': '',
            'type': 'signal',
            'expr': 'do_double(value2)',
            'signal_type': 'signal',
            'data_type': 'int',
            'default_value': 0,
            'direct': 'out',
            'factor': 1,
            'ranges': [{'max_value': 255, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}]
        },
        {
            'name': 'value2',
            'desc': '',
            'type': 'signal',
            'expr': '10',
            'signal_type': 'signal',
            'data_type': 'int',
            'default_value': 0,
            'direct': 'out',
            'factor': 1,
            'ranges': [{'max_value': 255, 'min_value': 0, 'include_max_value': True, 'include_min_value': True}]
        }
    ]
}
