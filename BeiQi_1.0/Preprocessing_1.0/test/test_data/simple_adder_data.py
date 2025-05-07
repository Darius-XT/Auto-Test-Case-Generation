"""
This code is a part of BJEV Test ToolChain
Created by Bolow in 2022-02.
Copyright (c) 2022 BJEV. All rights reserved.
"""
from typing import List

from fsm.env.type.signal_type import SignalTypeConfig

adder_graph = {
    'id': 'add-graph',
    'desc': 'sum 1 to 10',
    'type_name': 'graph',
    'h_function': 'none',
    'entry_action_list': [],
    'exit_action_list': [],
    'nodes': [
        {
            'type_name': 'state',
            'id': 'start',
            'name': 'Start',
            'desc': 'Start',
            'is_init_node': True,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': [],
            'output_transitions': ['calc']
        },
        {
            'type_name': 'state',
            'id': 'adder',
            'name': 'adder',
            'desc': 'Running',
            'is_init_node': False,
            'is_finish_node': False,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'sum=sum+n', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0},
                {'express': 'n=n-1', 'type': 'action', 'pre_think_time': 0, 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [
                {'express': 'print(f"sum = {sum} n={n}")', 'type': 'action', 'pre_think_time': 0,
                 'post_think_time': 0}],
            'input_transitions': ['calc', 'continue'],
            'output_transitions': ['end', 'continue']
        },
        {
            'type_name': 'state',
            'id': 'finish',
            'name': 'finish',
            'desc': 'Finish',
            'is_init_node': False,
            'is_finish_node': True,
            'entry_action_list': [],
            'pre_think_time': 0,
            'during_action_list': [
                {'express': 'print(f"Finish node says: Result of sum is {sum}")',
                 'type': 'action',
                 'pre_think_time': 0,
                 'post_think_time': 0}],
            'post_think_time': 0,
            'exit_action_list': [],
            'input_transitions': ['end'],
            'output_transitions': []
        },
    ],
    'transitions': [
        {
            'type_name': 'transition',
            'id': 'calc',
            'name': 'start',
            'desc': 'goto adder',
            'condition': '',
            'event_list': ['start'],
            'action_list': [],
            'source_node': 'start',
            'target_node': 'adder',
            'weights': {'value': 1}
        },
        {
            'type_name': 'transition',
            'id': 'continue',
            'name': 'continue',
            'desc': 'goto adder',
            'condition': 'sum>0',
            'event_list': [],
            'action_list': [],
            'source_node': 'adder',
            'target_node': 'adder',
            'weights': {'value': 1},
            'test_layer': {'max_entry_count': 5}
        },
        {
            'type_name': 'transition',
            'id': 'end',
            'name': 'end',
            'desc': 'end',
            'condition': 'n==0',
            'event_list': [],
            'action_list': [],
            'source_node': 'adder',
            'target_node': 'finish',
            'weights': {'value': 1}
        }
    ]
}

adder_data: List[SignalTypeConfig] = [
    {
        'name': 'sum',
        'expr': '0',
        'data_type': 'int',
    },
    {
        'name': 'n',
        'expr': '10',
        'data_type': 'int',
    },
    {
        'name': 'start',
        'expr': '0',
        'data_type': 'int',
    }
]
